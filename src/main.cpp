//TEST GITHUB
#include <WiFi.h>
#include <Arduino.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FS.h"
#include <SD.h>
#include <SPI.h>
#include "utilities.h"
#include <Wire.h>



#define BLYNK_TEMPLATE_ID "TMPL4sQab-LLf"
#define BLYNK_TEMPLATE_NAME "Water flow and temp meter device"
#define BLYNK_AUTH_TOKEN "-VpXPSx3308b550B-A5MnkosS0pmdgtJ"
#include <Blynk.h>


//WIFI CONTENTS............................
const char* ssid = "iPhone";
const char* password = "Sest_Siemens@56";
//............................................

float temperatureIN;
float temperatureOUT;
float debit;


//SIMMODULE CONFIGURATION.............................................
//#define TINY_GSM_MODEM_SIM7600

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS


#define PIN "9528"
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
BlynkTimer timer;

const char apn[] = "internet.proximus.be";
const char gprsUser[] = "";
const char gprsPass[] = "";
const int port = 80; // Standard HTTP port
#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

unsigned long lastSMSTime = 0;
const unsigned long intervalBetweenSMS = 300000;

bool Fluctuation_flag = false; 
// Définition des plages de mesure
const int plageA_min = 10;
const int plageA_max = 100;
const int plageDiff_min = 5;
const int plageDiff_max = 50;
//....................................................................................

//Permet à la boucle de s'exécuter toutes les 10 sec.....................
unsigned long lastDataSendTime = 0; // Variable pour stocker le moment de la dernière envoi
//...............................................................

//DEFINE MODULE SD CARD...............
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
SPIClass sdSPI(VSPI);
char dataC[256];
//....................................

//FORMATTED TIME CONTENTS (NTP SERVER)................
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
struct tm timeinfo;
// Variables to save date and time
char formattedDate[20];
char dayStamp[12];
char timeStamp[9];
//....................................................

//EEPROM VARIABLES.................................
int addr_DeviceName = 0;
int addr_DeviceNumber = sizeof(String); 
int addr_PhoneNumber = sizeof(String) * 2;
String global_DeviceName;
String global_DeviceNumber;
String sms_target;
//..................................................

//WEB SERVER CONTENTS............
WebServer server(80);
const char* html_page;
//...............................

//TEMPERATURE READING CONTENTS...................................................
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// Number of temperature devices found
int numberOfDevices;
// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress; 
//..............................................................................

//ALL FUNCTIONS...........................................................................................................
void printAddress(DeviceAddress deviceAddress);
void Send_DATA(float valeur1, float valeur2, float valeur3, char* valeur4, char* valeur5, String valeur6, String valeur7);
void getFormattedDateTime();
void randomvalue();
void sendBlynk();

void locateDevice();
void tempReading();
void webInterface();
void webLogic();
void readEEPROMData();

//Sd card functions//
void SD_init();
void Write_DATA();
void Append_DATA_SD(fs::FS &fs, const char * path, const char * message);
void writeFile_SD(fs::FS &fs, const char * path, const char * message);

//MOBILE FUNCTIONS
void modem_setup();
bool send_SMS(const String& phoneNumber, const String& dayStamp, const String& timeStamp, float debit, float temperatureIN, float temperatureOUT, const String& global_DeviceName, const String& global_DeviceNumber);
//.............................................................................................................................


void setup() {

  // Initialize serial communication first
  Serial.begin(115200);

  // Other initialization tasks
  sensors.begin();
  EEPROM.begin(512);

// CONNEXION AU WIFI.....................
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wifi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
//.......................................
  randomSeed(analogRead(0)); // Initialisation de la graine pour les valeurs aléatoires
  randomvalue();

  readEEPROMData();

  modem_setup();
  webInterface();
  webLogic();
  locateDevice();

  SD_init();
  Write_DATA();
}

