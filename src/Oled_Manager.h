#ifndef OLED_MANAGER_H
#define OLED_MANAGER_H

#include <U8g2lib.h>

class Oled_Manager {
private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
  

public:
  // Constructor initializing the OLED display
  Oled_Manager() : u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE) {}

  // Method to initialize the OLED display
  void init() {
    u8g2.begin();
  }

  // Method to write a message on the OLED display
  void writemessage(const char* message) {
    u8g2.clearBuffer(); // Clear the previous content of the buffer

    // Determine the width and height of the screen
    int displayWidth = u8g2.getDisplayWidth();
    int displayHeight = u8g2.getDisplayHeight();

    // Choose font and color
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setFontMode(1);
    u8g2.setDrawColor(1);

    // Coordinates to center the text horizontally
    int16_t x = (displayWidth - u8g2.getUTF8Width(message)) / 2;

    // Initial y coordinate to center the text vertically
    int16_t y = displayHeight / 2;

    // Check if the message exceeds the screen width
    if (u8g2.getUTF8Width(message) > displayWidth - 5) {
      // Find the last space before overflow
      int splitIndex = -1;
      for (int i = strlen(message) - 1; i >= 0; i--) {
        if (message[i] == ' ' && u8g2.getUTF8Width(message) <= displayWidth - 5) {
          splitIndex = i;
          break;
        }
      }

      // If no space is found, split at the middle of the message
      if (splitIndex == -1) {
        splitIndex = strlen(message) / 2;
      }

      // Display the first half of the message
      char firstHalf[50]; // Space reserved for the first half of the message
      strncpy(firstHalf, message, splitIndex);
      firstHalf[splitIndex] = '\0'; // Terminate the string
      int16_t x_firstHalf = (displayWidth - u8g2.getUTF8Width(firstHalf)) / 2;
      u8g2.drawStr(x_firstHalf, y - 7, firstHalf);

      // Calculate the y coordinate for the second half of the message
      y += 7;

      // Display the second half of the message on the next line
      char secondHalf[50]; // Space reserved for the second half of the message
      strncpy(secondHalf, message + splitIndex, strlen(message) - splitIndex);
      secondHalf[strlen(message) - splitIndex] = '\0'; // Terminate the string
      int16_t x_secondHalf = (displayWidth - u8g2.getUTF8Width(secondHalf)) / 2;
      u8g2.drawStr(x_secondHalf, y, secondHalf);
    } else {
      // If the message fits on one line, display it directly in the center
      y = (displayHeight - 8) / 2; // Center the text vertically
      u8g2.setCursor(x, y);
      u8g2.print(message);
    }

    u8g2.sendBuffer(); // Send the buffer content to the screen
  }

  void oled_data(float tempIN, float tempOUT, float flow, float batteryLevel) {
    u8g2.clearBuffer(); // Clear the previous content of the buffer

    // Draw lines to form the frame
    u8g2.drawLine(0, 0, 130, 0); // Top line
    u8g2.drawLine(0, 63, 127, 63); // Bottom line

    // Display text at the top of the screen (in yellow)
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setDrawColor(2); // Yellow color
    u8g2.drawStr(8, 12, "Flow & Temp");

    // **Battery Indicator with u8g2_font_battery24_tr (Half Size):**
    int batteryIconWidth = 12; // Half the original width
    int batteryIconHeight = 6;  // Half the original height
    int batteryX = 108;           // Adjust X-coordinate for battery icon placement
    int batteryY = 30;           // Adjust Y-coordinate for battery icon placement

    // Convert integer to a string representing the Unicode code point
    char batteryIconStr[5];
    snprintf(batteryIconStr, sizeof(batteryIconStr), "%d", 0x8000 + (int)(batteryLevel * 5));

    // Draw battery icon using the string
    u8g2.setFont(u8g2_font_battery24_tr);  // Set the battery icon font
    u8g2.setDrawColor(1); // White for empty battery
    u8g2.drawUTF8(batteryX, batteryY, batteryIconStr);



    // Display information at the bottom of the screen (in white)
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.setDrawColor(1); // White color

    char tempStr[10];
    dtostrf(tempIN, 4, 1, tempStr); // Convert tempIN to a string
    u8g2.drawStr(5, 30, "TempIN: ");
    u8g2.drawStr(60, 30, tempStr);
    u8g2.drawUTF8(88, 30, "C°"); // Display the degree symbol (°)

    dtostrf(tempOUT, 4, 1, tempStr); // Convert tempOUT to a string
    u8g2.drawStr(5, 45, "TempOUT: ");
    u8g2.drawStr(60, 45, tempStr);
    u8g2.drawUTF8(88, 45, "C°"); // Display the degree symbol (°)

    char flowStr[10];
    dtostrf(flow, 5, 1, flowStr); // Convert flow to a string
    u8g2.drawStr(5, 60, "Flow: ");
    u8g2.drawStr(60, 60, flowStr);
    u8g2.drawStr(100, 60, "l/h");

    u8g2.sendBuffer(); // Send the buffer content to the screen
}


  // Method to display saved data on the OLED display
  void Eeprom_data(const String& deviceName, const String& deviceNumber, const String& phoneNumber) {
    u8g2.clearBuffer(); // Clear the previous content of the buffer

    // Draw lines to form the frame
    u8g2.drawLine(0, 0, 130, 0); // Top line
    u8g2.drawLine(0, 63, 127, 63); // Bottom line

    // Display text at the top of the screen (in yellow)
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setDrawColor(2); // Yellow color
    u8g2.drawStr(8, 12, "Data Saved !");

    // Display information at the bottom of the screen (in white)
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.setDrawColor(1); // White color

    // Convert String objects to const char*
    const char* deviceNameStr = deviceName.c_str();
    const char* deviceNumberStr = deviceNumber.c_str();
    const char* phoneNumberStr = phoneNumber.c_str();

    u8g2.drawStr(5, 30, "Name: ");
    u8g2.drawStr(50, 30, deviceNameStr);

    u8g2.drawStr(5, 45, "N°: ");
    u8g2.drawStr(50, 45, deviceNumberStr);

    u8g2.drawStr(5, 60, "Phone: ");
    u8g2.drawStr(50, 60, phoneNumberStr);

    u8g2.sendBuffer(); // Send the buffer content to the screen
  }

  
};
#endif
