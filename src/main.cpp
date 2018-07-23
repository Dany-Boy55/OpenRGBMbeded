#include <Arduino.h>
#include <FastLED.h>

#define LEDNUM 255

CRGB ledBuffer[LEDNUM];
CRGB Color1;
CRGB Color2;
byte rate;

void setup(void) {
    Serial.begin(19200);
    FastLED.addLeds<WS2812, 2>(ledBuffer, LEDNUM);

}

void loop(void) {
    
}

void showWithScaling(){
    
}

CRGB colormix(CRGB &target, CRGB mixer){

}

void fillSolid(){
    
}