void loop() {

  randomvalue();
  readEEPROMData();
  
  Blynk.run();
  timer.run();
  sendBlynk();


  if (WiFi.status() == WL_CONNECTED) {

    server.handleClient();

    // Obtenir le temps actuel en millisecondes
    unsigned long currentTime = millis();
    
    if (currentTime - lastDataSendTime >= 20000) {

      // Mettre à jour la date et l'heure
      getFormattedDateTime();
   
      // Envoyer les données
      Send_DATA(temperatureIN, temperatureOUT, debit,  dayStamp, timeStamp, global_DeviceName, global_DeviceNumber);
      // Mettre à jour le moment du dernier envoi
      lastDataSendTime = currentTime;
       Write_DATA();

    }
    
    if (millis() - lastSMSTime >= intervalBetweenSMS) {
      // Votre condition pour envoyer un SMS
      if (debit < plageA_min || debit > plageA_max || (abs(temperatureOUT - temperatureIN) < plageDiff_min || abs(temperatureOUT - temperatureIN) > plageDiff_max)) {
        send_SMS(sms_target, dayStamp, timeStamp, debit, temperatureIN, temperatureOUT, global_DeviceName, global_DeviceNumber);
        lastSMSTime = millis(); // Mettre à jour le moment de l'envoi du SMS
      }
    }
  }
  else {
    Serial.print("Wifi failed. Passage en mode hors connexion");
    delay(2000);

  }

}

// function to send all the data to PowerAutomate flow with a Http request
void Send_DATA(float valeur1, float valeur2, float valeur3, char* valeur4, char* valeur5,  String valeur6,  String valeur7) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    //Server URL
    String url = "https://prod-01.westeurope.logic.azure.com:443/workflows/7d254d10055740139d924de23ddad9d6/triggers/manual/paths/invoke?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=tMdfU4rTNOX6Yjz-56Ap5Qn08llK16QJXVtuB0OvvlA";

    // Create JSON object
    StaticJsonDocument<200> jsonDocument;
    jsonDocument["temperatureIN"] = valeur1;
    jsonDocument["temperatureOUT"] = valeur2;
    jsonDocument["debit"] = valeur3;
    jsonDocument["date"] = valeur4;
    jsonDocument["hour"] = valeur5;
    jsonDocument["Device Name"] = valeur6;
    jsonDocument["Device Number"] = valeur7;

    // Serialize JSON to string
    String jsonData;
    serializeJson(jsonDocument, jsonData);

    Serial.print("Sending the data ... ");
    Serial.println(jsonData);

    // Start the HTTP Connection.
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    //http.addHeader("Authorization", "Bearer VOTRE_CLÉ_API");

    // Send the post request with the data.
    int codeReponse = http.POST(jsonData);

    // Check the response of the server.
    if (codeReponse > 0) {
      Serial.print("Response: ");
      Serial.println(codeReponse);
      String reponse = http.getString();
      Serial.println(reponse);
    } else {
      Serial.print("HTTP Request Error: ");
      Serial.println(codeReponse);
    }

    // Free up the resources of the HTTP connection
    http.end();
  } else {
    Serial.println("WiFi connection error.");
  }
}

//function who takes the local time to a NTP server every hour. 
void getFormattedDateTime() {
  unsigned long lastUpdateTime = 0;
  const unsigned long updateInterval = 60000;  // Mettre à jour le temps toutes les 60 secondes (60000 millisecondes)
  
  // Vérifier si le temps doit être mis à jour//
  if (millis() - lastUpdateTime >= updateInterval || lastUpdateTime == 0) {
    
    // Mettre à jour le temps uniquement si nécessaire
    timeClient.update();

    // Enregistrer le moment de la dernière mise à jour
    lastUpdateTime = millis();
  }

  // Obtain elapsed time since epoch (January 1, 1970) in seconds
  time_t rawtime = timeClient.getEpochTime();
  
  // Ajouter le décalage horaire (par exemple, 1 heure = 3600 secondes)
  rawtime += 7200;

  // Convertir le temps en une structure de temps (struct tm) contenant la date et l'heure
  localtime_r(&rawtime, &timeinfo);
  
  // Formater la date et l'heure
  strftime(formattedDate, sizeof(formattedDate), "%Y-%m-%d %H:%M:%S", &timeinfo);
  
  // Extraire la date et l'heure des structures de temps
  strftime(dayStamp, sizeof(dayStamp), "%Y-%m-%d", &timeinfo);
  strftime(timeStamp, sizeof(timeStamp), "%H:%M:%S", &timeinfo);

  // Afficher la date et l'heure
  Serial.println(formattedDate);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
}

