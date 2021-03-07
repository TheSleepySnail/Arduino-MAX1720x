#include <Arduino.h>
#include <Wire.h>
#include <max1720x.h>

// Battery gauge with 10mOhm shunt
Max1720x batteryGauge(&Wire, 10);

/**
 * Helper for printing
 */
void print(const char *text, const char *unit, double value) {
	Serial.print(text);
	Serial.print(value);
	Serial.println(unit);
}

/**
 * Helper for printing
 */
void printHex(const char *text, uint16_t value) {
	Serial.print(text);
	Serial.print("0x");
	Serial.println(value, HEX);
}

/**
 * Read configuration registers and print them out
 */
void readAndPrintConfiguration() {
	printHex("vEmpty     ", batteryGauge.readRegister(MAX1720X_ADDRESS_V_EMPTY));
	printHex("learnCfg   ", batteryGauge.readRegister(MAX1720X_ADDRESS_LEARN_CFG));
	printHex("qRTable00  ", batteryGauge.readRegister(MAX1720X_ADDRESS_QR_TABLE_00));
	printHex("qRTable10  ", batteryGauge.readRegister(MAX1720X_ADDRESS_QR_TABLE_10));
	printHex("qRTable20  ", batteryGauge.readRegister(MAX1720X_ADDRESS_QR_TABLE_20));
	printHex("qRTable30  ", batteryGauge.readRegister(MAX1720X_ADDRESS_QR_TABLE_30));
	printHex("fullCapNom ", batteryGauge.readRegister(MAX1720X_ADDRESS_FULL_CAP_NOM));
	printHex("iAvgEmpty  ", batteryGauge.readRegister(MAX1720X_ADDRESS_I_AVG_EMPTY));
	printHex("fullCapRep ", batteryGauge.readRegister(MAX1720X_ADDRESS_FULL_CAP_REP));
	printHex("designCap  ", batteryGauge.readRegister(MAX1720X_ADDRESS_DESIGN_CAP));
	printHex("convgCfg   ", batteryGauge.readRegister(MAX1720X_ADDRESS_CONVG_CFG));
	printHex("nVCfg0     ", batteryGauge.readRegister(MAX1720X_ADDRESS_NVC_FG_0));
	printHex("nVCfg1     ", batteryGauge.readRegister(MAX1720X_ADDRESS_NVC_FG_1));
	printHex("nVCfg2     ", batteryGauge.readRegister(MAX1720X_ADDRESS_NVC_FG_2));
	printHex("rSense     ", batteryGauge.readRegister(MAX1720X_ADDRESS_R_SENSE));
}

struct StTime {
	uint8_t seconds = 0;
	uint8_t minutes = 0;
	uint8_t hours = 0;
	uint8_t days = 0;
};

/**
 * Convert seconds into a struct of day, minutes, seconds and hours
 */
struct StTime convertTime(uint32_t timeInSeconds) {
	StTime result;
	if (timeInSeconds > 0) {
		result.seconds = (timeInSeconds % 60);
		result.minutes = (timeInSeconds % 3600) / 60;
		result.hours = (timeInSeconds % 86400) / 3600;
		result.days = (timeInSeconds % (86400 * 30)) / 86400;
	}
	return result;
}

void setup() {
	Serial.begin(115200);
	Wire.begin();

	delay(5000);
	Serial.println("Actual configuration:");
	readAndPrintConfiguration();

	/// Set the register values
	Max1720xConfiguration batteryConfiguration;
	batteryConfiguration.capacity = 350; ///< Capacity in mAh
	// batteryConfiguration.vEmpty = MAX1720X_CONFIG_V_EMPTY(3300, 3880); // 0xA561
	// batteryConfiguration.nVCfg0 = ...

	uint8_t updates = batteryGauge.configure(batteryConfiguration);
	if (updates > 0) {
		Serial.println("Configuration was updated and the battery gauge was reseted");
		readAndPrintConfiguration();
	} else {
		Serial.println("Configuration has not changed");
	}
}

void loop() {
	Serial.println("===========================");
	uint32_t tte = batteryGauge.readTTE();
	uint32_t ttf = batteryGauge.readTTF();
	print("Current        ", "mA", batteryGauge.readCurrent());
	print("AvgCurrent     ", "mA", batteryGauge.readAvgCurrent());
	print("Voltage        ", "mV", batteryGauge.readVCell());
	print("AvgVoltage     ", "mV", batteryGauge.readAvgVCell());
	print("Capacity       ", "mAH", batteryGauge.readRepCap());
	print("FullCapacity   ", "mAH", batteryGauge.readFullCap());
	print("DesignCapacity ", "mAH", batteryGauge.readDesignCap());
	print("StateOfCharge  ", "%", batteryGauge.readRepSOC());
	print("Temperature    ", "C", batteryGauge.readTemp());
	print("TimeToEmpty    ", "s", tte);
	print("TimeToFull     ", "s", ttf);

	StTime time;
	if (tte > 0) {
		time = convertTime(tte);
		Serial.print("TTE            ");
	} else {
		time = convertTime(ttf);
		Serial.print("TTF            ");
	}
	Serial.print(time.days);
	Serial.print(" days ");
	Serial.print(time.hours);
	Serial.print(":");
	Serial.print(time.minutes);
	Serial.print(":");
	Serial.println(time.seconds);
	delay(5000);
}
