# The IoT Guru Arduino library

## Summary

This is an Arduino library for ESP8266/ESP32 to provide integration with [The IoT Guru cloud](https://iotguru.live) services.

- Support forum: [https://www.facebook.com/groups/theiotguru/](https://www.facebook.com/groups/theiotguru/)
- Facebook page: [https://www.facebook.com/IoTGuruLive/](https://www.facebook.com/IoTGuruLive/)
- Twitter page: [https://twitter.com/IoTGuruLive](https://twitter.com/IoTGuruLive)
- Web page: [https://iotguru.live](https://iotguru.live)

## Examples

- Device connection example: [01_device_connection.ino](https://github.com/IoTGuruLive/theiotguru-arduino-library/blob/master/examples/01_device_connection/01_device_connection.ino)
- Send measurement example: [02_send_measurement.ino](https://github.com/IoTGuruLive/theiotguru-arduino-library/blob/master/examples/02_send_measurement/02_send_measurement.ino)
- Send and receive MQTT messages: [03_mqtt_send_and_callback.ino](https://github.com/IoTGuruLive/theiotguru-arduino-library/blob/master/examples/03_mqtt_send_and_callback/03_mqtt_send_and_callback.ino)
- OTA firmware update: [04_ota_firmware_update.ino](https://github.com/IoTGuruLive/theiotguru-arduino-library/blob/master/examples/04_ota_firmware_update/04_ota_firmware_update.ino)

## Constructors

```IoTGuru(String userShortId, String deviceShortId, String deviceKey);```

## Methods

### Set callback function

```IoTGuru* setCallback( void (*callback)(const char*, const char*, const char*) );```

### Set check duration

```IoTGuru* setCheckDuration(unsigned long checkDuration);```

### Set debug printer

```IoTGuru* setDebugPrinter(HardwareSerial* debugPrinter);```

### Set network client

```IoTGuru* setNetworkClient(Client* client);```

### Check in

```boolean check();```

```boolean check(const char* ota_version);```

### Firmware OTA update

```boolean firmwareUpdate(const char* ota_version);```

### Loop (need to call in the loop)

```boolean loop();```

### Send float value

```boolean sendHttpValue(String nodeShortId, String fieldName, float value);```

```boolean sendMqttValue(String nodeShortId, String fieldName, float value);```