// function to print a device address
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
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
}

// function who read the température of the DS18B20 sensors
/*void tempReading(){ 

  sensors.requestTemperatures(); // Send the command to get temperatures
  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      // Output the device ID
      Serial.print("Device_add ");
      Serial.print(i);
      Serial.print(" ");
      if (i == 0) {
        Serial.print("TempératureIN: ");
        temperatureIN = sensors.getTempC(tempDeviceAddress); // Get temperature reading
        Serial.println(temperatureIN);
      } else if (i == 1) {
        Serial.print("TempératureOUT: ");
        temperatureOUT = sensors.getTempC(tempDeviceAddress); // Get temperature reading
        Serial.println(temperatureOUT);
      }
    }
  }
}
*/

// function for the design of the web server
void webInterface(){

html_page = R"(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Water flow/temp meter device</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #f9f9f9;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }
    #container {
      width: 400px;
      padding: 30px;
      background-color: #fff;
      border-radius: 10px;
      box-shadow: 0 0 20px rgba(0, 0, 0, 0.1);
    }
    h1 {
      text-align: center;
      color: #333;
    }
    form {
      margin-top: 20px;
    }
    label {
      display: block;
      margin-bottom: 5px;
      color: #555;
    }
    input[type="text"] {
      width: 100%;
      padding: 10px;
      margin-bottom: 20px;
      border: 1px solid #ddd;
      border-radius: 5px;
      transition: border-color 0.3s ease;
    }
    input[type="text"]:focus {
      border-color: #007bff;
    }
    input[type="button"] {
      width: 100%;
      padding: 10px;
      background-color: #007bff;
      color: #fff;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }
    input[type="button"]:hover {
      background-color: #0056b3;
    }
    img {
      display: block;
      margin: 0 auto;
      max-width: 100%;
      height: auto;
      margin-bottom: 20px;
      width: 200px; 
    }
  </style>
</head>
<body>
  <div id='container'>
    <img src='https://oncovalue.org/wp-content/uploads/2023/03/Siemens_Healthineers_logo.svg_.png' alt='Logo' width="100">
    <h1>Water flow/temp meter</h1>
    <form id='myForm'>
      <label for='deviceName'>Device Name:</label>
      <input type='text' id='deviceName' name='deviceName' placeholder="Enter the device name">
      <label for='deviceNumber'>Serial number:</label>
      <input type='text' id='deviceNumber' name='deviceNumber' placeholder="Enter the Serial number">
      <label for='phoneNumber'>Phone number :</label>
      <input type='text' id='phoneNumber' name='phoneNumber' placeholder="Enter your phone number">
      <input type='button' value='Save' onclick='submitForm()'>
    </form>
  </div>
  <script>
    function submitForm() {
      var form = document.getElementById('myForm');
      var deviceName = form.elements['deviceName'].value;
      var deviceNumber = form.elements['deviceNumber'].value;
      var phoneNumber = form.elements['phoneNumber'].value;
      var xhr = new XMLHttpRequest();
      xhr.open('POST', '/send', true);
      xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
      xhr.onreadystatechange = function() {
        if (xhr.readyState == XMLHttpRequest.DONE && xhr.status == 200) {
          alert(xhr.responseText);
          form.reset();
        }
      };
      xhr.send('deviceName=' + encodeURIComponent(deviceName) + '&deviceNumber=' + encodeURIComponent(deviceNumber) + '&phoneNumber=' + encodeURIComponent(phoneNumber));
    }
  </script>
</body>
</html>
)";


}

// function to interact with the web server
void webLogic(){

    // Route pour la page d'accueil du web server
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", html_page);
  });

  // Route pour recevoir les données
  server.on("/send", HTTP_POST, []() {
    String DeviceName = server.arg("deviceName");
    String DeviceNumber = server.arg("deviceNumber");
    String PhoneNumber = server.arg("phoneNumber");
    
    // Enregistrement des valeurs dans l'EEPROM
    EEPROM.put(addr_DeviceName, DeviceName);
    EEPROM.put(addr_DeviceNumber, DeviceNumber);
    EEPROM.put(addr_PhoneNumber, PhoneNumber);
    EEPROM.commit(); // Commit des changements
    Serial.println("Data saved in EEPROM !");

    Serial.print("Device Name: ");
    Serial.println(DeviceName);
    Serial.print("Device Number: ");
    Serial.println(DeviceNumber);
    Serial.print("Device Name: ");
    Serial.println(PhoneNumber);
    server.send(200, "text/plain", "Data send ! :)");
  });

  server.begin();
  Serial.println("Serveur Web démarré !");
}

