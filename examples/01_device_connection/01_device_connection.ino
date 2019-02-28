/**
 * 01. Device connection
 *
 * Periodically checks in to the IoT Guru cloud (where you can set device offline alert).
 *
 * You need:
 * - user short identifier (you can find it on the Account page)
 * - the device short identifier (you can find it on the Device page)
 * - the device key (you can find it on the Device page)
 */

#include <IoTGuru.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif

#ifdef ESP32
  #include <WiFi.h>
#endif

const char* ssid      = "ssid";
const char* password  = "password";

String userShortId    = "xxxxxxxxxxxxxxxxxxxxxx";
String deviceShortId  = "yyyyyyyyyyyyyyyyyyyyyy";
String deviceKey      = "zzzzzzzzzzzzzzzzzzzzzz";
IoTGuru iotGuru = IoTGuru(userShortId, deviceShortId, deviceKey);

void setup() {
    Serial.begin(115200);
    delay(10);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
}

void loop() {
    iotGuru.check();
}
