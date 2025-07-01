/**
 * @file PP_ADC.h
 * @author Dirk Jan Bakels
 *
 * High level interface for getting pressure plate difference readings
 * using MCP3204 ADC chip.
 */
#pragma once

// Includes
#include <Arduino.h>
#include "MCP320x.h"

// User defines
#define NUM_PLATES 1  // Number of pressure plates
#define NUM_SENSORS 4 // Number of sensors per pressure plate
#define ADC_VREF 5.0  // Voltage reference for ADC in V
#define BASELINE_READS 16 // Number of baseline readings to take for each sensor at startup (Max 16 or overflow might occur)
#define AVERAGING_FACTOR 4 // Number of samples to average when performing readings, comment out to disable averaging (Max 16 or overflow might occur)
#define ADC_DEBUG // Uncomment to print debug information

// Classes
class PP_ARRAY
{
    public:
        // Functions
        PP_ARRAY(uint8_t pinData, uint8_t pinClock); // Constructor
        void readAll(); // Function to read all pressure plate values and compute them into the difference array
        void readAllRaw(uint16_t (*valArray)[NUM_SENSORS]); // Function to read all raw pressure plate sensor values and place them in an external array
        uint16_t readChannel(uint8_t adc, uint8_t ch); // Internal function to read a single channel value accounting for the baseline value

        // Variables
        int16_t mDiffValuesX[NUM_PLATES]; // Array to store X direction difference values
        int16_t mDiffValuesY[NUM_PLATES]; // Array to store Y direction difference values
    
    private:
        // Instances
        MCP320x mcp;                        // ADC instance

        // Variables
        uint16_t mBaselineValues[NUM_PLATES][4]; // 2D array to store all sensor baseline values
        
};
