/**
 * Based on the PubSubClient of knolleary from https://github.com/knolleary/pubsubclient
 */
#include "Arduino.h"
#include "MqttClient.h"

#define MQTT_HEADER_VERSION_LENGTH 7

MqttClient::MqttClient() {
    this->state = MQTT_DISCONNECTED;
    setCallback(NULL);
    this->networkClient = NULL;
}

MqttClient::MqttClient(Client* networkClient) {
    this->state = MQTT_DISCONNECTED;
    setCallback(NULL);
    setNetworkClient(networkClient);
}

boolean MqttClient::isConnected() {
    boolean rc;
    if (networkClient == NULL ) {
        rc = false;
    } else {
        rc = (int)networkClient->connected();
        if (!rc) {
            if (this->state == MQTT_CONNECTED) {
                this->state = MQTT_CONNECTION_LOST;
                networkClient->flush();
                networkClient->stop();
            }
        }
    }
    return rc;
}

int MqttClient::getState() {
    return this->state;
}

MqttClient& MqttClient::setCallback(MQTT_CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}

MqttClient& MqttClient::setNetworkClient(Client* networkClient){
    this->networkClient = networkClient;
    return *this;
}

MqttClient& MqttClient::setServer(const char* domain, uint16_t port) {
    this->domain = domain;
    this->port = port;
    return *this;
}

boolean MqttClient::connect(const char *id, const char *user, const char *pass) {
    if (!isConnected()) {
        int result = 0;

        if (domain != NULL) {
            result = networkClient->connect(this->domain, this->port);
        }

        if (result == 1) {
            nextMsgId = 1;
            uint16_t length = MQTT_MAX_HEADER_SIZE;
            unsigned int j;

            uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};

            for (j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) {
                buffer[length++] = d[j];
            }

            uint8_t v = 0x00;
            v = v|0x02;

            if(user != NULL) {
                v = v|0x80;

                if(pass != NULL) {
                    v = v|(0x80>>1);
                }
            }

            buffer[length++] = v;
            buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
            buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);

            CHECK_STRING_LENGTH(length,id)
            length = writeString(id,buffer,length);
            if(user != NULL) {
                CHECK_STRING_LENGTH(length,user)
                length = writeString(user,buffer,length);
                if(pass != NULL) {
                    CHECK_STRING_LENGTH(length,pass)
                    length = writeString(pass,buffer,length);
                }
            }

            write(MQTTCONNECT,buffer,length-MQTT_MAX_HEADER_SIZE);

            lastInActivity = lastOutActivity = millis();

            while (!networkClient->available()) {
                unsigned long t = millis();
                if (t-lastInActivity >= ((int32_t) MQTT_SOCKET_TIMEOUT*1000UL)) {
                    state = MQTT_CONNECTION_TIMEOUT;
                    networkClient->stop();
                    return false;
                }
            }
            uint8_t llen;
            uint16_t len = readPacket(&llen);

            if (len == 4) {
                if (buffer[3] == 0) {
                    lastInActivity = millis();
                    pingOutstanding = false;
                    state = MQTT_CONNECTED;
                    return true;
                } else {
                    state = buffer[3];
                }
            }
            networkClient->stop();
        } else {
            state = MQTT_CONNECT_FAILED;
        }

        return false;
    }

    return true;
}

void MqttClient::disconnect() {
    buffer[0] = MQTTDISCONNECT;
    buffer[1] = 0;
    networkClient->write(buffer,2);
    state = MQTT_DISCONNECTED;
    networkClient->flush();
    networkClient->stop();
    lastInActivity = lastOutActivity = millis();
}

