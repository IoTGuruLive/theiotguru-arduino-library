/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */
#include "IoTGuru.h"

IoTGuru::IoTGuru(String userShortId, String deviceShortId, String deviceKey) {
    this->userShortId = userShortId;
    this->deviceShortId = deviceShortId;
    this->deviceKey = deviceKey;
}

inline void IoTGuru::debugPrint(String function, int line, String msg) {
    if (this->debugPrinter) {
        debugPrinter->print(millis());
        debugPrinter->print(": {");
        debugPrinter->print(function);
        debugPrinter->print(":");
        debugPrinter->print(line);
        debugPrinter->print("} - ");
        debugPrinter->println(msg);
    }
}

IoTGuru* IoTGuru::setCheckDuration(unsigned long checkDuration) {
    this->checkDuration = checkDuration;

    return this;
}

IoTGuru* IoTGuru::setDebugPrinter(HardwareSerial* debugPrinter) {
    this->debugPrinter = debugPrinter;
    return this;
}

boolean IoTGuru::check() {
    if (lastChecked == 0 || lastChecked + checkDuration < millis()) {
        lastChecked = millis();
    } else {
        return false;
    }

    IOTGURU_DEBUG_PRINT("ENTRY");

    IOTGURU_DEBUG_PRINT("Send request to the cloud");
    HTTPClient httpClient;
    httpClient.useHTTP10(true);
    httpClient.setTimeout(1000);

    httpClient.begin(String(IOT_GURU_BASE_URL) + "firmware/check/" + this->deviceKey);
    int code = httpClient.GET();
    httpClient.end();

    IOTGURU_DEBUG_PRINT("Response received from the cloud (status code " + String(code) + ")");

    IOTGURU_DEBUG_PRINT("EXIT");
    return code == 200;
}

boolean IoTGuru::sendFloatValue(String nodeShortId, String fieldName, float value) {
    IOTGURU_DEBUG_PRINT("ENTRY");

    IOTGURU_DEBUG_PRINT("Send request to the cloud");
    HTTPClient httpClient;
    httpClient.useHTTP10(true);
    httpClient.setTimeout(1000);

    httpClient.begin(String(IOT_GURU_BASE_URL) + "measurement/create/" + nodeShortId + "/" + fieldName + "/" + String(value));
    int code = httpClient.GET();
    httpClient.end();

    IOTGURU_DEBUG_PRINT("Response received from the cloud (status code " + String(code) + ")");

    IOTGURU_DEBUG_PRINT("EXIT");
    return code == 200;
}
