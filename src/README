# MCP3204 Pressure Plate ADC System

## Overview
This project provides a complete library and example code for reading multiple pressure plates using MCP3204 12-bit ADC chips. The system uses shift registers to control multiple ADC chip select (CS) lines, allowing you to connect multiple pressure plates to a single Arduino.

Each pressure plate has 4 load sensors arranged in a square pattern. The system calculates the center of pressure by computing X and Y direction differences from the sensor readings.

## Features
- Support for multiple pressure plates (configurable)
- 12-bit ADC resolution using MCP3204 chips
- Automatic baseline calibration on startup
- Optional signal averaging for noise reduction
- Real-time X/Y position calculation
- SPI-based communication for fast readings
- Shift register control for multiple ADC selection

## Hardware Requirements

### Components
- Arduino Uno, Nano, Mega, ESP, Pico or similar
- Custom PCB with MCP3204 ADC chips
- Pressure plates with 4 load sensors each
- Shift register (Either using Flip-Flops or chips such as the 74HC595) for CS control

### Pin Connections

#### Arduino Uno/Nano Connections:
```
Arduino Pin  | PCB Connection
-------------|---------------
Pin 2        | FF_DATA_PIN (Shift Register Data)
Pin 3        | FF_CLK_PIN (Shift Register Clock)
Pin 11 (MOSI)| SPI Data In (All MCP3204s)
Pin 12 (MISO)| SPI Data Out (All MCP3204s)
Pin 13 (SCK) | SPI Clock (All MCP3204s)
5V           | VCC (PCB Power)
GND          | GND (PCB Ground)
```

#### Arduino Mega Connections:
```
Arduino Pin  | PCB Connection
-------------|---------------
Pin 2        | FF_DATA_PIN (Shift Register Data)
Pin 3        | FF_CLK_PIN (Shift Register Clock)
Pin 51 (MOSI)| SPI Data In (All MCP3204s)
Pin 50 (MISO)| SPI Data Out (All MCP3204s)
Pin 52 (SCK) | SPI Clock (All MCP3204s)
5V           | VCC (PCB Power)
GND          | GND (PCB Ground)
```

## Software Setup

### 1. User Configuration (PP_ADC.h)
Edit the following defines in `PP_ADC.h` to match your setup:

```cpp
#define NUM_PLATES 1      // Number of pressure plates (1-8)
#define NUM_SENSORS 4     // Sensors per plate (always 4)
#define ADC_VREF 5.0      // Reference voltage (5.0V for Arduino Uno/Mega and the PCB's built in regulator)
#define BASELINE_READS 16 // Baseline calibration samples (1-16)
#define AVERAGING_FACTOR 4 // Signal averaging (1-16, comment out to disable)
#define ADC_DEBUG         // Uncomment for debug output
```

### 2. Pin Configuration (main.cpp)
Update the pin definitions in `main.cpp` if needed:

```cpp
#define FF_DATA_PIN 2     // Shift register data pin
#define FF_CLK_PIN 3      // Shift register clock pin
```

### 3. PlatformIO Setup
The project is configured for PlatformIO. If using Arduino IDE:
1. Copy all `.cpp` and `.h` files to your sketch folder
2. Ensure SPI library is included

## Usage

### Basic Example
```cpp
#include "PP_ADC.h"

// Initialize pressure plate system
PP_ARRAY pressurePlates(FF_DATA_PIN, FF_CLK_PIN);

void setup() {
    Serial.begin(9600);
    SPI.begin();
    // Automatic baseline calibration occurs in constructor
}

void loop() {
    // Read all pressure plates
    pressurePlates.readAll();
    
    // Access X/Y differences for plate 0
    int16_t x_diff = pressurePlates.mDiffValuesX[0];
    int16_t y_diff = pressurePlates.mDiffValuesY[0];
    
    Serial.print("X: "); Serial.print(x_diff);
    Serial.print(" Y: "); Serial.println(y_diff);
    
    delay(100);
}
```

### Advanced Usage
```cpp
// Read raw sensor values (before baseline correction)
uint16_t rawValues[NUM_PLATES][NUM_SENSORS];
pressurePlates.readAllRaw(rawValues);

// Read single channel (ADC 1, Channel 0)
uint16_t singleValue = pressurePlates.readChannel(1, 0);
```

## Understanding the Output

### X/Y Difference Values
- **Positive X**: Weight toward sensors 0-1 side
- **Negative X**: Weight toward sensors 2-3 side  
- **Positive Y**: Weight toward sensors 1-2 side
- **Negative Y**: Weight toward sensors 0-3 side

### Sensor Layout (Top View)
```
            Y+
            |
        [0]---[1]
 X- ---  |     |  --- X+
        [3]---[2]
            |
            Y-
```

## Calibration Process

### Automatic Baseline Calibration
1. Keep pressure plates unloaded during startup
2. System takes `BASELINE_READS` samples from each sensor
3. Averages these values as baseline reference
4. All subsequent readings are relative to this baseline

### Manual Recalibration
To recalibrate, simply reset the Arduino with unloaded plates.

## Troubleshooting

### Common Issues
1. **All readings are zero**: Check SPI connections and power
2. **Erratic readings**: Increase `AVERAGING_FACTOR` or check for electrical noise
3. **One direction not working**: Check sensor wiring and connections
4. **Slow readings**: Reduce `AVERAGING_FACTOR` if speed is critical

### Debug Mode
Enable `ADC_DEBUG` to see:
- Baseline calibration values
- Initialization status
- Detailed sensor readings

### Performance Notes
- Reading time: ~250-4000 microseconds per plate (depending on averaging)
- Maximum update rate: ~2000 Hz (with minimal averaging)
- ADC resolution: 12-bit (0-4095 counts)

## Library Reference

### PP_ARRAY Class
- `PP_ARRAY(pinData, pinClock)` - Constructor
- `readAll()` - Read all plates, calculate X/Y differences
- `readAllRaw(array)` - Read raw sensor values
- `readChannel(adc, channel)` - Read single sensor
- `mDiffValuesX[]` - X direction differences
- `mDiffValuesY[]` - Y direction differences

### MCP320x Class
Low-level ADC interface (used internally by PP_ARRAY)

### FF Class  
Shift register control for CS line management (used internally)

## License
This project is based on work licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

## Author
Dirk Jan Bakels - d.bakels2709@gmail.com / 524334@student.fontys.nl
If there's bugs, contacting me is no problem