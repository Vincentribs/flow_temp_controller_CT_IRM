#include <Arduino.h>
#include <time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoHttpClient.h>


class DateTime_Manager {
private:
    const char* AT_CCLK_COMMAND = "AT+CCLK?";
    String dateStr;
    String timeStr;

    char date[20];
    char hour[9];

public:
  void getSIMDateTime() {
    // Envoie la commande AT pour obtenir l'heure et la date
    SerialAT.println(AT_CCLK_COMMAND);
    delay(100);

    // Lit et analyse la réponse
    while (SerialAT.available()) {
      String response = SerialAT.readStringUntil('\n');
      if (response.startsWith("+CCLK: ")) {
        // Extrait l'heure et la date de la réponse
        String dateTime = response.substring(7); // Supprime "+CCLK: "

        // Trouve l'index de la virgule séparant la date de l'heure
        int commaIndex = dateTime.indexOf(',');

        // Extrait la date jusqu'à la virgule et enlève les guillemets
        dateStr = dateTime.substring(1, commaIndex - 1);

        // Extrait l'année complète, le mois et le jour de la date
        String year = dateStr.substring(7, 11); // Extraire les quatre chiffres de l'année
        String month = dateStr.substring(3, 5);
        String day = dateStr.substring(0, 2);

        // Formatage de la date dans le bon ordre
        dateStr = day + "/" + month + "/2024" ;

        // Extrait l'heure après la virgule et enlève les guillemets et le fuseau horaire
        timeStr = dateTime.substring(commaIndex + 1, commaIndex + 9); 

        break;
      }
    }
  }

  // Méthode pour récupérer la date sous forme de char*
  const char* getDate() {
    return dateStr.c_str();
  }

  // Méthode pour récupérer l'heure sous forme de char*
  const char* getTime() {
    return timeStr.c_str();
  }


};
