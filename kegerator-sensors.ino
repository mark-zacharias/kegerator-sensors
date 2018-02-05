#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

const char* ssid = "jerkstore2.4"; //WIFI Name, WeMo will only connect to a 2.4GHz network.
const char* password = "burlives"; //WIFI Password

//defining the pin and setting up the "server"
//3
//int relayPin = D1; // The Shield uses pin 1 for the relay
//WiFiServer server(80);
IPAddress ip(192, 168, 1, 120); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 254); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS); // Data wire is plugged into port 1 on the wemos
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

void setup() {
    //Setup Serial port speed
    SetupSerial();
    
    SetupWIFI();

    SetupTempProbe();    
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

void loop() {
    sensors.requestTemperatures();

    //Get resolution of DS18b20
    // Serial.print("Resolution: ");
    // Serial.print(sensors.getResolution(0));
    // Serial.println();
    
    //Read temperature from DS18b20
    float tempC = sensors.getTempCByIndex(0);
    Serial.print("Temp: ");
    Serial.println(tempC);
    
    delay(5000);
}