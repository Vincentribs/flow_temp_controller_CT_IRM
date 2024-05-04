#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FS.h"
#include "utilities.h"
#include <Wire.h>
#include <ArduinoHttpClient.h>
#include <esp_adc_cal.h>

//ALL CLASS....................
#include "SD_Manager.h"
#include "Eeprom_Manager.h"
#include "WebServer_Manager.h"
#include "HttpRequest_Manager.h"
#include "DateTime_Manager.h"
#include "Oled_Manager.h"
#include "Temp_Manager.h"
//............................

//BLYNK CONTENTS...............................................
#define BLYNK_TEMPLATE_ID "TMPL4sQab-LLf"
#define BLYNK_TEMPLATE_NAME "Water flow and temp meter device"
#define BLYNK_AUTH_TOKEN "-VpXPSx3308b550B-A5MnkosS0pmdgtJ"
#include <Blynk.h>
//....................................................................

//WIFI CONTENTS............................
const char* ssid = "iPhone";
const char* password = "Sest_Siemens@56";
//............................................

//CLASS OBJECTS.................................................................
HttpRequest_Manager HttpRequest_manager("https://prod-01.westeurope.logic.azure.com:443/workflows/7d254d10055740139d924de23ddad9d6/triggers/manual/paths/invoke?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=tMdfU4rTNOX6Yjz-56Ap5Qn08llK16QJXVtuB0OvvlA");
DateTime_Manager DateTime_manager;
Oled_Manager Oled_manager;
Eeprom_Manager Eeprom_manager(Oled_manager);
WebServer_Manager WebServer_manager(80, Eeprom_manager);
//...........................................................................

//TEMPERATURE CONTENTS......................
#define Pin 4
float temperatureIN;
float temperatureOUT;
float debit;
Temp_Manager Temp_manager(Pin);
//..................................................

// Déclaration de la variable pour stocker la tension de la batterie
uint16_t battery_voltage = 0;

//MODULE GSM CONTENTS.......................................................................

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

#define PIN "9528"
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
//BlynkTimer timer;

const char apn[] = "internet.proximus.be";
const char gprsUser[] = "";
const char gprsPass[] = "";
const char gprsserver[] = "prod-01.westeurope.logic.azure.com";
const int port = 80;
#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
//.............................................................................................


//TIME AND NORMES CONTENTS.....................................................
unsigned long lastSMSTime = 0;
const unsigned long intervalBetweenSMS = 30000;
unsigned long last_ok_time = 0;
const unsigned long minIntervalAfterOK = 300000;

bool Fluctuation_flag = false; 
// Définition des plages de mesure
const int plageA_min = 10;
const int plageA_max = 100;
const int plageDiff_min = 5;
const int plageDiff_max = 50;
//....................................................................................


//Permet à la boucle de s'exécuter toutes les 10 sec.....................
unsigned long lastDataSendTime = 0; // Variable pour stocker le moment de la dernière envoi
unsigned long lastSDWriteTime = 0; 
unsigned long lastBlynkSendTime = 0;
//...............................................................


//EEPROM VARIABLES.................................
String global_DeviceName;
String global_DeviceNumber;
String sms_target;
//..................................................


//ALL FUNCTIONS...........................................................................................................
void getFormattedDateTime();
void randomvalue();
void sendBlynk(float temperatureIN, float temperatureOUT, float debit);
void receive_SMS();
void readBatteryVoltage();


//MOBILE FUNCTIONS
void modem_setup();
bool send_SMS(const String& phoneNumber, const String& dayStamp, const String& timeStamp, float debit, float temperatureIN, float temperatureOUT, const String& global_DeviceName, const String& global_DeviceNumber);
//.............................................................................................................................


