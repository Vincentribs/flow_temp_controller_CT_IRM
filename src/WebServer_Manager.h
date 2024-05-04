#include <WebServer.h>
#include "Eeprom_Manager.h"

class WebServer_Manager {
public:
    WebServer_Manager(int port, Eeprom_Manager& eeprom_manager) : server(port), Eeprom_manager(eeprom_manager) {}

    void begin() {
        server.begin();
    }

    void handleClient() {
        server.handleClient();
    }

    void webInterface() {
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
        server.on("/", HTTP_GET, [&]() {
            server.send(200, "text/html", html_page);
        });
    }

    void webLogic() {
        server.on("/send", HTTP_POST, [&]() {
            String deviceName = server.arg("deviceName");
            String deviceNumber = server.arg("deviceNumber");
            String phoneNumber = server.arg("phoneNumber");

            Eeprom_manager.save_to_eeprom(deviceName.c_str(), deviceNumber.c_str(), phoneNumber.c_str());

            Serial.print("Device Name: ");
            Serial.println(deviceName);
            Serial.print("Device Number: ");
            Serial.println(deviceNumber);
            Serial.print("Device Name: ");
            Serial.println(phoneNumber);
            server.send(200, "text/plain", "Data send ! :)");
        });
    }

private:
    WebServer server;
    const char* html_page;
    Eeprom_Manager Eeprom_manager;
};

