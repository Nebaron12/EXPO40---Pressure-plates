#include "FF.h"

// Constructor for FF class
// Initializes the shift register with the specified number of bits, data pin, clock pin, and initial value
FF::FF(uint8_t numBits, uint8_t pinData, uint8_t pinClock, uint8_t initValue)
{
    mNumBits = numBits;
    mPinData = pinData;
    mPinClock = pinClock;

    pinMode(mPinData, OUTPUT);
    pinMode(mPinClock, OUTPUT);

    setAll(initValue);
}

// Advances the shift register by one bit, setting the last bit to the specified value
void FF::advance(bool value)
{
    digitalWrite(mPinClock, LOW);
    digitalWrite(mPinData, value);
    digitalWrite(mPinClock, HIGH);
}

// Sets all bits in the shift register to the specified value
void FF::setAll(bool value)
{
    for (int i = 0; i < mNumBits; i++)
    {
        advance(value);
    }
}