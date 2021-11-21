#include <IoTGuru.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  ADC_MODE(ADC_VCC);
#endif

#ifdef ESP32
  #include <WiFi.h>
#endif

#define AP0_SSID       "iotguru.cloud"
#define AP0_PASSWORD   "********"
#define AP1_SSID       "iotguru.cloud"
#define AP1_PASSWORD   "********"
#define AP2_SSID       "iotguru.cloud"
#define AP2_PASSWORD   "********"

WiFiClient client;

const char* ota_version = "smart-room-1.0.0";

String userShortId      = "uuuuuuuuuuuuuuuuuuuuuu";
String deviceShortId    = "dddddddddddddddddddddd";
String deviceKey        = "zzzzzzzzzzzzzzzzzzzzzz";
IoTGuru iotGuru = IoTGuru(userShortId, deviceShortId, deviceKey);

String envNodeShortId   = "eeeeeeeeeeeeeeeeeeeeee";
String lgtNodeShortId   = "llllllllllllllllllllll";
String sysNodeShortId   = "ssssssssssssssssssssss";

#define D0             16

#define D1             5
#define D2             4
#define D3             0
#define D4             2

#define D5             14
#define D6             12
#define D7             13
#define D8             15

volatile int PIN_PIR    = D2;
volatile int PIN_SDA    = D3;
volatile int PIN_SCL    = D4;
volatile int PIN_POWER  = D5;
volatile int PIN_PWM    = D8;

/**
 * BME280 sensor.
 */
#include <BME280_MOD-1022.h>
#include <Wire.h>

volatile double temp;
volatile double humidity;
volatile double pressure;

/**
 * PWM output values.
 */
#define PWM_MAX_VALUE  512
#define PWM_MAX_FREQ   256
volatile int pwmBrightnessLookup[PWM_MAX_VALUE + 1];
volatile int pwmMaxPower = PWM_MAX_VALUE;
volatile int pwmCurrentPower = 0;
volatile int pwmTargetPower = -1;
volatile int pwmPrevTargetPower = -1;
volatile int pwmTargetDirty = 0;

/**
 * PIR sensor values.
 */
volatile int pirState = 0;
volatile int pirDirty = 0;
volatile unsigned long pirLastChange = 0;
volatile unsigned long pirSwitchDuration = 300000;

/**
 * Wifi connection timestamps.
 */
volatile unsigned long wifiLastConnected = 0;
volatile unsigned long wifiConnectionTimeout = 10000;
volatile unsigned long wifiLastConnectionStarted = 0;

void setup() {
    Serial.begin(115200);
    delay(10);

    /**
     * Wait for the first WiFi connection to do HTTP OTA firmware update.
     */
    do {
        delay(1);
        wifiConnect();
    } while (wifiLastConnected == 0 && millis() < wifiConnectionTimeout * 3);

    iotGuru.setCallback(&callback);
    iotGuru.setCheckDuration(60000);
    iotGuru.setDebugPrinter(&Serial);
    iotGuru.setNetworkClient(&client);

    iotGuru.firmwareUpdate(ota_version);

    pinMode(PIN_SDA, INPUT);     // BME280 SCA
    pinMode(PIN_SCL, INPUT);     // BME280 SCL
    pinMode(PIN_PIR, INPUT);     // PIR
    pinMode(PIN_POWER, OUTPUT);  // BME280 power

    pinMode(PIN_PWM, OUTPUT);    // PWM dimmer output
    analogWriteRange(PWM_MAX_VALUE);
    analogWriteFreq(PWM_MAX_FREQ);

    /**
     * Calculate the non-linear brightness of PWM output.
     */
    float R = (PWM_MAX_VALUE * log10(2)) / log10(PWM_MAX_VALUE);
    for (int i = 0; i <= PWM_MAX_VALUE; i++) {
        int brightness = pow(2, (i / R));
        pwmBrightnessLookup[i] = brightness - 1;
    }
    pwmBrightnessLookup[PWM_MAX_VALUE - 1] = PWM_MAX_VALUE;
    pwmBrightnessLookup[PWM_MAX_VALUE] = PWM_MAX_VALUE;

    attachInterrupt(PIN_PIR, handlePirPin, CHANGE);

    /**
     * Attach PWM and external watchdog interrupt.
     */
    noInterrupts();
    timer0_isr_init();
    timer0_attachInterrupt(updatePWM);
    timer0_write(ESP.getCycleCount() / 833200 * 833200 + 833200);
    interrupts();
}

/**
 * Fade the output to the specified power.
 */
ICACHE_RAM_ATTR void updatePWM() {
    timer0_write(ESP.getCycleCount() / 833200 * 833200 + 833200);

    if (pwmTargetPower > pwmCurrentPower) {
        pwmCurrentPower += 1;
    }
    if (pwmTargetPower < pwmCurrentPower) {
        pwmCurrentPower -= 1;
    }

    if (pwmCurrentPower <= 0) {
        digitalWrite(PIN_PWM, LOW);
    } else if (pwmCurrentPower >= PWM_MAX_VALUE) {
        digitalWrite(PIN_PWM, HIGH);
    } else {
        analogWrite(PIN_PWM, pwmBrightnessLookup[pwmCurrentPower]);
    }

    if (pwmTargetPower != -1) {
        if (pirState == 0) {
            if (pirLastChange + pirSwitchDuration < millis() && pwmTargetPower > pwmPrevTargetPower / 4 * 3) {
                pwmPrevTargetPower = pwmTargetPower;
                pwmTargetPower = pwmPrevTargetPower / 4 * 3;
                pwmTargetDirty = 1;
            } else if (pirLastChange + pirSwitchDuration * 2 < millis()) {
                pwmTargetPower = -1;
                pwmTargetDirty = 1;
                pwmPrevTargetPower = -1;
            }
        }
    }
}

