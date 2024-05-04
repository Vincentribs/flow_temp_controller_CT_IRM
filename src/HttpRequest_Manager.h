#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class HttpRequest_Manager {
private:
    const char* serverUrl;

public:
    HttpRequest_Manager(const char* url) : serverUrl(url) {}

    void sendData(float temperatureIN, float temperatureOUT, float debit, const char* date, const char* hour, String deviceName, String deviceNumber) {
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;

            // Create JSON object
            DynamicJsonDocument jsonDocument(200);
            jsonDocument["temperatureIN"] = temperatureIN;
            jsonDocument["temperatureOUT"] = temperatureOUT;
            jsonDocument["debit"] = debit;
            jsonDocument["date"] = date;
            jsonDocument["hour"] = hour;
            jsonDocument["Device Name"] = deviceName;
            jsonDocument["Device Number"] = deviceNumber;

            // Serialize JSON to string
            String jsonData;
            serializeJson(jsonDocument, jsonData);

            Serial.print("Sending the data ... ");
            Serial.println(jsonData);

            // Start the HTTP Connection.
            http.begin(serverUrl);
            http.addHeader("Content-Type", "application/json");

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
};




/*
if (!modem.waitForNetwork()) {
    Serial.println("Échec de la connexion au réseau");
    return;
  }
  SerialMon.print("Connecting to APN: ");
  SerialMon.println(apn);

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
  }

  Serial.println("Connecté au réseau GPRS");

  // Créez un objet client GSM pour la connexion
  TinyGsmClient client(modem);

  // Initialisez l'objet HttpClient
  HttpClient http(client, "", 443);

  // URL du serveur
  String url = "https://prod-01.westeurope.logic.azure.com:443/workflows/7d254d10055740139d924de23ddad9d6/triggers/manual/paths/invoke?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=tMdfU4rTNOX6Yjz-56Ap5Qn08llK16QJXVtuB0OvvlA";

  // Créer un objet JSON
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["temperatureIN"] = valeur1;
  jsonDocument["temperatureOUT"] = valeur2;
  jsonDocument["debit"] = valeur3;
  jsonDocument["date"] = valeur4;
  jsonDocument["hour"] = valeur5;
  jsonDocument["Device Name"] = valeur6;
  jsonDocument["Device Number"] = valeur7;

  // Sérialisez JSON en chaîne
  String jsonData;
  serializeJson(jsonDocument, jsonData);

  Serial.print("Envoi des données ... ");
  Serial.println(jsonData);

  // Envoyer la demande POST avec les données
  http.beginRequest();
  http.post(url, "application/json", jsonData);
  http.endRequest();

  // Vérifiez la réponse du serveur.
  int statusCode = http.responseStatusCode();
  if (statusCode > 0) {
    Serial.print("Réponse: ");
    Serial.println(statusCode);
    String response = http.responseBody();
    Serial.println(response);
  } else {
    Serial.print("Erreur de requête HTTP: ");
    Serial.println(statusCode);
  }
}
*/