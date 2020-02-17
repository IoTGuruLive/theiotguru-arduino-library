/**
 * 06. WS2812 LED strip color push MQTT example
 *
 * You need:
 * - the user's short identifier (you can find it on the Account page)
 * - the device's short identifier (you can find it on the Device page)
 * - the device's key (you can find it on the Device page)
 * - the node's short identifier (you can find it on the Node page)
 * - the field's name (you can find it on the Field page)
 *
 * Tutorial: https://iotguru.live/tutorials/mqtt
 *
 * Example page: https://github.com/IoTGuruLive/web-api-examples/blob/master/push-color.html
 */
#include <IoTGuru.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif

#ifdef ESP32
  #include <WiFi.h>
#endif

const char* ssid      = "ssid";
const char* password  = "password";

WiFiClient client;

String userShortId    = "uuuuuuuuuuuuuuuuuuuuuu";
String deviceShortId  = "dddddddddddddddddddddd";
String deviceKey      = "zzzzzzzzzzzzzzzzzzzzzz";
IoTGuru iotGuru = IoTGuru(userShortId, deviceShortId, deviceKey);

String nodeShortId    = "nnnnnnnnnnnnnnnnnnnnnn";
String fieldName      = "push";

#include <Adafruit_NeoPixel.h>

#define LED_PIN 4
#define NUM_PIXELS 60 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

int red[NUM_PIXELS + 2];
int green[NUM_PIXELS + 2];
int blue[NUM_PIXELS + 2];
int pushed = 0;

void setup()
{
    Serial.begin(115200);
    delay(10);

    pixels.begin();
    for (int i = 0; i < NUM_PIXELS; i++) {
        red[i] = 0;
        green[i] = 0;
        blue[i] = 0;
    }
    showPixels();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
    Serial.println("");

    /**
     * Set the callback function.
     */
    iotGuru.setCallback(&callback);
    /**
     * Set the debug printer (optional).
     */
    iotGuru.setDebugPrinter(&Serial);
    /**
     * Set the network client.
     */
    iotGuru.setNetworkClient(&client);
}

void loop()
{
    iotGuru.loop();
    delay(1);
}

void showPixels() {
    for (int i = 0; i < NUM_PIXELS; i++ ) {
        pixels.setPixelColor(i, pixels.Color(red[i],green[i],blue[i]));
    }
    pixels.show();
}

void callback(const char* nodeShortId, const char* fieldName, const char* message) {
    Serial.print(nodeShortId);Serial.print(" - ");Serial.print(fieldName);Serial.print(": ");Serial.println(message);

    if (strlen(message) != 6) {
        Serial.println("Message is not a color hex code...");
        return;
    }

    int number = (int) strtol(message, NULL, 16);
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

    Serial.print("Red: ");Serial.print(r);Serial.print(", green: ");Serial.print(g);Serial.print(", blue: ");Serial.println(b);

    for (int i = pushed; i >= 0; i--) {
        red[i + 1] = red[i];
        green[i + 1] = green[i];
        blue[i + 1] = blue[i];
    }

    if (pushed < NUM_PIXELS) {
        pushed++;
    }

    red[0] = r;
    green[0] = g;
    blue[0] = b;

    showPixels();
}
