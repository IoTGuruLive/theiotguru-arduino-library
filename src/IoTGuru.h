/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */
#include <Arduino.h>

#define IOT_GURU_BASE_URL              "http://api.iotguru.live/"
#define IOT_GURU_BASE_HOST             "api.iotguru.live"
#define IOT_GURU_MQTT_HOST             "mqtt.iotguru.live"
#define IOT_GURU_CLIENT_VERSION        1.0.0

#include "MqttClient.h"

#ifdef ESP8266
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
#endif
#ifdef ESP32
    #include <HTTPClient.h>
    #include <Update.h>
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define IOT_GURU_CALLBACK_SIGNATURE    std::function<void(const char*, const char*, const char*)> callback
#else
#define IOT_GURU_CALLBACK_SIGNATURE    void (*callback)(const char*, const char*, const char*)
#endif

class IoTGuru {
    private:
        String userShortId;
        String deviceShortId;
        String deviceKey;

        HardwareSerial* debugPrinter;
        Client* networkClient;

#if defined(ESP8266) || defined(ESP32)
        WiFiClient wiFiClient;
#endif
        MqttClient mqttClient;

        volatile unsigned long lastChecked = 0;
        volatile unsigned long lastFirmwareChecked = 0;
        volatile unsigned long checkDuration = 60000;

        volatile unsigned long mqttLastConnected = 0;
        volatile unsigned long mqttReconnectDuration = 5000;

        void debugPrint(String function, int line, String msg);

        IOT_GURU_CALLBACK_SIGNATURE;

        bool mqttConnect();
        bool mqttCallback(char* topic, byte* payload, unsigned int length);

        String getHeaderValue(String header, String headerName);
    public:
        IoTGuru(String userShortId, String deviceShortId, String deviceKey);

        IoTGuru* setCheckDuration(unsigned long checkDuration);
        IoTGuru* setDebugPrinter(HardwareSerial* debugPrinter);
        IoTGuru* setNetworkClient(Client* client);

        bool check();
        bool check(const char* ota_version);
        bool firmwareUpdate(const char* ota_version);
        bool loop();

        bool sendHttpValue(String nodeShortId, String fieldName, float value);
        bool sendMqttValue(String nodeShortId, String fieldName, float value);

        IoTGuru& setCallback(IOT_GURU_CALLBACK_SIGNATURE);
};
