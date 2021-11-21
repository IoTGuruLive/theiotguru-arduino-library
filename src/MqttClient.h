#include <Arduino.h>
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"

#define MQTT_VERSION                 4
#define MQTT_MAX_PACKET_SIZE         128
#define MQTT_KEEPALIVE               15
#define MQTT_SOCKET_TIMEOUT          15

#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT                  1 << 4  // Client request to connect to Server
#define MQTTCONNACK                  2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH                  3 << 4  // Publish message
#define MQTTPUBACK                   4 << 4  // Publish Acknowledgment
#define MQTTPUBREC                   5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL                   6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP                  7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE                8 << 4  // Client Subscribe request
#define MQTTSUBACK                   9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE              10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK                 11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ                  12 << 4 // PING Request
#define MQTTPINGRESP                 13 << 4 // PING Response
#define MQTTDISCONNECT               14 << 4 // Client is Disconnecting
#define MQTTReserved                 15 << 4 // Reserved

#define MQTTQOS0                    (0 << 1)
#define MQTTQOS1                    (1 << 1)
#define MQTTQOS2                    (2 << 1)

#define MQTT_MAX_HEADER_SIZE         5

#define CHECK_STRING_LENGTH(l,s)     if (l+2+strlen(s) > MQTT_MAX_PACKET_SIZE) {networkClient->stop();return false;}

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_CALLBACK_SIGNATURE      std::function<void(char*, uint8_t*, unsigned int)> callback
#else
#define MQTT_CALLBACK_SIGNATURE      void (*callback)(char*, uint8_t*, unsigned int)
#endif

class MqttClient: public Print {
private:
    Client* networkClient;
    const char* domain;
    uint16_t port;

    uint8_t buffer[MQTT_MAX_PACKET_SIZE];
    uint16_t nextMsgId;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    bool pingOutstanding;
    int state;

    MQTT_CALLBACK_SIGNATURE;

    uint16_t readPacket(uint8_t*);
    boolean readByte(uint8_t * result);
    boolean readByte(uint8_t * result, uint16_t * index);

    boolean write(uint8_t header, uint8_t* buf, uint16_t length);
    uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos);

    size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length);
public:
    MqttClient();
    MqttClient(Client* networkClient);

    boolean isConnected();
    int getState();

    MqttClient& setCallback(MQTT_CALLBACK_SIGNATURE);
    MqttClient& setNetworkClient(Client* networkClient);
    MqttClient& setServer(const char* domain, uint16_t port);

    boolean connect(const char* id, const char* user, const char* pass);
    void disconnect();

    boolean publish(const char* topic, const char* payload);
    boolean subscribe(const char* topic);

    boolean loop();

    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);
};
