/**
 * 05. Relay switch from Android client via MQTT connection
 *
 * You need:
 * - the user's short identifier (you can find it on the Account page)
 * - the device's short identifier (you can find it on the Device page)
 * - the device's key (you can find it on the Device page)
 * - the node's short identifier (you can find it on the Node page)
 * - the field's name (you can find it on the Field page)
 *
 * Tutorial: https://iotguru.live/tutorials/mqtt
 *
 * If your device is online and operating, the 'loop()' function periodically sends
 * check in messages to our cloud service where you can see the last check in timestamp
 * on the list of your devices page 'Last firmware check' column.
 *
 * The `callback` method called every time when an MQTT message arrive. We are process it
 * and control the relay shield.
 */
#include <IoTGuru.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif

#define RELAY_PIN         5

/**
 * WiFi parameters.
 */
const char* ssid        = "ssid";
const char* password    = "password";
WiFiClient client;

/**
 * Initialize the connection with the cloud.
 */
String userShortId      = "xxxxxxxxxxxxxxxxxxxxxx";
String deviceShortId    = "yyyyyyyyyyyyyyyyyyyyyy";
String deviceKey        = "zzzzzzzzzzzzzzzzzzzzzz";
IoTGuru iotGuru         = IoTGuru(userShortId, deviceShortId, deviceKey);

/**
 * Constants of the MQTT channel check.
 */
String nodeShortId      = "nnnnnnnnnnnnnnnnnnnnnn";
String fieldName        = "relay";

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
     * Set the relay pin.
     */
    pinMode(RELAY_PIN, OUTPUT);

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

void loop() {
    iotGuru.loop();
}

void callback(const char* cbNodeShortId, const char* cbFieldName, const char* message) {
    Serial.print(cbNodeShortId);Serial.print(" - ");Serial.print(cbFieldName);Serial.print(": ");Serial.println(message);

    if (strcmp(cbNodeShortId, nodeShortId.c_str()) == 0) {
        if (strcmp(cbFieldName, fieldName.c_str()) == 0) {
            if (strcmp(message, "0") == 0) {
                Serial.println("Switch relay to LOW");
                digitalWrite(RELAY_PIN, LOW);
            } else {
                Serial.println("Switch relay to HIGH");
                digitalWrite(RELAY_PIN, HIGH);
            }
        }
    }
}
