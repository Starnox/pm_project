#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 80

#define AUDIO_PIN A1

CRGB leds[NUM_LEDS];

uint8_t hue = 160;

ISR(TIMER1_COMPA_vect) {
  //Serial.println("here");
  hue += 10;
}

void configure_timer() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 31250;            // compare match register 16MHz/256/2Hz-1
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TCCR1B |= (1 << CS10);
}

void init_timer() {
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
}

CRGBPalette16 purplePalette = CRGBPalette16 (
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    
    CRGB::Magenta,
    CRGB::Magenta,
    CRGB::Linen,
    CRGB::Linen,
    
    CRGB::Magenta,
    CRGB::Magenta,
    CRGB::DarkViolet,
    CRGB::DarkViolet,

    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::Linen,
    CRGB::Linen
);

CRGBPalette16 myPal = purplePalette;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(25);
  FastLED.setCorrection(TypicalPixelString);

  // configure A1 as input pin and enable adc -> lab4
  ADMUX = 0;
  /* ADC1 - channel 1 */
  ADMUX |= (1 << MUX0);
  /* AVCC with external capacitor at AREF pin */
  ADMUX |= (1 << REFS0);

  ADCSRA = 0;
  /* set prescaler at 128 */
  ADCSRA |= (7 << ADPS0);
  /* enable ADC */
  ADCSRA |= (1 << ADEN);

  // configure timer and intrerrupt every 2 seconds
  cli();
  configure_timer();
  init_timer();
  sei();

  Serial.begin(9600);
}
uint8_t paletteIndex = 0;

void loop() {
  ADCSRA |= (1 << ADSC);
  /* wait until conversion is complete */
  while (!(ADCSRA & (1 << ADIF)));
  uint16_t result = ADC * 2;

  uint8_t brightness = map(result, 0, 1023, 0, 200);
  //Serial.println(brightness);
  // FastLED.setBrightness(brightness);

  // EVERY_N_MILLISECONDS(50){
  //   //Switch on an LED at random, choosing a random color from the palette
  //   leds[random8(0, NUM_LEDS - 1)] = ColorFromPalette(myPal, random8(), 255, LINEARBLEND);
  // }

  //  // Fade all LEDs down by 1 in brightness each time this is called
  //  fadeToBlackBy(leds, NUM_LEDS, 1);
  
  // Waves for LED position
  uint8_t posBeat  = beatsin8(30, 0, NUM_LEDS - 1, 0, 0);
  uint8_t posBeat2 = beatsin8(60, 0, NUM_LEDS - 1, 0, 0);
  uint8_t posBeat3 = beatsin16(30, 0, NUM_LEDS - 1, 0, 127);
  uint8_t posBeat4 = beatsin16(60, 0, NUM_LEDS - 1, 0, 127);


  // In the video I use beatsin8 for the positions. For longer strips,
  // the resolution isn't high enough for position and can lead to some
  // LEDs not lighting. If this is the case, use the 16 bit versions below
  // uint16_t posBeat  = beatsin16(30, 0, NUM_LEDS - 1, 0, 0);
  // uint16_t posBeat2 = beatsin16(60, 0, NUM_LEDS - 1, 0, 0);
  // uint16_t posBeat3 = beatsin16(30, 0, NUM_LEDS - 1, 0, 32767);
  // uint16_t posBeat4 = beatsin16(60, 0, NUM_LEDS - 1, 0, 32767);

  // Wave for LED color
  uint8_t colBeat  = beatsin8(45, 0, 255, 0, 0);

  leds[(posBeat + posBeat2) / 2]  = CHSV(colBeat, 255, 255);
  leds[(posBeat3 + posBeat4) / 2]  = CHSV(colBeat, 255, 255);

  fadeToBlackBy(leds, NUM_LEDS, 10);
  FastLED.show();
}
