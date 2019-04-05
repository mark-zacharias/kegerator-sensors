#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "./config.h"

const char* ssid = "jerkstore2.4"; //WIFI Name, WeMo will only connect to a 2.4GHz network.
const char* password = WIFI_PASSWORD; //WIFI Password
IPAddress ip(192, 168, 1, 120); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 254); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

IPAddress mqtt_server(192, 168, 1, 70);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define led_gpio BUILTIN_LED
#define relay_gpio D3
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS); // Data wire is plugged into port 1 on the wemos
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

void setup() {
    //Setup Serial port speed
    SetupSerial();
    SetupPins();
    
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
    
    //delay(5000);
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

    int resolution = sensors.getResolution();
    Serial.print("Device Resolution: ");
    Serial.println(resolution);
}
void SetupPins() {
  pinMode(relay_gpio, OUTPUT);
  pinMode(led_gpio, OUTPUT);
}

long prevTempPublish = 0;
#define intervalTemp 10000
#define topicTemp "home/kegerator/temp"
int tempCalibration = 0;
void publishTemp() {
    long now = millis();
    if(now - prevTempPublish >= intervalTemp) {
        prevTempPublish = now;

        sensors.requestTemperatures();
        // delay(500);
        //Read temperature from DS18b20
        float tempC = sensors.getTempCByIndex(0);
        tempC = tempC + tempCalibration;
        // Serial.print("Temp: ");
        // Serial.println(tempC);
        char msg[10];
        dtostrf(tempC, 2, 2, msg); 
        // snprintf (msg, 10, "%ld", tempC);
        Serial.print("Publishing: ");
        Serial.println(msg);
        mqttClient.publish(topicTemp, msg);
        
        turnOnOffRelay(tempC);
    }
}
long prevSwitchTime = 0;
#define intervalSwitchTime 30000
#define topicSwitch "home/kegerator/power"
int maxTemp = 16;
int minTemp = 14;
void turnOnOffRelay(float tempC) {
  //only change power if it hasn't changed in x minutes
  long now = millis();
  if(now - prevSwitchTime >= intervalSwitchTime) {
    prevSwitchTime = now;

    if(tempC >= maxTemp){
      blinkTwice();
      digitalWrite(relay_gpio, HIGH);
      digitalWrite(led_gpio, LOW);
      mqttClient.publish(topicSwitch, "ON");
      Serial.println("Power On");
    }
    else if(tempC <= minTemp) {
      blinkTwice();
      digitalWrite(relay_gpio, LOW);
      digitalWrite(led_gpio, HIGH);
      mqttClient.publish(topicSwitch, "OFF");
      Serial.println("Power Off");
    }
  }
}
void blinkTwice() {
  
      digitalWrite(led_gpio, LOW);
      delay(1000);
      digitalWrite(led_gpio, HIGH);
      delay(500);
      digitalWrite(led_gpio, LOW);
      delay(1000);
      digitalWrite(led_gpio, LOW);
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
#define maxTempTopic "home/kegerator/maxtemp"
#define minTempTopic "home/kegerator/mintemp"
#define tempCalibrationTopic "home/kegerator/tempcalibrate"
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
      mqttClient.subscribe(maxTempTopic);
      mqttClient.subscribe(minTempTopic);
      mqttClient.subscribe(tempCalibrationTopic);
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

      
  if(strcmp(topic, maxTempTopic) == 0) {
    setMaxTemp(payload, length);
  }
  else if(strcmp(topic, minTempTopic) == 0) {
    setMinTemp(payload, length);
  }
  else if(strcmp(topic, tempCalibrationTopic) == 0) {
    setTempCalibration(payload, length);
  }
  
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
void setMaxTemp(byte* payload, int length) {
  payload[length] = '\0';  
  maxTemp = atoi((char *)payload);
  Serial.print("MaxTemp:");
  Serial.println(maxTemp);
}
void setMinTemp(byte* payload, int length) {
  payload[length] = '\0';  
  minTemp = atoi((char *)payload);
  Serial.print("MinTemp:");
  Serial.println(minTemp);
}
void setTempCalibration(byte* payload, int length) {
  payload[length] = '\0';  
  tempCalibration = atoi((char *)payload);
  Serial.print("Temp Adjustment:");
  Serial.println(tempCalibration);
}