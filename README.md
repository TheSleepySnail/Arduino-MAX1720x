# MAX1720x ModelGauge m5 Fuel Gauges
Arduino library for interfacing with the MAX172x battery fuel gauge.

## Usage

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <max1720x.h>

// Battery gauge with 10mOhm shunt
Max1720x batteryGauge(&Wire, 10);

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Max1720xConfiguration batteryConfiguration;
    batteryConfiguration.capacity = 700; ///< Battery capacity 700mAh
    batteryGauge.configure(batteryConfiguration);
}

void loop() {
    Serial.print("Current:       ");
    Serial.println(batteryGauge.readCurrent());
    Serial.print("Voltage:       ");
    Serial.println(batteryGauge.readVCell());
    Serial.print("StateOfCharge: ");
    Serial.println(batteryGauge.readRepSOC());
    delay(5000);
}
```

## Configuration

The struct `Max1720xConfiguration` contains the parameters stored in registers and is initialized with the default values. For a basic usage only the capacity and the size of the shunt needs to be provided. For more information check the [configuration](docs/configuration.md) page.


## Links
- [Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX17201-MAX17215.pdf)