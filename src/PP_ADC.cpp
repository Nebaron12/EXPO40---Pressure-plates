#include "PP_ADC.h"
#include <SPI.h>

// Constructor for PP_ARRAY class
PP_ARRAY::PP_ARRAY(uint8_t pinData, uint8_t pinClock) : 
mcp(NUM_PLATES, NUM_SENSORS, pinData, pinClock) // Initialize MCP320x with NUM_PLATES ADCs, NUM_SENSORS channels each
{
    SPI.begin();
    
    // Initialize baseline values to zero
    for (uint8_t i = 0; i < NUM_PLATES; i++) {
        for (uint8_t j = 0; j < NUM_SENSORS; j++) {
            mBaselineValues[i][j] = 0;
        }
    }
    
    // Initialize all sensor baseline values by reading from each ADC with no load
    uint16_t channelValues[NUM_SENSORS];
    for (uint8_t i = 0; i < NUM_PLATES; i++) {
        // Read baseline values for all channels of the current ADC
        for (uint8_t j = 0; j < BASELINE_READS; j++) {
            mcp.readAllChannels(i + 1, channelValues);
            for (uint8_t k = 0; k < NUM_SENSORS; k++) {
                mBaselineValues[i][k] += channelValues[k];
            }
        }

        // Average the baseline values over the number of reads
        for (uint8_t j = 0; j < NUM_SENSORS; j++) {
            mBaselineValues[i][j] /= BASELINE_READS;
            #ifdef ADC_DEBUG
            Serial.println(String("Baseline value plate ") + i + ", sensor " + j + ": " + mBaselineValues[i][j]);
            #endif
        }
    }

    #ifdef ADC_DEBUG
    Serial.println("Pressure plate array initialized");
    #endif
}

// Function to read all pressure plate values and compute them into the difference array
void PP_ARRAY::readAll() {
    uint16_t channelValues[NUM_SENSORS];
    
    // Loop through all pressure plates and read their values into the mDiffValues array
    for (uint8_t i = 0; i < NUM_PLATES; i++) {
        #ifdef AVERAGING_FACTOR // If averaging is enabled, read multiple times and average the results
        uint16_t sumValues[NUM_SENSORS] = {0}; // Use uint32_t to prevent overflow during accumulation
        
        for (uint8_t j = 0; j < AVERAGING_FACTOR; j++) {
            mcp.readAllChannels(i + 1, channelValues);
            for (uint8_t k = 0; k < NUM_SENSORS; k++) {
                sumValues[k] += channelValues[k]; // Accumulate values
            }
        }
        // Calculate the average for each channel
        for (uint8_t k = 0; k < NUM_SENSORS; k++) {
            channelValues[k] = sumValues[k] / AVERAGING_FACTOR; // Average the accumulated values
        }
        #else // If averaging is not enabled, read just once
        mcp.readAllChannels(i + 1, channelValues);
        #endif

        // Apply baseline correction to each channel
        int16_t correctedValues[NUM_SENSORS];
        for (uint8_t j = 0; j < NUM_SENSORS; j++) {
            correctedValues[j] = (int16_t)channelValues[j] - (int16_t)mBaselineValues[i][j];
        }
        
        // Calculate averages of channels 01 12 23 30
        int16_t avg[4]; // Use signed to handle negative results
        avg[0] = (correctedValues[0] + correctedValues[1]) / 2;
        avg[1] = (correctedValues[1] + correctedValues[3]) / 2;
        avg[2] = (correctedValues[2] + correctedValues[3]) / 2;
        avg[3] = (correctedValues[0] + correctedValues[2]) / 2;

        // Calculate the difference values for X and Y directions
        mDiffValuesX[i] = avg[0] - avg[2]; // X direction difference
        mDiffValuesY[i] = avg[1] - avg[3]; // Y direction difference
    }
}

// Function to read all raw pressure plate sensor values and place them in an external array
void PP_ARRAY::readAllRaw(uint16_t (*valArray)[NUM_SENSORS]) {
    for (uint8_t i = 0; i < NUM_PLATES; i++) {
        mcp.readAllChannels(i + 1, valArray[i]); // Read directly into the provided array
    }
}

// Function to read a single channel value accounting for the baseline value
uint16_t PP_ARRAY::readChannel(uint8_t adc, uint8_t ch) {
    uint16_t rawValue = mcp.readChannel(adc, ch);
    int16_t baseline = mBaselineValues[adc - 1][ch]; // Convert ADC number to array index
    
    // Return the difference from baseline as unsigned, clamping negative values to 0
    return (int16_t)rawValue - baseline;
}