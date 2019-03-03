/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */
#include <Arduino.h>

#define IOT_GURU_BASE_URL              "http://api.iotguru.live/"
#define IOT_GURU_MQTT_HOST             "mqtt.iotguru.live"
#define IOT_GURU_CLIENT_VERSION        1.0.0

#include "MqttClient.h"

#ifdef ESP8266
    #include <ESP8266HTTPClient.h>
#endif
#ifdef ESP32
    #include <HTTPClient.h>
#endif

#define IOTGURU_DEBUG_PRINT(msg) debugPrint(__PRETTY_FUNCTION__, __LINE__, msg)

class IoTGuru {
    private:
        String userShortId;
        String deviceShortId;
        String deviceKey;

        HardwareSerial* debugPrinter;
        Client* networkClient;
        MqttClient mqttClient;

        volatile unsigned long lastChecked = 0;
        volatile unsigned long checkDuration = 60000;
 
        volatile unsigned long mqttLastConnected = 0;
        volatile unsigned long mqttReconnectDuration = 5000;

        void debugPrint(String function, int line, String msg);

        boolean mqttConnect();
        boolean mqttCallback(char* topic, byte* payload, unsigned int length);
    public:
        IoTGuru(String userShortId, String deviceShortId, String deviceKey);

        IoTGuru* setCheckDuration(unsigned long checkDuration);
        IoTGuru* setDebugPrinter(HardwareSerial* debugPrinter);
        IoTGuru* setNetworkClient(Client* client);

        boolean check();
        boolean loop();

        boolean sendHttpValue(String nodeShortId, String fieldName, float value);
        boolean sendMqttValue(String nodeShortId, String fieldName, float value);
};
