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

void loop() {
  ADCSRA |= (1 << ADSC);
  /* wait until conversion is complete */
  while (!(ADCSRA & (1 << ADIF)));
  uint16_t result = ADC * 2;

  uint8_t brightness = map(result, 0, 1023, 0, 200);
  //Serial.println(brightness);
  FastLED.setBrightness(brightness);

  //Serial.println(result);

  EVERY_N_MILLISECONDS(50) {

    leds[0] = CHSV(hue, random8(), random8(100, 255));

    for (int i = NUM_LEDS - 1; i > 0; i--) {
      leds[i] = leds[i - 1];
    }
  }

  FastLED.show();
}
