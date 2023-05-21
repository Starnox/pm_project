#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 60
int s;
CRGB leds[NUM_LEDS];

int r=152;
int g=0;
int b=10;

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();

  Serial.begin(9600);
  pinMode(A0,INPUT); // TODO set this with register -> lab1
}

//TODO add timer and interrupt that will make a wave let's say every 1s

void loop() {
  // put your main code here, to run repeatedly:
  // leds[0] = CRGB(255, 0, 0); // RED
  // leds[1] = CRGB(0, 255, 0); // GREEN
  // leds[2] = CRGB(0, 0, 255); // BLUE
  s=analogRead(A0);
  s=s*2;
  Serial.println(s);
 // delay(50);
  if((s>=450)&&(s<=550))
  {
    leds[(NUM_LEDS/2)-1]=CRGB (0, 0, 255);
    leds[NUM_LEDS/2]=CRGB (0, 0, 255);
  }
  else if((s>=400)&&(s<=450))
  {
    leds[(NUM_LEDS/2)-1]=CRGB (153, 153, 0);
    leds[NUM_LEDS/2]=CRGB (153, 153, 0);
  }
  else if((s>=350)&&(s<=400))
   {
     leds[(NUM_LEDS/2)-1]=CRGB (255, 50, 255);
    leds[NUM_LEDS/2]=CRGB (255, 50, 255);
   }
   else if((s>=300)&&(s<=350))
  {
    leds[(NUM_LEDS/2)-1]=CRGB (10, 25, 217);
    leds[NUM_LEDS/2]=CRGB (10, 25, 217);
  }

    else if((s>=276)&&(s<=300))
   {
     leds[(NUM_LEDS/2)-1]=CRGB (50, 50, 150);
    leds[NUM_LEDS/2]=CRGB (50, 50, 150);
   }
   else if((s>=250)&&(s<=275))
   {
     leds[(NUM_LEDS/2)-1]=CRGB (230, 0, 10);
    leds[NUM_LEDS/2]=CRGB (230, 0, 10);
   }
  else if((s>=235)&&(s<=250))
   {
     leds[(NUM_LEDS/2)-1]=CRGB (0, 160, 0);
    leds[NUM_LEDS/2]=CRGB (0, 160, 0);
   }
   else if((s>=200)&&(s<=230))
   {
     leds[(NUM_LEDS/2)-1]=CRGB (1, 0, 1);
    leds[NUM_LEDS/2]=CRGB (1, 0, 1);
   }
  else
  {
     leds[(NUM_LEDS/2)-1] = CRGB ( r,s-100,b);
     leds[NUM_LEDS/2] = CRGB ( r,s-100,b);
  }
    for (int i = 0; i <= ((NUM_LEDS/2)-2); i++) 
  {
     leds[i] = leds[i+1];
     leds[NUM_LEDS-1-i] = leds[(NUM_LEDS)-i-2];
     
  }
 FastLED.show();
 delay(25);
}
