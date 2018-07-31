#include <Arduino.h>
#include <FastLED.h>
#include <EEPROM.h>

#define LEDNUM 255

CRGB ledBuffer[LEDNUM];
CRGB colors[8];
byte effectArgs[8];
byte effectOptions[4];
char name[16];
unsigned long nextTick;

// Based on Dallas/Maxim 8 bit variant CRC
uint8_t calculateCRC(uint8_t *data, uint8_t len){
     uint8_t crc = 0x00;
     for( uint8_t i = 0; i < len; i++)
     {
          uint8_t inbyte = data[i];
          for ( uint8_t j = 0; j < 8; ++j )
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if ( mix ) crc ^= 0x8C;
            inbyte >>= 1;
        }
     }
     return crc;     
}

void saveDataInEEPROM(){
    EEPROM.begin();
    for(size_t i = 0; i < 8; i++)
    {
        EEPROM.update(i * 3, colors[i].red);
        EEPROM.update(i * 3 + 1, colors[i].green);
        EEPROM.update(i *3 + 2, colors[i].blue);
    }
    uint8_t offset = sizeof(colors) * 3;
    for(size_t i = 0; i < sizeof(effectArgs); i++)
    {
        EEPROM.update(i + offset, effectArgs[i]);
    }
    offset += sizeof(effectArgs);
    for(size_t i = 0; i < sizeof(effectOptions); i++)
    {
        EEPROM.update(i + offset, effectOptions[i]);
    }
    offset += sizeof(effectOptions);
    for(size_t i = 0; i < sizeof(name); i++)
    {
        EEPROM.update(i + offset, name[i]);
    }
    EEPROM.end();
}

void getDataFromEEPROM(){
    EEPROM.begin();
    for(size_t i = 0; i < 8; i++)
    {
        colors[i].red = EEPROM.read(i * 3);
        colors[i].green = EEPROM.read(i * 3 + 1);
        colors[i].blue = EEPROM.read(i *3 + 2);
    }
    uint8_t offset = sizeof(colors) * 3;
    for(size_t i = 0; i < sizeof(effectArgs); i++)
    {
        effectArgs[i] = EEPROM.read(i + offset);
    }
    offset += sizeof(effectArgs);
    for(size_t i = 0; i < sizeof(effectOptions); i++)
    {
        effectOptions[i] = EEPROM.read(i + offset);
    }
    offset += sizeof(effectOptions);
    for(size_t i = 0; i < sizeof(name); i++)
    {
        name[i] = EEPROM.read(i + offset);
    }
    EEPROM.end();
}

/*
* Description of the serial protocol 
* byte --> 0        1       2          3   ...
* val  --> length  crc     command     args...
* Length is counted from the third byte (index 2), does not include itself nor CRC
* CRC  is calculated from the thid byte (index 2), does not include length
*/

void receiveData(){
    // First byte is the length of upcoming packet
    uint8_t dataLen = Serial.read();    
    // Second byte is a CRC checksum 
    uint8_t checksum = Serial.read();
    //thid byte onwards is the packet
    byte data[dataLen];
    Serial.readBytes(data, dataLen);
    // If checksum does not match, write 0x00 on serial port and await next packet
    if (calculateCRC(data, dataLen) != checksum) {
        //Serial.write(0x00);
        //return 0;
    }    
    // Third byte is the command being parsed
    switch (data[0])
    {
        case 0x00:  // 0 Identify this device serial write (type, name)
            Serial.write(0x03);
            Serial.write(name);
            break;
        case 0x01: // Receive an effect argument: (4th byte, 5th byte) --> (effect argument index, value)
            effectArgs[data[1]] = data[2];
            break;
        case 0x03: // Receive a color (4th byte, 5th byte, 6th byte, 7th byte) --> (colorNumber, red, green, blue)
            colors[data[1]].red = data[2];
            colors[data[1]].green = data[3];
            colors[data[1]].blue = data[4];
            break;
        case 0x04: // Receive additional effect options (4th byte, 5th byte) --> (effect option index, value)
            effectOptions[data[1]] = data[2];
            break;
        case 0x05: // Receive a new name            
            for(uint8_t i = 0; i < sizeof(name); i++)
            {
                if(i < sizeof(data) - 1){
                    name[i] = char(data[i + 1]);
                }
            }            
            break;
        case 0x06: // Save into EEPROM
            saveDataInEEPROM();
            break;
        default:
            Serial.flush();
            break;
    }
}

// Mixes source color (by reference) into target color by a specified amount
void colormix(CRGB &source, CRGB mixer){
    if(source.red < mixer.red)
        source.red++;
    else if(source.red > mixer.red)
        source.red--;

    if(source.green < mixer.red)
        source.green++;
    else if(source.green > mixer.green)
        source.green--;

    if(source.blue < mixer.blue)
        source.blue++;
    else if(source.blue > mixer.blue)
        source.blue--;
}

void fadeToColor(){    
    for(uint8_t i = 0; i < LEDNUM; i++)
    {
        colormix(ledBuffer[i], colors[0]);
    }
    
}

void setup(void) {
    Serial.begin(19200);
    FastLED.addLeds<WS2812, 2, GRB>(ledBuffer, LEDNUM);
    
    for(uint8_t i = 0; i < LEDNUM; i++)
    {
        ledBuffer[i] = CRGB::Red;
        FastLED.show();
    }
    for(uint8_t i = 0; i < LEDNUM; i++)
    {
        ledBuffer[i] = CRGB::Green;
        FastLED.show();
    }
    for(uint8_t i = 0; i < LEDNUM; i++)
    {
        ledBuffer[i] = CRGB::Blue;
        FastLED.show();
    }
    for(uint8_t i = 0; i < LEDNUM; i++)
    {
        ledBuffer[i] = CRGB::Black;
    }
    FastLED.show();
}

void loop(void) {
    if(Serial.available() > 0){
        receiveData();
    }


}