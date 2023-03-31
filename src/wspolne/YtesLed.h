#ifndef _YTESLED_h
#define _YTESLED_h
#include "arduino.h"

#define YTESLED_ON HIGH
#define YTESLED_OFF LOW
#define YTES_DELAY_STEP 20

//Talking led
class YtesLed {
private:   
    uint8_t LEDPin;
    uint8_t LEDRepeater;
    bool repeaterInverted;
    uint8_t LEDStatus;
    uint16_t* sequenceNext;
    uint16_t* sequenceCurrent;
    uint16_t* sequence;
    uint32_t now;
    uint32_t changeTime;
    uint32_t delayEnd;
    bool sequenceEnd;
    //bool _build_message_sequence();
    bool _write_repeater();
public:
    YtesLed(void);
    bool begin(uint8_t aLEDPin);
    bool update(void);
    void delay(uint32_t aDelay);
    void setLED(uint8_t aLEDStatus);
    bool isEnd(void);
    void waitEnd(void);

    bool setSequence(uint16_t* aSequence);
    void on(void);
    void off(void);
};
//---------------KONSTRUKTOR , DESTRUKTOR , PRYWATNE ------------
YtesLed::YtesLed(void) {
    //LEDPin = LED_PIN;
    LEDRepeater = 0;
    repeaterInverted = false;
    LEDStatus = YTESLED_OFF;
    sequenceNext = NULL;
    sequenceCurrent = NULL;
    sequence = NULL;
    sequenceEnd = true;
    changeTime = millis();
}


bool YtesLed::begin(uint8_t aLEDPin) {
    LEDPin = aLEDPin;
    digitalWrite(LEDPin, LEDStatus);
    pinMode(LEDPin, OUTPUT);
    return true;
};

bool YtesLed::_write_repeater() {
    if (!LEDRepeater)
        return false;
    if (repeaterInverted)
        digitalWrite(LEDRepeater, !LEDStatus);
    else
        digitalWrite(LEDRepeater, LEDStatus);
    return true;
}

//------------------PUBLICZNE ----------------------------------
bool YtesLed::update(void) {
    now = millis();
    if (sequence) {
        if (now < changeTime)
            return false;
        else {
            if (*sequence) {
                LEDStatus ^= 0x1;
                digitalWrite(LEDPin, LEDStatus);
                _write_repeater();
                changeTime = now + *sequence++;
                sequenceEnd = false;
                return true;
            }
            else
                sequenceEnd = true;
        }
    }
    if (sequenceNext) {
        sequenceCurrent = sequenceNext;
        sequenceNext = NULL;
    }
    sequence = sequenceCurrent;
    changeTime = now;
    return false;
}

void YtesLed::delay(uint32_t aDelay) {
    delayEnd = aDelay + millis();
    while (delayEnd - millis() > YTES_DELAY_STEP) {
        update();
        ::delay(YTES_DELAY_STEP);
    }
    update();
    ::delay(delayEnd - millis());
}

void YtesLed::setLED(uint8_t aLEDStatus) {
    LEDStatus = aLEDStatus;
    digitalWrite(LEDPin, LEDStatus);
    _write_repeater();
    sequenceNext = NULL;
    sequenceCurrent = NULL;
    sequence = NULL;
    sequenceEnd = true;
    changeTime = millis();
}

bool YtesLed::isEnd() {
    return sequenceEnd;
}


void YtesLed::waitEnd() {
    while (!sequenceEnd) {
        update();
        ::delay(YTES_DELAY_STEP);
    }
    sequenceEnd = false;
}

bool YtesLed::setSequence(uint16_t* aSequence) {
    sequenceNext = aSequence;
    return true;
}
void YtesLed::on() {
    digitalWrite(LEDPin, YTESLED_ON);
}

void YtesLed::off() {
    digitalWrite(LEDPin, YTESLED_OFF);
}

#endif