/**
 * Handle the PIR sensor output.
 */
ICACHE_RAM_ATTR void handlePirPin() {
    int pin = digitalRead(PIN_PIR);
    pirLastChange = millis();
    pirDirty = 1;

    if (pin == HIGH && pirState == LOW) {
        pirState = HIGH;
        pwmTargetPower = 512;
        pwmTargetDirty = 1;
    } else if (pin == LOW && pirState == HIGH) {
        pirState = LOW;
    }
}

/**
 * Connect to the WiFi.
 */
void wifiConnect() {
    if (WiFi.status() == WL_CONNECTED) {
        if (wifiLastConnected == 0) {
            wifiLastConnected = millis();
            wifiLastConnectionStarted = 0;

            Serial.print("Connected to the WiFi, device URL: ");
            Serial.print("http://");
            Serial.print(WiFi.localIP());
            Serial.println("/");
        }

        return;
    }

    wifiLastConnected = 0;
    if (wifiLastConnectionStarted == 0) {
        wifiLastConnectionStarted = millis();
        wifiConnect(AP0_SSID, AP0_PASSWORD);
    }

    if ((millis() - wifiLastConnectionStarted) > wifiConnectionTimeout) {
        wifiLastConnectionStarted = millis();
        int selector = wifiLastConnectionStarted % (wifiConnectionTimeout * 3) / wifiConnectionTimeout;
        switch (selector) {
            case 0: wifiConnect(AP0_SSID, AP0_PASSWORD); break;
            case 1: wifiConnect(AP1_SSID, AP1_PASSWORD); break;
            case 2: wifiConnect(AP2_SSID, AP2_PASSWORD); break;
        }
    }
}

/**
 * Connect to the WiFi.
 */
void wifiConnect(char* ssid, char* password) {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi [ssid=" + String(ssid) + "]");
}

volatile unsigned long nextSend = 0;

void loop() {
    wifiConnect();
    iotGuru.loop();
    if (iotGuru.check(ota_version)) {
        ESP.restart();
    }

    if (nextSend < millis()) {
        if (nextSend + 60000 < millis()) {
            nextSend = (millis() / 60000 + 1) * 60000;
        } else {
            nextSend = nextSend + 60000;
        }

        iotGuru.sendMqttValue(sysNodeShortId, "vcc", ESP.getVcc() / 1.0);
        iotGuru.sendMqttValue(sysNodeShortId, "uptime", millis() / 1000.0);

        if (readBME280() == 1) {
            iotGuru.sendMqttValue(envNodeShortId, "temperature", temp);
            iotGuru.sendMqttValue(envNodeShortId, "humidity", humidity);
            iotGuru.sendMqttValue(envNodeShortId, "pressure", pressure);
        } else {
            Serial.println("Could not read values from BME280 sensor.");
        }

        iotGuru.sendMqttValue(lgtNodeShortId, "pir", pirState);
        iotGuru.sendMqttValue(lgtNodeShortId, "led_pwm", pwmTargetPower);
    }

    if (pirDirty) {
        pirDirty = 0;
        iotGuru.sendMqttValue(lgtNodeShortId, "pir", pirState);
    }
    if (pwmTargetDirty) {
        pwmTargetDirty = 0;
        iotGuru.sendMqttValue(lgtNodeShortId, "led_pwm", pwmTargetPower);
    }
}

void callback(const char* nodeShortId, const char* fieldName, const char* message) {
    Serial.print(nodeShortId);Serial.print(" - ");Serial.print(fieldName);Serial.print(": ");Serial.println(message);
    if (String(nodeShortId) == lgtNodeShortId && String(fieldName) == "led_pwm") {
        Serial.println(message);
        pwmTargetPower = String(message).toInt();
    }
}

int readBME280() {
    digitalWrite(PIN_POWER, HIGH);
    delay(5);

    Wire.begin(PIN_SDA, PIN_SCL);
    uint8_t chipID = BME280.readChipId();

    Serial.print("ChipID = 0x");
    Serial.println(chipID, HEX);

    if (chipID != 0x60) {
        Serial.println("Not found BME280 sensor.");
        digitalWrite(PIN_POWER, LOW);
        return -1;
    }

    BME280.readCompensationParams();
    BME280.writeOversamplingPressure(os16x);
    BME280.writeOversamplingTemperature(os16x);
    BME280.writeOversamplingHumidity(os16x);
    BME280.writeMode(smForced);

    int result = repeatedBMERead();
    BME280.writeMode(smNormal);
    digitalWrite(PIN_POWER, LOW);

    Serial.println("Temperature  = " + String(temp));
    Serial.println("Humidity     = " + String(humidity));
    Serial.println("Pressure     = " + String(pressure));

    if (result == 0) {
        Serial.println("Cannot read valid measurements...");
        return 0;
    }

    return 1;
}

int repeatedBMERead() {
    for (int i = 0; i < 10; i++) {
        while (BME280.isMeasuring()) {
            delay(1);
        }

        noInterrupts();
        BME280.readMeasurements();
        interrupts();

        temp = BME280.getTemperature();
        humidity = BME280.getHumidity();
        pressure = BME280.getPressure();

        BME280.writeStandbyTime(tsb_0p5ms);
        BME280.writeFilterCoefficient(fc_16);

        if (pressure > 800 && pressure < 1200) {
            return 1;
        }

        delay(10);
    }

    return 0;
}
