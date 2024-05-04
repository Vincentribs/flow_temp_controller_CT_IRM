#include <SPI.h>
#include <SD.h>

class SD_Manager {

private:
  SPIClass sdSPI;
  const int SD_MISO_PIN = 2;
  const int SD_MOSI_PIN = 15;
  const int SD_SCLK_PIN = 14;
  const int SD_CS_PIN = 13;
  char dataC[256];

public:
  SD_Manager() : sdSPI(VSPI) {
    // Initialisation des broches SPI et de l'objet SPIClass dans le constructeur
    sdSPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  }


  void sd_init() {
    sdSPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN, sdSPI)) {
      Serial.println("Card Mount Failed");
      return;
    }
    Serial.println("SPI.OK!");
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      return;
    }
    Serial.println("Initializing SD card...");

    if (!SD.begin(SD_CS_PIN)) {
      Serial.println("!ERROR! initialization failed!");
      return;
    }
    Serial.println("Initialization SD card OK !");

    // IF FILE NOT EXIST CREATE NEW FILE (SD CARD)...............
    File file = SD.open("/DATA_SAVE1.txt", FILE_APPEND);
    if (!file) {
      Serial.println("File doesn't exist");
      Serial.println("Creating file...");
      writeFile("/DATA_SAVE1.txt", "Reading ID, Date, Hour, Temperature \r\n");
    } else {
      Serial.println("File already exists");
    }
    file.close();
    //...................................................
  }

  void write_data(float temperatureIN, float temperatureOUT, float debit, String dayStamp, String timeStamp, String global_DeviceName, String global_DeviceNumber) {
    sprintf(dataC, "%.2f %.2f %.2f %s %s %s %s", temperatureIN, temperatureOUT, debit, dayStamp.c_str(), timeStamp.c_str(), global_DeviceName.c_str(), global_DeviceNumber.c_str());
    String dataMessage = String(dataC);
    dataMessage += "\n";
    Serial.print("Save data.. ");
    Serial.println(dataMessage);
    appendData("/DATA_SAVE1.txt", dataMessage.c_str());
  }

  void writeFile(const char *path, const char *message) {
    Serial.printf("Writing file: %s\n", path);

    File file = SD.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }

    if (file.print(message)) {
      Serial.println("File written");
    } else {
      Serial.println("Write failed");
    }
    file.close();
  }

  void appendData(const char *path, const char *message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = SD.open(path, FILE_APPEND);
    if (!file) {
      Serial.println("Failed to open file for appending");
      init(); // Reinitialize SD card if append fails
      return;
    }

    if (file.print(message)) {
      Serial.println("Message appended");
    } else {
      Serial.println("Append failed");
    }
    file.close();
  }
};

