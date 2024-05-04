#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Oled_Manager.h"

class Eeprom_Manager {
private:

    int addr_DeviceName = 0;
    int addr_DeviceNumber = sizeof(String); 
    int addr_PhoneNumber = sizeof(String) * 2;
    Oled_Manager& Oled_manager; // Référence à Oled_Manager

public:

    Eeprom_Manager(Oled_Manager& Oled_manager) : Oled_manager(Oled_manager) {}

    void save_to_eeprom(String deviceName, String deviceNumber, String phoneNumber) {

        // Enregistrement des valeurs dans l'EEPROM
        EEPROM.put(addr_DeviceName, deviceName);
        EEPROM.put(addr_DeviceNumber, deviceNumber);
        EEPROM.put(addr_PhoneNumber, phoneNumber);
        EEPROM.commit(); // Commit des changements
        Serial.println("Data saved in EEPROM !");
        Oled_manager.Eeprom_data(deviceName, deviceNumber, phoneNumber);

        Serial.print("Device Name: ");
        Serial.println(deviceName);
        Serial.print("Device Number: ");
        Serial.println(deviceNumber);
        Serial.print("Device Name: ");
        Serial.println(phoneNumber);
        delay(4000);
        }
    
    String get_Eeprom_DeviceName() {
        String Eeprom_device_Name;
        EEPROM.get(addr_DeviceName, Eeprom_device_Name);
        return Eeprom_device_Name;
    }

    String get_Eeprom_DeviceNumber() {
        String Eeprom_device_Number;
        EEPROM.get(addr_DeviceNumber, Eeprom_device_Number);
        return Eeprom_device_Number;
    }

    String get_Eeprom_PhoneNumber() {
        String Eeprom_phone_Number;
        EEPROM.get(addr_PhoneNumber, Eeprom_phone_Number);
        return Eeprom_phone_Number;
    }
};

#endif
