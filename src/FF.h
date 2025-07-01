/**
 * @file FF.h
 * @author Dirk Jan Bakels <d.bakels2709@gmail.com>
 *
 * Low level interface for using Shift register/FFs for CS# with SPI devices
 */
#pragma once

#include <Arduino.h>

class FF
{
    public:
    FF(uint8_t numBits, uint8_t pinData, uint8_t pinClock, uint8_t initValue);
    void advance(bool value);
    void setAll(bool value);

    private:
    uint8_t mNumBits;
    uint8_t mPinData;
    uint8_t mPinClock;
};