void setup() {

  // Initialize serial communication first
  Serial.begin(115200);
  Oled_manager.init();
  Temp_manager.begin();
  Oled_manager.writemessage("Temp sensor initialing...");
  delay(1000);
  Oled_manager.writemessage("Temp sensor initialized!");
  delay(1000);
 

  // Other initialization tasks
  EEPROM.begin(512);
  Oled_manager.writemessage("Setup EEPROM Memory!");

  delay(1000);

  //CLASS....................
  SD_Manager SD_manager;
  SD_manager.sd_init(); 
  Oled_manager.writemessage("SD Initialized!");
  delay(1000);
  modem_setup();
  delay(2000);

// CONNEXION AU WIFI.....................
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wifi.");
  Oled_manager.writemessage("Connecting to Wifi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
  Oled_manager.writemessage("WiFi connected!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
  delay(2000);
//.......................................


//WEBSERVER Functions...................................
  WebServer_manager.begin();
  WebServer_manager.webInterface();
  WebServer_manager.webLogic();
  Oled_manager.writemessage("Web server online!");
//......................................................
  
  Temp_manager.locateDevice();

  randomSeed(analogRead(34)); // Initialisation de la graine pour les valeurs aléatoires
  randomvalue();

//Récupération des variables de l'eeprom................
  Eeprom_manager.get_Eeprom_DeviceName();
  Eeprom_manager.get_Eeprom_DeviceNumber();
  Eeprom_manager.get_Eeprom_PhoneNumber();
//.............................................................

  DateTime_manager.getSIMDateTime();
  // Obtenez les valeurs de date et d'heure sous forme de chaînes de caractères
  const char* date = DateTime_manager.getDate();
  const char* hour = DateTime_manager.getTime();

  SD_manager.write_data(temperatureIN,temperatureOUT,debit,date,hour,global_DeviceName, global_DeviceNumber);
}

void loop() {
  unsigned long currentTime = millis();
  randomvalue();

  global_DeviceName = Eeprom_manager.get_Eeprom_DeviceName();
  global_DeviceNumber = Eeprom_manager.get_Eeprom_DeviceNumber();
  sms_target = Eeprom_manager.get_Eeprom_PhoneNumber();

  // Exécution de Blynk et gestion des minuteries
  Blynk.run();

  //Lecture des température via le ONEWIRE. 
  /*
    Temp_manager.tempReading();
    temperatureIN = Temp_manager.getTemperatureIN();
    temperatureOUT = Temp_manager.getTemperatureOUT();
  */

  // Récupérer la date et l'heure à partir de la classe DateTime_Manager
  DateTime_manager.getSIMDateTime();
  // Obtenez les valeurs de date et d'heure sous forme de chaînes de caractères
  const char* date = DateTime_manager.getDate();
  const char* hour = DateTime_manager.getTime();
  
  if (WiFi.status() == WL_CONNECTED) {
    WebServer_manager.handleClient();
    // Envoi des données toutes les 20 secondes si le WiFi est connecté
    if (currentTime - lastDataSendTime >= 20000 && WiFi.status() == WL_CONNECTED) {
      HttpRequest_manager.sendData(temperatureIN, temperatureOUT, debit, date, hour, global_DeviceName, global_DeviceNumber);
      // Mettre à jour le moment du dernier envoi
      lastDataSendTime = currentTime;
    }
  }


  sendBlynk(temperatureIN, temperatureOUT, debit);
  Oled_manager.oled_data(temperatureIN, temperatureOUT, debit, battery_voltage);

  // Enregistrement des données dans la carte SD toutes les 20 secondes
  if (currentTime - lastSDWriteTime >= 20000) {
    SD_Manager SD_manager;
    SD_manager.write_data(temperatureIN,temperatureOUT,debit,date,hour,global_DeviceName, global_DeviceNumber);
    // Mettre à jour le moment de la dernière écriture dans la carte SD
    lastSDWriteTime = currentTime;
  }

  // Appel de la fonction pour recevoir les SMS
  receive_SMS();

  // Condition pour envoyer un SMS
  if (debit < plageA_min || debit > plageA_max || (abs(temperatureOUT - temperatureIN) < plageDiff_min || abs(temperatureOUT - temperatureIN) > plageDiff_max)) {
    // Vérifier si le délai de 5 minutes après la réception du SMS "OK" est écoulé
    if (millis() - last_ok_time >= minIntervalAfterOK) {
      // Envoi de SMS toutes les 30 secondes
      if (millis() - lastSMSTime >= intervalBetweenSMS) {
        send_SMS(sms_target, date, hour, debit, temperatureIN, temperatureOUT, global_DeviceName, global_DeviceNumber);
        // Mettre à jour le moment de l'envoi du SMS
        lastSMSTime = millis();
      }
    }
  }


  readBatteryVoltage();
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
    Oled_manager.writemessage("Start modem...");
    delay(3000);

    while (!modem.testAT()) {
        delay(10);
    }

    // Wait PB DONE
    delay(10000);

    // Unlock SIM PIN
    Serial.println("Unlocking SIM PIN...");
    Oled_manager.writemessage("Unlocking SIM PIN...");
    delay(1000);

    if (modem.simUnlock(PIN)) {
        Serial.println("SIM unlocked successfully!");
        Oled_manager.writemessage("SIM unlocked successfully!");
    } else {
        Serial.println("Failed to unlock SIM!");
        Oled_manager.writemessage("Failed to unlock SIM!");
    }
    delay(1000);

    // Check network registration
    Serial.println("Checking network registration...");
    Oled_manager.writemessage("Checking network registration...");
    if (!modem.waitForNetwork()) {
        Serial.println("Failed to register to network!");
        Oled_manager.writemessage("Failed to register to network!");
        return;
    } else {
        Serial.println("Network registered!");
        Oled_manager.writemessage("Network registered!");
    }
    // Print messages before Blynk initialization
  

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, modem, apn, gprsUser, gprsPass);

}

//Fonction d'envoi d'sms
bool send_SMS(const String& phoneNumber, const String& dayStamp, const String& timeStamp, float debit, float temperatureIN, float temperatureOUT, const String& deviceName, const String& deviceNumber) {

  String message = "Fluctuation detected :"+ dayStamp+"/"+ timeStamp+ " \n\n";
  message += "Appareil : " + global_DeviceName + "\n";
  message += "Numero de serie : " + global_DeviceNumber + "\n";
  message += "Debit : " + String(debit) + "\n";
  message += "Temp IN : " + String(temperatureIN) + "\n";
  message += "Temp OUT : " + String(temperatureOUT) + "\n";
  
  Serial.println("Fluctuation !!");

  // Envoi du SMS et vérification du succès
  bool success = modem.sendSMS(phoneNumber, message);
  if (success) {
    Serial.println("SMS envoyé avec succès !");
  } 
  else {
    Serial.println("Échec de l'envoi du SMS !");
  }
  
  return modem.sendSMS(phoneNumber, message);
}


void receive_SMS() {
  // Envoie la commande AT pour vérifier les SMS non lus
  modem.sendAT(F("AT+CMGL=\"REC UNREAD\""));

  // Attend la réponse du module (le contenu des SMS non lus)
  String response = String((char*)modem.waitResponse());

  // Vérifie si la réponse contient des SMS non lus
  if (response.indexOf("+CMGL:") != -1) {
    // Lecture du premier SMS non lu
    modem.sendAT(F("AT+CMGR=1"));

    // Attend la réponse du module (le contenu du premier SMS non lu)
    String smsContent = String((char*)modem.waitResponse());

    // Supprimer le premier SMS après l'avoir lu
    modem.sendAT(F("AT+CMGD=1"));

    // Traiter le contenu du SMS (par exemple, vérifier si c'est "OK")
    if (smsContent.indexOf("OK") != -1) {
      last_ok_time = millis(); // Mettre à jour le moment de la dernière réception du SMS "OK"
    }
  }
}


void randomvalue() {
  // Génère de nouvelles valeurs aléatoires
  temperatureIN = random(13,15);
  temperatureOUT = random(12, 14);
  debit = random(2000, 2500);
}


void read_EEPROM_Data() {
//Récupération des variables de l'eeprom........................
  global_DeviceName = Eeprom_manager.get_Eeprom_DeviceName();
  global_DeviceNumber = Eeprom_manager.get_Eeprom_DeviceNumber();
  sms_target = Eeprom_manager.get_Eeprom_PhoneNumber();
//.............................................................
}

void sendBlynk(float temperatureIN, float temperatureOUT, float debit){

    Blynk.virtualWrite(V0, temperatureIN);
    Blynk.virtualWrite(V1, temperatureOUT);
    Blynk.virtualWrite(V2, debit);
    Blynk.virtualWrite(V3, battery_voltage);
    delay(100);
}

// Fonction pour lire la tension de la batterie
void readBatteryVoltage() {
    // Configuration de l'ADC
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    
    // Lecture de la tension de la batterie
    battery_voltage = (esp_adc_cal_raw_to_voltage(analogRead(BOARD_BAT_ADC_PIN), &adc_chars) * 2);
    
    // Affichage de la tension de la batterie
    Serial.print("Tension de la batterie : ");
    Serial.print(battery_voltage);
    Serial.println(" mV");
}

