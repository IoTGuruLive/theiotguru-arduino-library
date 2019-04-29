/**
 * 04. OTA firmware update
 *
 * Periodically checks in to the IoT Guru cloud and update the firmware to target one if
 * the current version is not equals.
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
 * You can upload your firmwares: https://iotguru.live/firmwares
 *
 * Set the "Device firmware version" field of your device and our library will
 * update your device if the reported firmware version and the target firmware
 * version is not equals.
 */
#include <IoTGuru.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif

const char* ssid        = "ssid";
const char* password    = "password";

const char* ota_version = "example-1.0.1";

String userShortId      = "xxxxxxxxxxxxxxxxxxxxxx";
String deviceShortId    = "yyyyyyyyyyyyyyyyyyyyyy";
String deviceKey        = "zzzzzzzzzzzzzzzzzzzzzz";
IoTGuru iotGuru         = IoTGuru(userShortId, deviceShortId, deviceKey);

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

    /**
     * Check new firmware and try to update during the clean boot.
     */
    iotGuru.firmwareUpdate(ota_version);
}

void loop() {
    if (iotGuru.check(ota_version)) {
        ESP.restart();
    }
}
