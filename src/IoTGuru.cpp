/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */

#include "IoTGuru.h"

IoTGuru::IoTGuru(String userShortId, String deviceShortId, String deviceKey) {
    this->userShortId = userShortId;
    this->deviceShortId = deviceShortId;
    this->deviceKey = deviceKey;
}

boolean IoTGuru::handle() {
    return true;
}
