#include <max1720x.h>

#define MAX1720X_CHECK_BIT(var, pos) ((var) & (1<<(pos)))

// MAX1720X I2C address
const int8_t MAX1720X_ADDRESS_1 = 0x36;
const int8_t MAX1720X_ADDRESS_2 = 0x0B;

Max1720x::Max1720x(TwoWire *twoWire, uint16_t _shunt) {
	wire = twoWire;
	rSense = _shunt * 100;
	shunt = (double) _shunt / 1000;
	factCapacity = 5 / shunt;
	factCurrent = 0.0015625 / shunt;
}

uint8_t Max1720x::configure(Max1720xConfiguration config) {
	uint16_t capacity = rSense / 500 * config.capacity;
	uint8_t configHasChanged = 0;

	configHasChanged += updateConfig(MAX1720X_ADDRESS_V_EMPTY, config.vEmpty);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_LEARN_CFG, config.learnCfg);

	configHasChanged += updateConfig(MAX1720X_ADDRESS_QR_TABLE_00, config.qRTable00);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_QR_TABLE_10, config.qRTable10);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_QR_TABLE_20, config.qRTable20);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_QR_TABLE_30, config.qRTable30);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_FULL_CAP_NOM, capacity); // fullCapNom = designCap * 1.16 ??
	configHasChanged += updateConfig(MAX1720X_ADDRESS_I_AVG_EMPTY, config.iAvgEmpty);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_FULL_CAP_REP, capacity);

	configHasChanged += updateConfig(MAX1720X_ADDRESS_DESIGN_CAP, capacity);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_CONVG_CFG, config.convgCfg);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_NVC_FG_0, config.nVCfg0);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_NVC_FG_1, config.nVCfg1);
	configHasChanged += updateConfig(MAX1720X_ADDRESS_NVC_FG_2, config.nVCfg2);

	configHasChanged += updateConfig(MAX1720X_ADDRESS_R_SENSE, rSense);

	if (configHasChanged) {
		reset();
	}
	return configHasChanged;
}

void Max1720x::reset() {
	writeRegister(0x00BB, 0x0001);
	wire->beginTransmission(MAX1720X_ADDRESS_2);
	wire->write(0x80);
	wire->endTransmission();
}

double Max1720x::readDouble(uint16_t registerAddress) {
	return readRegister(registerAddress);
}

double Max1720x::readCapacity(uint16_t registerAddress) {
	return readRegister(registerAddress) * factCapacity;
}

double Max1720x::readPercentage(uint16_t registerAddress) {
	return readRegister(registerAddress) / 256;
}

double Max1720x::readVoltage(uint16_t registerAddress) {
	return readRegister(registerAddress) * 0.078125;
}

double Max1720x::readCurrent(uint16_t registerAddress) {
	return (int16_t)readRegister(registerAddress) * factCurrent;
}

double Max1720x::readTemperature(uint16_t registerAddress) {
	return readRegister(registerAddress) / 256;
}

double Max1720x::readResistance(uint16_t registerAddress) {
	return readRegister(registerAddress) / 4096;
}

uint32_t Max1720x::readTime(uint16_t registerAddress) {
	uint16_t time = readRegister(registerAddress);
	if (time == 0xffff) {
		return 0;
	} else {
		return readRegister(registerAddress) * 5.625;
	}
}

double Max1720x::readVCell() {
	return readVoltage(0x09);
}

double Max1720x::readAvgVCell() {
	return readVoltage(0x19);
}

double Max1720x::readCurrent() {
	return readCurrent(0x0A);
}

double Max1720x::readAvgCurrent() {
	return readCurrent(0x0b);
}

double Max1720x::readFullCapRep() {
	return readCapacity(0x35);
}

double Max1720x::readRepCap() {
	return readCapacity(0x05);
}

double Max1720x::readFullCap() {
	return readCapacity(0x10);
}

double Max1720x::readFullCapNom() {
	return readCapacity(0xA5);
}

double Max1720x::readDesignCap() {
	return readCapacity(0x18);
}

uint32_t Max1720x::readTTE() {
	return readTime(0x11);
}

uint32_t Max1720x::readTTF() {
	return readTime(0x20);
}

double Max1720x::readRepSOC() {
	return readPercentage(0x06);
}
double Max1720x::readAge() {
	return readPercentage(0x07);
}

double Max1720x::readTemp() {
	return readTemperature(0x08);
}

uint16_t Max1720x::readStatus() {
	return readRegister(0x00);
}

//uint16_t Max1720x::readRegister(uint8_t addr) {
//	wire->beginTransmission(MAX1720X_ADDRESS_2);
//	wire->write((uint8_t) addr);
//	wire->endTransmission(false);
//	wire->requestFrom(MAX1720X_ADDRESS_1, 2, HIGH);
//
//	uint16_t combined = wire->read() | (wire->read() << 8);
//	return combined;
//}
//uint16_t Max1720x::readRegister(uint8_t addr) {
//	wire->beginTransmission(MAX1720X_ADDRESS_1);
//	wire->write((uint8_t) addr);
//	wire->endTransmission(false);
//	wire->requestFrom(MAX1720X_ADDRESS_1, 2, HIGH);
//	uint16_t combined = wire->read() | (wire->read() << 8);
//	return combined;
//}

uint16_t Max1720x::readRegister(uint16_t addr) {
	if (MAX1720X_CHECK_BIT(addr, 8)) {
		wire->beginTransmission(MAX1720X_ADDRESS_2);
	} else {
		wire->beginTransmission(MAX1720X_ADDRESS_1);
	}
	wire->write((uint8_t) addr);
	wire->endTransmission(false);

	if (MAX1720X_CHECK_BIT(addr, 9)) {
		wire->requestFrom(MAX1720X_ADDRESS_1, 2, HIGH);
	} else {
		wire->requestFrom(MAX1720X_ADDRESS_1, 2, HIGH);
	}

	uint16_t combined = wire->read() | (wire->read() << 8);
	return combined;
}
//
//int16_t Max1720x::readInt16_t(uint8_t addr) {
//	return readUInt16_t(addr);
//}

void Max1720x::writeRegister(uint16_t registerAddress, uint16_t value) {
	if (MAX1720X_CHECK_BIT(registerAddress, 8)) {
		wire->beginTransmission(MAX1720X_ADDRESS_2);
	} else {
		wire->beginTransmission(MAX1720X_ADDRESS_1);
	}
	wire->write((uint8_t) registerAddress);
	wire->write((uint8_t) value);
	wire->write((uint8_t) (value >> 8));
	wire->endTransmission(false);
}

uint8_t Max1720x::updateConfig(uint16_t registerAddress, uint16_t value) {
	uint16_t actual = readRegister(registerAddress);
	if (actual != value) {
		writeRegister(registerAddress, value);
		return 1;
	}
	return 0;
}

