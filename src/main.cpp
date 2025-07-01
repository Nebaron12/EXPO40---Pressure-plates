#include <SPI.h>
#include "MCP320x.h"
#include "PP_ADC.h"

// Pressure plate configuration
#define FF_DATA_PIN 2				// Data pin for the flip flop (FF) shift register
#define FF_CLK_PIN 3				// Clock pin for the FF shift register

PP_ARRAY pressurePlates(FF_DATA_PIN, FF_CLK_PIN);

void setup()
{
	Serial.begin(9600);
  	SPI.begin();
	Serial.println("Pressure Plate Test - Starting...");
}

void loop()
{
	uint32_t delta, time_in, time_out;

	// Read all pressure plate values and compute differences
	time_in = micros();
	pressurePlates.readAll();
	time_out = micros();
  
	// Print X and Y difference values for plate 0
	Serial.print("Plate 0 - X difference: ");
	Serial.print(pressurePlates.mDiffValuesX[0]);
	Serial.print("\t Y difference: ");
	Serial.println(pressurePlates.mDiffValuesY[0]);
	
	Serial.print("Read operation duration: "); 
	delta = time_out - time_in;	
	Serial.print(delta);    
	Serial.println(" usec");

	delay(500);
	Serial.println();
}


// A debugging loop for reading raw values from the pressure plates
// void loop()
// {
// 	uint16_t rawValues[NUM_PLATES][NUM_SENSORS]; // Array to hold raw values for all sensors from all plates

// 	// Read all raw pressure plate sensor values
// 	pressurePlates.readAllRaw(rawValues);

// 	// Print raw values for each sensor
// 	for (uint8_t i = 0; i < NUM_PLATES; i++) {
// 		Serial.print("Plate ");
// 		Serial.print(i);
// 		Serial.println(": ");
// 		for (uint8_t j = 0; j < NUM_SENSORS; j++) {
// 			Serial.print("  Sensor ");
// 			Serial.print(j);
// 			Serial.print(": ");
// 			Serial.println(rawValues[i][j]);
// 		}
// 	}

// 	delay(2000); // Delay before the next read
// }


