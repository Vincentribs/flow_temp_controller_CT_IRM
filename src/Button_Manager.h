#include <Arduino.h>

// Classe ButtonManager pour gérer l'état du bouton
class ButtonManager {
public:
  ButtonManager(int pin) : buttonPin(pin) {
    pinMode(buttonPin, INPUT);
  }

  bool isButtonPressed() {
    return digitalRead(buttonPin) == HIGH;
  }

private:
  int buttonPin;
  
};