boolean MqttClient::publish(const char* topic, const char* payloadChar) {
    const uint8_t* payload = (const uint8_t*)payloadChar;
    unsigned int plength = strlen(payloadChar);
    boolean retained = false;

    if (isConnected()) {
        if (MQTT_MAX_PACKET_SIZE < MQTT_MAX_HEADER_SIZE + 2+strlen(topic) + plength) {
            return false;
        }
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        length = writeString(topic,buffer,length);
        uint16_t i;
        for (i=0;i<plength;i++) {
            buffer[length++] = payload[i];
        }
        uint8_t header = MQTTPUBLISH;
        if (retained) {
            header |= 1;
        }
        return write(header,buffer,length-MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

boolean MqttClient::subscribe(const char* topic) {
    if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic)) {
        return false;
    }
    if (isConnected()) {
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        buffer[length++] = (nextMsgId >> 8);
        buffer[length++] = (nextMsgId & 0xFF);
        length = writeString((char*)topic, buffer,length);
        buffer[length++] = 1;
        return write(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

/**
 * PRIVATE METHODS
 */
boolean MqttClient::readByte(uint8_t * result) {
    uint32_t previousMillis = millis();
    while(!networkClient->available()) {
        yield();
        uint32_t currentMillis = millis();
        if(currentMillis - previousMillis >= ((int32_t) MQTT_SOCKET_TIMEOUT * 1000)){
            return false;
        }
    }

    *result = networkClient->read();
    return true;
}

boolean MqttClient::readByte(uint8_t * result, uint16_t * index){
    uint16_t current_index = *index;
    uint8_t * write_address = &(result[current_index]);
    if(readByte(write_address)){
        *index = current_index + 1;
        return true;
    }
    return false;
}

uint16_t MqttClient::readPacket(uint8_t* lengthLength) {
    uint16_t len = 0;
    if(!readByte(buffer, &len)) return 0;
    bool isPublish = (buffer[0]&0xF0) == MQTTPUBLISH;
    uint32_t multiplier = 1;
    uint16_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint8_t start = 0;

    do {
        if (len == 5) {
            state = MQTT_DISCONNECTED;
            networkClient->stop();
            return 0;
        }
        if(!readByte(&digit)) return 0;
        buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier *= 128;
    } while ((digit & 128) != 0);
    *lengthLength = len-1;

    if (isPublish) {
        if(!readByte(buffer, &len)) return 0;
        if(!readByte(buffer, &len)) return 0;
        skip = (buffer[*lengthLength+1]<<8)+buffer[*lengthLength+2];
        start = 2;
        if (buffer[0]&MQTTQOS1) {
            skip += 2;
        }
    }

    for (uint16_t i = start;i<length;i++) {
        if(!readByte(&digit)) return 0;
        if (len < MQTT_MAX_PACKET_SIZE) {
            buffer[len] = digit;
        }
        len++;
    }

    if (len > MQTT_MAX_PACKET_SIZE) {
        len = 0;
    }

    return len;
}

boolean MqttClient::loop() {
    if (isConnected()) {
        unsigned long t = millis();
        if ((t - lastInActivity > MQTT_KEEPALIVE*1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE*1000UL)) {
            if (pingOutstanding) {
                this->state = MQTT_CONNECTION_TIMEOUT;
                networkClient->stop();
                return false;
            } else {
                buffer[0] = MQTTPINGREQ;
                buffer[1] = 0;
                networkClient->write(buffer,2);
                lastOutActivity = t;
                lastInActivity = t;
                pingOutstanding = true;
            }
        }
        if (networkClient->available()) {
            uint8_t llen;
            uint16_t len = readPacket(&llen);
            uint16_t msgId = 0;
            uint8_t *payload;
            if (len > 0) {
                lastInActivity = t;
                uint8_t type = buffer[0]&0xF0;
                if (type == MQTTPUBLISH) {
                    if (callback) {
                        uint16_t tl = (buffer[llen+1]<<8)+buffer[llen+2];
                        memmove(buffer+llen+2,buffer+llen+3,tl);
                        buffer[llen+2+tl] = 0;
                        char *topic = (char*) buffer+llen+2;
                        if ((buffer[0]&0x06) == MQTTQOS1) {
                            msgId = (buffer[llen+3+tl]<<8)+buffer[llen+3+tl+1];
                            payload = buffer+llen+3+tl+2;
                            callback(topic,payload,len-llen-3-tl-2);

                            buffer[0] = MQTTPUBACK;
                            buffer[1] = 2;
                            buffer[2] = (msgId >> 8);
                            buffer[3] = (msgId & 0xFF);
                            networkClient->write(buffer,4);
                            lastOutActivity = t;

                        } else {
                            payload = buffer+llen+3+tl;
                            callback(topic,payload,len-llen-3-tl);
                        }
                    }
                } else if (type == MQTTPINGREQ) {
                    buffer[0] = MQTTPINGRESP;
                    buffer[1] = 0;
                    networkClient->write(buffer,2);
                } else if (type == MQTTPINGRESP) {
                    pingOutstanding = false;
                }
            } else if (!isConnected()) {
                return false;
            }
        }
        return true;
    }
    return false;
}

size_t MqttClient::write(uint8_t data) {
    lastOutActivity = millis();
    return networkClient->write(data);
}

size_t MqttClient::write(const uint8_t *buffer, size_t size) {
    lastOutActivity = millis();
    return networkClient->write(buffer,size);
}

size_t MqttClient::buildHeader(uint8_t header, uint8_t* buf, uint16_t length) {
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint16_t len = length;
    do {
        digit = len % 128;
        len = len / 128;
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while(len>0);

    buf[4-llen] = header;
    for (int i=0;i<llen;i++) {
        buf[MQTT_MAX_HEADER_SIZE-llen+i] = lenBuf[i];
    }
    return llen+1;
}

boolean MqttClient::write(uint8_t header, uint8_t* buf, uint16_t length) {
    uint16_t rc;
    uint8_t hlen = buildHeader(header, buf, length);

#ifdef MQTT_MAX_TRANSFER_SIZE
    uint8_t* writeBuf = buf+(MQTT_MAX_HEADER_SIZE-hlen);
    uint16_t bytesRemaining = length+hlen;
    uint8_t bytesToWrite;
    boolean result = true;
    while((bytesRemaining > 0) && result) {
        bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE)?MQTT_MAX_TRANSFER_SIZE:bytesRemaining;
        rc = networkClient->write(writeBuf,bytesToWrite);
        result = (rc == bytesToWrite);
        bytesRemaining -= rc;
        writeBuf += rc;
    }
    return result;
#else
    rc = networkClient->write(buf+(MQTT_MAX_HEADER_SIZE-hlen),length+hlen);
    lastOutActivity = millis();
    return (rc == hlen+length);
#endif
}

uint16_t MqttClient::writeString(const char* string, uint8_t* buf, uint16_t pos) {
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);
    buf[pos-i-1] = (i & 0xFF);
    return pos;
}
