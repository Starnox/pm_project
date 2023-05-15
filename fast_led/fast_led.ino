#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 10

CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  // leds[0] = CRGB(255, 0, 0); // RED
  // leds[1] = CRGB(0, 255, 0); // GREEN
  // leds[2] = CRGB(0, 0, 255); // BLUE

  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB(0,  255 - 20 * i, 20 * i);
    FastLED.setBrightness(6* i);
    FastLED.show();
    delay(50);
  }

  for (int i = NUM_LEDS- 1; i >= 0; i--) {
    leds[i] = CRGB(20 * i, 0, 255 - 20 * i);
    FastLED.setBrightness(60 - 2 * i);
    FastLED.show();
    delay(50);
  }

  FastLED.show();
}
