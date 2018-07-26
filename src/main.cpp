#include <Arduino.h>
#include <FastLED.h>

#define LEDNUM 255

CRGB ledBuffer[LEDNUM];
CRGB Colors[10];
byte rate;
byte effect;

/*
* Serial Protocol:
* Byte   1       2        3      4       5     6    .....
*      length   crc    Command  arguments   ....
*/

/*
* Effect List:
* 0x00 --> OFF
* 0x01 --> Solid color[0]
* 0x02 --> Solid color[1]
* .... --> ..... ........
* .... --> ..... ........
* 0x0A --> Solid color[9]
*/

void receiveData(){
    byte dataLen = Serial.read();
    byte data[dataLen];
    Serial.readBytes(data, dataLen);
    // Second byte is a CRC checksum, not implemented yet

    // Third byte in sequence is the command being parsed
    switch (data[1])
    {
        case 0x00:  // 0 Identify this device (id, type, name)
            Serial.write(0x00);
            break;
        case 0x01: // Receive an effect
            effect = data[2];
            break;
        case 0x03: // Receive a color --> (color[] red green blue)
            Colors[data[2]].red = data[3];
            Colors[data[2]].green = data[4];
            Colors[data[2]].blue = data[5];
            break;
        default:
            Serial.flush();
            break;
    }
}

// Mixes source color (by reference) into target color by a specified amount
void colormix(CRGB &source, CRGB target){
    if(source.red < target.red)
        source.red++;
    else if(source.red > target.red)
        source.red--;

    if(source.green < target.red)
        source.green++;
    else if(source.green > target.green)
        source.green--;

    if(source.blue < target.blue)
        source.blue++;
    else if(source.blue > target.blue)
        source.blue--;
}

void fillSolid(CRGB color){
    for(int i = 0; i < LEDNUM; i++){
        ledBuffer[i] = color;
    }
    FastLED.show();
}

void setup(void) {
    Serial.begin(19200);
    FastLED.addLeds<WS2812, 2, GRB>(ledBuffer, LEDNUM);
    fillSolid(CRGB::Red);
    delay(200);
    fillSolid(CRGB::Green);
    delay(200);
    fillSolid(CRGB::Blue);
    delay(200);
    for(int i = 0; i < 5; i++)
    {
        delay(50);
        if(i%2 == 0)
            fillSolid(CRGB::Black);
        else
            fillSolid(CRGB::White);
    }
}

void loop(void) {
    if(Serial.available() > 0){
        receiveData();
    }    

    switch (effect)
    {
        case 0x00:
            fillSolid(CRGB::Black);
            break;
        case 0x01:
            fillSolid(Colors[0]);
            break;
        case 0x02:
            fillSolid(Colors[1]);
            break;
        default:
            break;
    }
}