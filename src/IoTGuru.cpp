/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */

#include "IoTGuru.h"

#ifdef ESP8266
  #include <ESP8266HTTPClient.h>
#endif

#ifdef ESP32
  #include <HTTPClient.h>
#endif

IoTGuru::IoTGuru(String userShortId, String deviceShortId, String deviceKey) {
    this->userShortId = userShortId;
    this->deviceShortId = deviceShortId;
    this->deviceKey = deviceKey;
}

IoTGuru* IoTGuru::setCheckDuration(unsigned long checkDuration) {
    this->checkDuration = checkDuration;

    return this;
}

boolean IoTGuru::check() {
    if (lastChecked == 0 || lastChecked + checkDuration < millis()) {
        lastChecked = millis();
    } else {
        return false;
    }

    HTTPClient http;
    http.useHTTP10(true);
    http.setTimeout(8000);

    http.begin(String(IOT_GURU_BASE_URL) + "firmware/check/" + this->deviceKey);
    int code = http.GET();
    http.end();

    return code == 200;
}