//Fonction d'initialisation de la carte SD
void SD_init(){
  
// INITIALIZING SD CARD ...........................
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS, sdSPI)) {
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("SPI.OK!");
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("!ERROR! initialization failed!");
    return;  
  }
  Serial.println("Initialization SD card OK !");
//..................................................


// IF FILE NOT EXIST CREATE NEW FILE (SD CARD)...............
  File file = SD.open("/DATA_SAVE1.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile_SD(SD, "/DATA_SAVE1.txt", "Reading ID, Date, Hour, Temperature \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
//..................................................

}

// Fonction qui permet juste mettre notre data dans une variable et fait appel à Append_DATA pour ajouter dans le fichier présent dans la carte SD  //
void Write_DATA() {

  sprintf(dataC, "%.2f %.2f %.2f %s %s %s %s", temperatureIN, temperatureOUT, debit, dayStamp, timeStamp, global_DeviceName, global_DeviceNumber);
  String dataMessage = String(dataC);
  dataMessage += "\n";
  Serial.print("Save data.. ");
  Serial.println(dataMessage);
  Append_DATA_SD(SD, "/DATA_SAVE1.txt", dataMessage.c_str());
}

// FUNCTION TO WRITE A NEW FILE TO SD CARD //
void writeFile_SD(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card 
void Append_DATA_SD(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    SD_init();
    
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// Fonction qui permet de mettre en place toutes les fonctionalitées gsm. 
void modem_setup(){
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
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    Serial.println("Start modem...");
    delay(3000);

    while (!modem.testAT()) {
        delay(10);
    }

    // Wait PB DONE
    delay(10000);

    // Unlock SIM PIN
    Serial.println("Unlocking SIM PIN...");
    if (modem.simUnlock(PIN)) {
        Serial.println("SIM unlocked successfully!");
    } else {
        Serial.println("Failed to unlock SIM!");
    }

    // Check network registration
    Serial.println("Checking network registration...");
    if (!modem.waitForNetwork()) {
        Serial.println("Failed to register to network!");
        return;
    } else {
        Serial.println("Network registered!");
    }
    // Print messages before Blynk initialization
  Serial.println("Avant Blynk.begin");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, modem, apn, gprsUser, gprsPass);

  // Print message after Blynk initialization (optional)
  Serial.print("Après Blynk.begin");

}

//Fonction d'envoi d'sms
bool send_SMS(const String& phoneNumber, const String& dayStamp, const String& timeStamp, float debit, float temperatureIN, float temperatureOUT, const String& deviceName, const String& deviceNumber) {

  String message = "Fluctuation detected :"+ dayStamp+"/"+ timeStamp+ " \n\n";
  message += "Appareil : " + global_DeviceName + "\n";
  message += "Numero de serie : " + global_DeviceNumber + "\n";
  message += "Debit : " + String(debit) + "\n";
  message += "Temp IN : " + String(temperatureIN) + "\n";
  message += "Temp OUT : " + String(temperatureOUT) + "\n";
  
  Serial.println("Fluctuation !! Alarme send!");
  
  return modem.sendSMS(phoneNumber, message);
}

void randomvalue() {

  // Génère de nouvelles valeurs aléatoires
  temperatureIN = random(13,15);
  temperatureOUT = random(12, 14);
  debit = random(2000, 2500);
}

void readEEPROMData() {
    EEPROM.get(addr_DeviceName, global_DeviceName);
    EEPROM.get(addr_DeviceNumber, global_DeviceNumber);
    EEPROM.get(addr_PhoneNumber, sms_target);
}

void sendBlynk(){

    Blynk.virtualWrite(V0, temperatureIN);
    Blynk.virtualWrite(V1, temperatureOUT);
    Blynk.virtualWrite(V2, debit);
}


