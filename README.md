# The IoT Guru Arduino library

## Summary

This is an Arduino library for ESP8266/ESP32 to provide integration with [The IoT Guru cloud](https://iotguru.live) services.

## Examples

- Device connection example: [01_device_connection.ino](https://github.com/IoTGuruLive/theiotguru-arduino-library/blob/master/examples/01_device_connection/01_device_connection.ino)
- Send measurement example: [02_send_measurement.ino](https://github.com/IoTGuruLive/theiotguru-arduino-library/blob/master/examples/02_send_measurement/02_send_measurement.ino)

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

### Loop (need to call in the loop)

```boolean loop();```

### Send float value

```boolean sendHttpValue(String nodeShortId, String fieldName, float value);```

```boolean sendMqttValue(String nodeShortId, String fieldName, float value);```
