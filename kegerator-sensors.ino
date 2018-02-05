#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS); // Data wire is plugged into port 1 on the wemos
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

void setup() {
    //Setup Serial port speed
    Serial.begin(115200);
    delay(100);

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