/**
 * 02. Send measurement
 *
 * Send measurements to the IoT Guru cloud service.
 *
 * You need:
 * - the user's short identifier (you can find it on the Account page)
 * - the device's short identifier (you can find it on the Device page)
 * - the device's key (you can find it on the Device page)
 *
 * Tutorial: https://iotguru.live/tutorials/devices
 *
 * Also you need:
 * - the node's key (you can find it on the Node page)
 * - the field name (you can find it ont the Field page)
 *
 * Tutorial: https://iotguru.live/tutorials/nodes
 * Tutorial: https://iotguru.live/tutorials/fields
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

String nodeKey        = "wwwwwwwwwwwwwwwwwwwwww";
String fieldName      = "ffffNNNN";

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

    float measuredValue = 21.00f;
    iotGuru.sendHttpValue(nodeKey, fieldName, measuredValue);

    delay(30000);
}
