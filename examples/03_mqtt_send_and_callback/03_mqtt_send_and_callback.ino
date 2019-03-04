/**
 * 03. MQTT send and callback example
 *
 * Send MQTT messages to the IoT Guru cloud and register a callback function to handle incoming messages.
 *
 * You need:
 * - user short identifier (you can find it on the Account page)
 * - the device short identifier (you can find it on the Device page)
 * - the device key (you can find it on the Device page)
 *
 * Tutorial: https://iotguru.live/tutorials/devices
 *
 * Also you need:
 * - the node's key (you can find it on the Node page)
 * - the field name (you can find it ont the Field page)
 *
 * Tutorial: https://iotguru.live/tutorials/nodes
 * Tutorial: https://iotguru.live/tutorials/fields
 *
 * You can push MQTT messages to your device by using our API:
 *
 *     https://api.iotguru.live/mqtt/send/{nodeKey}/{fieldName}/{message}
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

WiFiClient client;

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
     * Set the callback function.
     */
    iotGuru.setCallback(&callback);
    /**
     * Set the debug printer (optional).
     */
    iotGuru.setDebugPrinter(&Serial);
    /**
     * Set the network client.
     */
    iotGuru.setNetworkClient(&client);
}

volatile unsigned long nextSendUptime = 0;

void loop() {
    iotGuru.loop();

    if (nextSendUptime < millis()) {
        nextSendUptime = millis() + 60000;
        iotGuru.sendMqttValue(nodeKey, fieldName, millis()/1000.0f);
    }
}

void callback(const char* nodeShortId, const char* fieldName, const char* message) {
    Serial.print(nodeShortId);Serial.print(" - ");Serial.print(fieldName);Serial.print(": ");Serial.println(message);
}

