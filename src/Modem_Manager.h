
/*
#ifndef MODEM_MANAGER_H
#define MODEM_MANAGER_H

#include <Arduino.h>
#include <TinyGsmClient.h>
#include "utilities.h"
#include <TinyGSM.h>

class Modem_Manager {
public:
    Modem_Manager(const char* apn, const char* gprsUser, const char* gprsPass, const char* simPin, TinyGsm& modem) 
        : _apn(apn), _gprsUser(gprsUser), _gprsPass(gprsPass), _simPin(simPin), _modem(modem) {}

    void modem_setup() {
  #ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif

    // Set modem reset pin ,reset modem
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
    digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

    // Turn on modem
    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);

    // Set ring pin input
    pinMode(MODEM_RING_PIN, INPUT_PULLUP);

        // Set modem baud
      // Set modem baud
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
        
        Serial.println("Start modem...");
        // Oled_manager.writemessage("Start modem..."); // Vous devez gérer l'affichage en dehors de cette classe
        delay(3000);

        while (!_modem.testAT()) {
            delay(10);
        }

        // Wait PB DONE
        delay(10000);

        // Unlock SIM PIN
        Serial.println("Unlocking SIM PIN...");
        // Oled_manager.writemessage("Unlocking SIM PIN..."); // Vous devez gérer l'affichage en dehors de cette classe
        delay(1000);

        if (_modem.simUnlock(_simPin)) {
            Serial.println("SIM unlocked successfully!");
            // Oled_manager.writemessage("SIM unlocked successfully!"); // Vous devez gérer l'affichage en dehors de cette classe
        } else {
            Serial.println("Failed to unlock SIM!");
            // Oled_manager.writemessage("Failed to unlock SIM!"); // Vous devez gérer l'affichage en dehors de cette classe
        }
        delay(1000);

        // Check network registration
        Serial.println("Checking network registration...");
        // Oled_manager.writemessage("Checking network registration..."); // Vous devez gérer l'affichage en dehors de cette classe
        if (!_modem.waitForNetwork()) {
            Serial.println("Failed to register to network!");
            // Oled_manager.writemessage("Failed to register to network!"); // Vous devez gérer l'affichage en dehors de cette classe
            return;
        } else {
            Serial.println("Network registered!");
            // Oled_manager.writemessage("Network registered!"); // Vous devez gérer l'affichage en dehors de cette classe
        }
    }

private:
    const char* _apn;
    const char* _gprsUser;
    const char* _gprsPass;
    const char* _simPin;
    TinyGsm _modem;
};
#endif

*/

