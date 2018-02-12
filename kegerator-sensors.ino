#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "jerkstore2.4"; //WIFI Name, WeMo will only connect to a 2.4GHz network.
const char* password = "burlives"; //WIFI Password
IPAddress ip(192, 168, 1, 120); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 254); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

IPAddress mqtt_server(192, 168, 1, 64);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS); // Data wire is plugged into port 1 on the wemos
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

void setup() {
    //Setup Serial port speed
    SetupSerial();
    
    SetupWIFI();
    SetupMQTT();

    SetupTempProbe();    
}
void loop() {

    //Get resolution of DS18b20
    // Serial.print("Resolution: ");
    // Serial.print(sensors.getResolution(0));
    // Serial.println();

    //publish temp
    publishTemp();

    //publish weight

    //loop mqtt client
    mqttLoop();
    
    delay(5000);
}

void SetupSerial() {
    Serial.begin(115200);
    delay(100);
}
void SetupWIFI() {
    Serial.println("Setting static ip to : ");
    Serial.println(ip);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.config(ip, gateway, subnet); 
    WiFi.begin(ssid, password);
    //Trying to connect it will display dots
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
}
void SetupMQTT() {    
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}
void SetupTempProbe() {
    Serial.println("Starting Tempature Reading");
    sensors.begin();    
    Serial.print("Parasite power is: "); 
    if(sensors.isParasitePowerMode() ) {
        Serial.println("ON");
    }
    else {
        Serial.println("OFF");
    }
    
    int numberOfDevices = sensors.getDeviceCount();
    Serial.print( "Device count: " );
    Serial.println( numberOfDevices );
}

long prevTempPublish = 0;
long intervalTemp = 10000;
char topicTemp[30] = "home/kegerator/temp";
void publishTemp() {
    long now = millis();
    if(now - prevTempPublish >= intervalTemp) {
        prevTempPublish = now;

        sensors.requestTemperatures();
        //Read temperature from DS18b20
        float tempC = sensors.getTempCByIndex(0);
        Serial.print("Temp: ");
        Serial.println(tempC);
        mqttClient.publish(topicTemp, (double)tempC);
    }
}

// long lastMsg = 0;
// char msg[50];
// int value = 0;

void mqttLoop() {
    if (!mqttClient.connected()) {
        reconnectMQTTClient();
    }
    mqttClient.loop();
    
    // long now = millis();
    // if (now - lastMsg > 5000) {
    //     lastMsg = now;
    //     ++value;
    //     snprintf (msg, 75, "hello world #%ld", value);
    //     Serial.print("Publish message: ");
    //     Serial.println(msg);
    //     mqttClient.publish("outTopic", msg);
    // }
}
void reconnectMQTTClient() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}