/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */

#include <Arduino.h>

#define IOT_GURU_CLIENT_VERSION        1.0.0

class IoTGuru: public Print {
    private:
        String userShortId;
        String deviceShortId;
        String deviceKey;
    public:
        IoTGuru(String userShortId, String deviceShortId, String deviceKey);
        boolean handle();
};
