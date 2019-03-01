/**
 * IoTGuru.h - Arduino client of the https://iotguru.live cloud services.
 */
#include <Arduino.h>

#define IOT_GURU_BASE_URL              "http://api.iotguru.live/"
#define IOT_GURU_CLIENT_VERSION        1.0.0

class IoTGuru {
    private:
        String userShortId;
        String deviceShortId;
        String deviceKey;

        HardwareSerial* debugPrinter = NULL;

        volatile unsigned long lastChecked = 0;
        volatile unsigned long checkDuration = 60000;

        void debugPrint(String function, int line, String msg);
    public:
        IoTGuru(String userShortId, String deviceShortId, String deviceKey);

        IoTGuru* setCheckDuration(unsigned long checkDuration);
        IoTGuru* setDebugPrinter(HardwareSerial* debugPrinter);

        boolean check();
};
