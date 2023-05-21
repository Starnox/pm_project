#include "Arduino.h"
#include <FastLED.h>
#include <OneButton.h>

#define LED_PIN 2
#define BUTTON_PIN 3
#define NUM_LEDS 80
#define UPDATES_PER_SECOND 100
#define AUDIO_PIN A1
#define NUM_PATTERNS 8

CRGB leds[NUM_LEDS];

uint8_t hue = 160;
uint8_t patternCounter = 0;

const uint8_t cooling = 55;
const uint8_t sparking = 120;
bool gReverseDirection = false;

int loop_max = 0;
int k = 255; // COLOR WHEEL POSITION
int decay = 0; // HOW MANY MS BEFORE ONE LIGHT DECAY
int decay_check = 0;
long pre_react = 0; // NEW SPIKE CONVERSION
long react = 0; // NUMBER OF LEDs BEING LIT
long post_react = 0; // OLD SPIKE CONVERSION

// RAINBOW WAVE SETTINGS
int wheel_speed = 3;

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

OneButton btn = OneButton(BUTTON_PIN, true, true);

void setup() {
  delay( 10 ); // power-up safety delay
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(50);
  FastLED.setCorrection(TypicalPixelString);

  // clear leds
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB(0, 0, 0);
  FastLED.show();

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

  // setup button
  btn.attachClick(nextPattern);

  Serial.begin(9600);
}

void loop() {
  ADCSRA |= (1 << ADSC);
  /* wait until conversion is complete */
  while (!(ADCSRA & (1 << ADIF)));
  uint16_t audio_input = ADC * 2;

  // translate audio level to number of LEDs
  pre_react = ((long)NUM_LEDS * (long)audio_input) / 1023L; // TRANSLATE AUDIO LEVEL TO NUMBER OF LEDs

    // only adjust level of led if level higher than current level
  if (pre_react > react)
    react = pre_react;

  switch (patternCounter) {
      case 0:
        movingDot();
        break;
      case 1:
        movingDots();
        break;
      case 2:
        rainbowBeat();
        break;
      case 3:
        redWhiteBlue();
        break;
      case 4:
        fireLoop();
        break;
      case 5:
        basic_color_change();
        break;
      case 6:
        rainbow();
        break;
      case 7:
        random_fade_to_black();
        break;
  }

  FastLED.show(); 
  btn.tick();
}

void nextPattern() {
  patternCounter = (patternCounter + 1) % NUM_PATTERNS;  // Change the number after the % to the number of patterns you have
}

// FUNCTION TO GENERATE COLOR BASED ON VIRTUAL WHEEL
// https://github.com/NeverPlayLegit/Rainbow-Fader-FastLED/blob/master/rainbow.ino
CRGB Scroll(int pos) {
  CRGB color (0,0,0);
  if(pos < 85) {
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = 255 - color.r;
  } else if(pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = 255 - color.g;
    color.b = 0;
  } else if(pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = 255 - color.b;
    color.r = 1;
  }
  return color;
}

// FUNCTION TO GET AND SET COLOR
// THE ORIGINAL FUNCTION WENT BACKWARDS
// THE MODIFIED FUNCTION SENDS WAVES OUT FROM FIRST LED
// https://github.com/NeverPlayLegit/Rainbow-Fader-FastLED/blob/master/rainbow.ino
void rainbow()
{
  for(int i = NUM_LEDS - 1; i >= 0; i--) {
    if (i < react)
      leds[i] = Scroll((i * 256 / 50 + k) % 256);
    else
      leds[i] = CRGB(0, 0, 0);      
  }
  k = k - wheel_speed; // SPEED OF COLOR WHEEL
  if (k < 0) // RESET COLOR WHEEL
    k = 255;

  // REMOVE LEDs
  decay_check++;
  if (decay_check > decay)
  {
    decay_check = 0;
    if (react > 0)
      react--;
  }
}

void basic_color_change()
{
  for(int i = NUM_LEDS - 1; i >= 0; i--) {
    if (i < react)
      leds[i] = CHSV(hue, 255, 255);
    else
      leds[i] = CRGB(0, 0, 0);      
  }

  k = k - wheel_speed; // SPEED OF COLOR WHEEL
  if (k < 0) // RESET COLOR WHEEL
    k = 255;

  // REMOVE LEDs
  decay_check++;
  if (decay_check > decay)
  {
    decay_check = 0;
    if (react > 0)
      react--;
  }
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

void random_fade_to_black() {
  EVERY_N_MILLISECONDS(50){
    //Switch on an LED at random, choosing a random color from the palette
    for (int i = 0; i < react / 3; i++) {
      leds[random8(0, NUM_LEDS - 1)] = ColorFromPalette(myPal, random8(), 255, LINEARBLEND);
    }
  }

  decay_check++;
  if (decay_check > decay)
  {
    decay_check = 0;
    if (react > 0)
      react--;
  }

   // Fade all LEDs down by 1 in brightness each time this is called
   fadeToBlackBy(leds, NUM_LEDS, 1);
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

void movingDot() {
  uint16_t sinBeat = beatsin16(30, 0, NUM_LEDS - 1, 0, 0);

  leds[sinBeat] = CRGB::Blue;
  
  fadeToBlackBy(leds, NUM_LEDS, 10);
}
