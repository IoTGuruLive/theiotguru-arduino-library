/**
 * 01. Device connection
 *
 * Periodically checks in to the IoT Guru cloud (where you can set device offline alert).
 *
 * You need:
 * - the user's short identifier (you can find it on the Account page)
 * - the device's short identifier (you can find it on the Device page)
 * - the device's key (you can find it on the Device page)
 *
 * Tutorial: https://iotguru.live/tutorials/devices
 *
 * If your device is online and operating, the 'check()' function periodically sends
 * check in messages to our cloud service where you can see the last check in timestamp
 * on the list of your devices page 'Last firmware check' column.
 *
 * Also, you can set device alerts and we will send an emails and Android push
 * messages whether your device goes to down, still down or goes to up state.
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
    Serial.println("");

    /**
     * Set check in duration, the default is 60000 milliseconds.
     */
    iotGuru.setCheckDuration(60000);
    /**
     * Set the debug printer.
     */
    iotGuru.setDebugPrinter(&Serial);
}

void loop() {
    iotGuru.check();
}
