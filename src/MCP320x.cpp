#if defined(ARDUINO) && (ARDUINO >= 10605)
#include "Arduino.h"
#else
#error This library requires Arduino IDE 1.6.5 or above
#endif

#include "MCP320x.h"
#include <SPI.h>

#define SPI_ADC_RESOLUTION		4096.0						// 12-bit MCP320x ADC resolution
#define MCP_CLOCK				1000000UL					// 1MHz clock rate - for compatibility, use lower rate (for 3.3V systems)

/*
 faster mode using SPI library
*/
MCP320x::MCP320x ( uint8_t adc_count, const uint8_t channel_count, const uint8_t ff_data_pin, const uint8_t ff_clk_pin) 
	: _flipFlopCS(adc_count, ff_data_pin, ff_clk_pin, 0) {
	_MCP_channelCount = channel_count;	// Number of channels in the MCP (4 or 8)

	for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) { 
		// set default sampling mode
		_DeviceMode[i] = MCP_SINGLE;
	}

	_numADCs = adc_count;
	_curADC = 0;	// Note: ADC being 0 indicates no ADC is selected
	_flipFlopCS.setAll(true);
}

/*
 returns the sampling config (single/differential) of the given channel or MCP_RANGE_ERROR if channel is out of range
*/
MCPMode MCP320x::getMCPConfig ( const uint8_t channel ) {
	if ( channel < MCP_MAX_PORTS ) {
		return _DeviceMode[channel];
	} else {
		return MCP_RANGE_ERROR;
	}
}

/*
 Set the sampling config (single/differential) of the given channel or ALL channels if channel == MCP_ALL_PORTS
 For DIFFERENTIAL mode, the channels operate in pairs. Setting either one will also set the other.
 Returns true if successful, else false
 
*/
bool MCP320x::setMCPConfig ( const MCPMode mode, const uint8_t channel ) {
	bool retval;
	
	if ( channel == MCP_ALL_PORTS ) {
		for ( uint8_t i = 0; i < MCP_MAX_PORTS; i++ ) {
			_DeviceMode[i] = mode;
		}
		retval = true;
	} else if ( channel < MCP_MAX_PORTS ) {
		if ( mode == MCP_SINGLE ) {
			_DeviceMode[channel] = mode;
		} else {
			// always pair up differential channels by inverting the LSbit to get the other channel number (2 -> 3, 3 -> 2, etc)
			uint8_t	altChannel = (channel & B110) | (channel ^ B001);
			
			_DeviceMode[channel] = mode;
			_DeviceMode[altChannel] = mode;
		}
		retval = true;
	} else {
		retval = false;
	}
	return retval;
}

/*
 read all MCP channels (based on given channel count)
 The number of channels is defined in the constructor.
*/
bool MCP320x::readAllChannels ( uint8_t adc, uint16_t channelValue[] ) {

	for ( uint8_t i = 0; i < _MCP_channelCount; i++ ) {
		channelValue[i] = _read_SPI(adc, i);
	}
	return true;
} 

/*
 read an individual channel, using the currently set mode
*/ 
uint16_t MCP320x::readChannel ( uint8_t adc, const uint8_t channel ) {
	if ( channel >= _MCP_channelCount ) {
		return MCP_CHANNEL_ERROR;
	}

	return _read_SPI(adc, channel);
}

/*
 Calculate the voltage given the raw (digitized) value of the channel
*/
float MCP320x::rawToVoltage ( const float VREF, const uint16_t ADCRawValue) {

    return (float)(ADCRawValue * (VREF/SPI_ADC_RESOLUTION));
}

/*
 For timing info and bit pattern, see Figure 6-1 in the Microchip MCP3204/8 Data Sheet
 Note that we are using the conservative clock value - 1MHz for 2.7V, 2MHz for 5V, so assume the lower value to be safe
*/
uint16_t MCP320x::_read_SPI ( uint8_t adc, const uint8_t channel ) {
	uint8_t		msb, lsb;
	uint16_t 	commandWord;
	uint16_t	digitalValue;
	
	/*
	 Build the following command pattern that will be sent as 3 bytes in separate transactions:
	 <5 leading 0s><start bit><sgl/diff><D2> <D1><D0><6 0s> <8 zeros>
	 byte    1                   2                    3
	 
	 And we expect to get:
	 <8 don't care bits> <<3 don't care bits>0<B11-B8>> <B7-B0>
	         1                     2                       3
	*/ 
	if ( _DeviceMode[channel] == MCP_SINGLE ) {
		commandWord = (_BV(10) | _BV(9));									// start bit(10) + single-ended(9)
	} else {
		commandWord = _BV(10);												// start bit(10) + double(9 is 0)
	}
	commandWord |= (uint16_t)(channel << 6);								// 3-bit channel number D2-D0
	
	// now send this command word as 3 bytes: MSB, LSB, then 0 (3rd is really a don't care)
	SPI.beginTransaction(SPISettings(MCP_CLOCK, MSBFIRST, SPI_MODE0));		// use highest clock allowed for 3.3V operation
	_setCS(adc);		 													// Deselect the previous ADC and select the requested ADC
	SPI.transfer(highByte(commandWord));									// send MSB, get byte 1: we ignore the returned value - it's garbage
	// keep msb & lsb in separate variables for debugging the SPI interface
	msb = SPI.transfer(lowByte(commandWord));								// send LSB, get byte 2: ???0<B11-B8>
	digitalValue = (uint16_t)((msb & 0x0F) << 8);							// mask off unused top 4 bits and move to MSB
	lsb = SPI.transfer(0);													// send don't care byte: get <B7-B0>
	digitalValue |= (uint16_t)lsb;											// add the LSB to the returned value
	SPI.endTransaction();
	
	return digitalValue;
}

/*
 Helper function to set the CS line for the given channel
 The MCP320x ADC CS pin is active LOW
 The MCP320x requires CS to go high before every conversion, so we always need to 
 shift out the current select bit and shift in a new one at the target position.
*/
void MCP320x::_setCS ( const uint8_t selected_adc ) {
    // Validate input
    if (selected_adc < 1 || selected_adc > _numADCs) {
        // Invalid ADC selection - reset to known state
        _flipFlopCS.setAll(true); // Deselect all ADCs
        _curADC = 0;
        return;
    }

	// First reset all FF's to 1 to deselect all ADCs if the current ADC is ahead of or equal to the selected one
	if (_curADC >= selected_adc) {
		_flipFlopCS.setAll(true); // Deselect all ADCs
		_flipFlopCS.advance(false); // Shift in a 0 to select the first ADC
		_curADC = 1; // We just selected the first ADC
	}

	// Now just shift it to the target position while shifting in 1s (no-select bits)
	for (uint8_t i = _curADC; i < selected_adc; i++) {
		_flipFlopCS.advance(true);
	}

	// The state is valid, so assume the function completes successfully
	_curADC = selected_adc;
	return;
}