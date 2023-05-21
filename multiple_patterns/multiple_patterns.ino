#include "Arduino.h"
#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 80

#define AUDIO_PIN A1

CRGB leds[NUM_LEDS];

uint8_t hue = 160;
uint8_t patternCounter = 0;

CRGBPalette16 purplePalette = CRGBPalette16(
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
  CRGB::Linen);

CRGBPalette16 myPal = purplePalette;

ISR(TIMER1_COMPA_vect) {
  //Serial.println("here");
  hue += 10;
}

void configure_timer() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 31250;           // compare match register 16MHz/256/2Hz-1
  TCCR1B |= (1 << WGM12);  // CTC mode
  TCCR1B |= (1 << CS12);   // 256 prescaler
  TCCR1B |= (1 << CS10);
}

void init_timer() {
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
}

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(50);
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

const uint8_t cooling = 55;
const uint8_t sparking = 120;
bool gReverseDirection = false;

void loop() {
  ADCSRA |= (1 << ADSC);
  /* wait until conversion is complete */
  while (!(ADCSRA & (1 << ADIF)))
    ;
  uint16_t result = ADC * 2;

  switch (patternCounter) {
    case 0:
      movingDots();
      break;
    case 1:
      rainbowBeat();
      break;
    case 2:
      redWhiteBlue();
      break;
  }

  EVERY_N_SECONDS(5) {
    nextPattern();
  }
  
  FastLED.show();

  FastLED.show();
}

void audioReactive(uint16_t result) {
  uint16_t intensity = map(result, 0, 500, 0, NUM_LEDS); 
  // Map the audio input to number of LEDs

  fill_solid(leds, NUM_LEDS, CRGB::Black); 
  // Fill the strip with black/off pixels

  for(int i = 0; i < intensity; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
  // Fill the LEDs up to the audio intensity with color from the palette
  
  // If the audio intensity is less than the total number of LEDs, 
  // we create a gradient from the intensity point to the end of the strip
  // if(intensity < NUM_LEDS) {
  //   for(int i = intensity; i < NUM_LEDS; i++) {
  //     // Linear interpolation between the color at the intensity point and black
  //     leds[i] = blend(leds[intensity], CRGB::Black, ((i-intensity) * 255) / (NUM_LEDS - intensity));
  //   }
  // }
}

void fireLoop() {
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((cooling * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < sparking ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor( heat[j]);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (NUM_LEDS-1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

void nextPattern() {
  patternCounter = (patternCounter + 1) % 3;  // Change the number after the % to the number of patterns you have
}

//------- Put your patterns below -------//

void movingDots() {

  uint16_t posBeat = beatsin16(30, 0, NUM_LEDS - 1, 0, 0);
  uint16_t posBeat2 = beatsin16(60, 0, NUM_LEDS - 1, 0, 0);

  uint16_t posBeat3 = beatsin16(30, 0, NUM_LEDS - 1, 0, 32767);
  uint16_t posBeat4 = beatsin16(60, 0, NUM_LEDS - 1, 0, 32767);

  // Wave for LED color
  uint8_t colBeat = beatsin8(45, 0, 255, 0, 0);

  leds[(posBeat + posBeat2) / 2] = CHSV(colBeat, 255, 255);
  leds[(posBeat3 + posBeat4) / 2] = CHSV(colBeat, 255, 255);

  fadeToBlackBy(leds, NUM_LEDS, 10);
}


void rainbowBeat() {

  uint16_t beatA = beatsin16(30, 0, 255);
  uint16_t beatB = beatsin16(20, 0, 255);
  fill_rainbow(leds, NUM_LEDS, (beatA + beatB) / 2, 8);
}


void redWhiteBlue() {

  uint16_t sinBeat = beatsin16(30, 0, NUM_LEDS - 1, 0, 0);
  uint16_t sinBeat2 = beatsin16(30, 0, NUM_LEDS - 1, 0, 21845);
  uint16_t sinBeat3 = beatsin16(30, 0, NUM_LEDS - 1, 0, 43690);

  leds[sinBeat] = CRGB::Blue;
  leds[sinBeat2] = CRGB::Red;
  leds[sinBeat3] = CRGB::White;

  fadeToBlackBy(leds, NUM_LEDS, 10);
}
