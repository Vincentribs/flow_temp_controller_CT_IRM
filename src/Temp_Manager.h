#include <OneWire.h>
#include <DallasTemperature.h>

class Temp_Manager{
private: 
    OneWire oneWire;
    DallasTemperature sensors;
    DeviceAddress tempDeviceAddress;

    int numberOfDevices;
    float temperatureIN;
    float temperatureOUT;
    
public:
    Temp_Manager(int pin) : oneWire(pin), sensors(&oneWire){}

    void begin(){
        sensors.begin();
        Serial.println("Temp sensor initialized !");
    }

    void printAddress(DeviceAddress deviceAddress) {
        for (uint8_t i = 0; i < 8; i++){
            if (deviceAddress[i] < 16) Serial.print("0");
                Serial.print(deviceAddress[i], HEX);
        }
    }  

    // function who locate and define device ADD 
    void locateDevice(){
        // Grab a count of devices on the wire
        numberOfDevices = sensors.getDeviceCount();

        // locate devices on the bus
        Serial.print("Locating temp devices...");
        Serial.print("Found ");
        Serial.print(numberOfDevices, DEC);
        Serial.println(" devices.");

        // Loop through each device, print out address
        for(int i=0;i<numberOfDevices; i++){
            // Search the wire for address
            if(sensors.getAddress(tempDeviceAddress, i)){
                Serial.print("Found device ");
                Serial.print(i, DEC);
                Serial.print(" with address: ");
                printAddress(tempDeviceAddress);
                Serial.println(" ");
            } 
            else {
                Serial.print("Found ghost device at ");
                Serial.print(i, DEC);
                Serial.print(" but could not detect address. Check power and cabling");
            }
        }
    }

    void tempReading(){ 
        sensors.requestTemperatures(); // Send the command to get temperatures
        // Loop through each device, print out temperature data
        for(int i=0;i<numberOfDevices; i++){
            // Search the wire for address
            if(sensors.getAddress(tempDeviceAddress, i)){
                // Output the device ID
                if (i == 0) {
                    temperatureIN = sensors.getTempC(tempDeviceAddress); // Get temperature reading
                } 
                    else if (i == 1) {
                        temperatureOUT = sensors.getTempC(tempDeviceAddress); // Get temperature reading   
                    }
            }
        }
    }
        // Fonction pour récupérer la température intérieure
    float getTemperatureIN() {
        return temperatureIN;
    }

    // Fonction pour récupérer la température extérieure
    float getTemperatureOUT() {
        return temperatureOUT;
    }
};