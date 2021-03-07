#ifndef MAX1720X_H_
#define MAX1720X_H_

#include <Arduino.h>
#include <Wire.h>

#define MAX1720X_CONFIG_V_EMPTY(emptyVoltage, recoveryVoltage) (((emptyVoltage / 20) << 8) | (recoveryVoltage / 40))

const uint16_t MAX1720X_ADDRESS_V_EMPTY = 0x19E;
const uint16_t MAX1720X_ADDRESS_LEARN_CFG = 0x19F;
const uint16_t MAX1720X_ADDRESS_QR_TABLE_00 = 0x1A0;
const uint16_t MAX1720X_ADDRESS_QR_TABLE_10 = 0x1A1;
const uint16_t MAX1720X_ADDRESS_QR_TABLE_20 = 0x1A2;
const uint16_t MAX1720X_ADDRESS_QR_TABLE_30 = 0x1A3;
const uint16_t MAX1720X_ADDRESS_FULL_CAP_NOM = 0x1A5;
const uint16_t MAX1720X_ADDRESS_I_AVG_EMPTY = 0x1A8;
const uint16_t MAX1720X_ADDRESS_FULL_CAP_REP = 0x1A9;
const uint16_t MAX1720X_ADDRESS_DESIGN_CAP = 0x1B3;
const uint16_t MAX1720X_ADDRESS_CONVG_CFG = 0x1B7;
const uint16_t MAX1720X_ADDRESS_NVC_FG_0 = 0x1B8;
const uint16_t MAX1720X_ADDRESS_NVC_FG_1 = 0x1B9;
const uint16_t MAX1720X_ADDRESS_NVC_FG_2 = 0x1BA;
const uint16_t MAX1720X_ADDRESS_R_SENSE = 0x1CF;

typedef struct Max1720xConfiguration {
	/// Capacity in mA. The conversion will be done in the configure method
	uint16_t capacity = 0x0000;

	/// Alternate Initial Value: 0xA561 (VE=3.3V, VR=3.88V)
	/// The nVempty register sets thresholds related to empty
	/// detection during operation. Figure 23 shows the register
	/// format.
	/// VE: Empty Voltage. Sets the voltage level for detecting empty.
	/// A 20mV resolution gives a 0 to 5.11V range. This value is writ-
	/// ten to 3.3V after reset if nonvolatile backup is disabled.
	/// VR: Recovery Voltage. Sets the voltage level for clearing
	/// empty detection. Once the cell voltage rises above this
	/// point, empty voltage detection is reenabled. A 40mV reso-
	/// lution gives a 0 to 5.08V range. This value is written to
	/// 3.88V after reset if nonvolatile backup is disabled.
	uint16_t vEmpty = 0x0000; // 0x19E
	uint16_t learnCfg = 0x2602; // 0x19F

	uint16_t qRTable00 = 0x3C00; // 0x1A0
	uint16_t qRTable10 = 0x1B80; // 0x1A1
	uint16_t qRTable20 = 0x0B04; // 0x1A2
	uint16_t qRTable30 = 0x0885; // 0x1A3

	/// Typical current seen in by the application a the point
	/// where the cell reaches empty. Factory default 0x0000
	uint16_t iAvgEmpty = 0x0000; // 0x1A8

	/// The nConvgCfg register configures operation of the
	/// converge to empty feature. The recommended value for
	/// nConvgCfg is 0x2241
	uint16_t convgCfg = 0x2241; // 0x1B7

	uint16_t nVCfg0 = 0x0100; // 0x1B8
	uint16_t nVCfg1 = 0x0006; // 0x1B9
	uint16_t nVCfg2 = 0xFF0A; // 0x1BA
} Max1720xConfiguration;

class Max1720x {
public:
	Max1720x(TwoWire *twoWire, uint16_t _shunt);

	/**
	 * Configures the MAX170x with given configuration.
	 *
	 * Will read the actual configuration before writing
	 * to prevent unnecessary "resetting" the battery gauge
	 * For manually resetting the battery just call reset
	 * @return returns the number of configurations that
	 *         have been updated.
	 */
	uint8_t configure(Max1720xConfiguration config);

	void reset();

	/**
	 * RepSOC is a filtered version of the AvSOC register
	 * that prevents large jumps in the reported value caused
	 * by changes in the application such as abrupt changes
	 * in load current. RepSOC corresponds to RepCap and
	 * FullCapRep. RepSOC is intended to be the final state of
	 * charge percentage output for use by the application
	 */
	double readRepSOC();

	/**
	 * Each update cycle, the lowest reading from all cell volt-
	 * age measurements is placed in the VCell register. VCell
	 * is used as the voltage input to the fuel gauge algorithm
	 */
	double readVCell();

	/**
	 * The AvgVCell register reports an average of the VCell
	 * register readings. The time period for averaging is con-
	 * figurable from a 12-second to 24-minute time period. See
	 * the FilterCfg register description for details on setting the
	 * time filter.
	 */
	double readAvgVCell();

	/**
	 * The IC measures the voltage between the CSP and CSN
	 * pins and the result is stored as a two complement value
	 * in the Current register. Voltages outside the minimum and
	 * maximum register values are reported as the minimum or
	 * maximum value.
	 */
	double readCurrent();

	/**
	 * The AvgCurrent register reports an average of Current
	 * register readings over a configurable 0.7-second to 6.4-
	 * hour time period. See the FilterCfg register description for
	 * details on setting the time filter.
	 */
	double readAvgCurrent();

	/**
	 * The Age register contains a calculated percentage value of
	 * the application's present cell capacity compared to its expected
	 * capacity. The result can be used by the host to gauge the
	 * battery pack health as compared to a new pack of the same type.
	 * The equation for the register output is:
	 * Age register = 100% x (FullCapNom register/DesignCap register)
	 */
	double readAge();

	/**
	 * RepCap or reported capacity is a filtered version of the
	 * AvCap register that prevents large jumps in the reported
	 * value caused by changes in the application such as
	 * abrupt changes in temperature or load current.
	 */
	double readRepCap();

	/**
	 * This register reports the full capacity that goes with
	 * RepCap, generally used for reporting to the user. A new
	 * full-capacity value is calculated at the end of every charge
	 * cycle in the application.
	 */
	double readFullCapRep();

	/**
	 * This register holds the calculated full capacity of the cell
	 * based on all inputs from the ModelGauge m5 algorithm
	 * including empty compensation. A new full-capacity value is
	 * calculated continuously as application conditions change.
	 */
	double readFullCap();

	/**
	 * This register holds the calculated full capacity of the cell,
	 * not including temperature and empty compensation. A
	 * new full-capacity nominal value is calculated each time a
	 * cell relaxation event is detected. This register is used to
	 * calculate other outputs of the ModelGauge m5 algorithm.
	 */
	double readFullCapNom();

	/**
	 * The nDesignCap register holds the expected capacity of
	 * the cell. This value is used to determine age and health
	 * of the cell by comparing against the measured present
	 * cell capacity
	 */
	double readDesignCap();

	/**
	 * The TTE register holds the estimated time to empty for the
	 * application under present temperature and load conditions. The
	 * TTE value is determined by dividing the AvCap register by the
	 * AvgCurrent register. The corresponding AvgCurrent filtering
	 * gives a delay in TTE empty, but provides more stable results
	 */
	uint32_t readTTE();

	/**
	 * The TTF register holds the estimated time to full for the
	 * application under present conditions. The TTF value is
	 * determined by learning the constant-current and con-
	 * stant-voltage portions of the charge cycle based on expe-
	 * rience of prior charge cycles. Time to full is then estimated
	 * by comparing present charge current to the charge ter-
	 * mination current
	 */
	uint32_t readTTF();

	/**
	 * The Temp register value is selected from Temp1, Temp2,
	 * IntTemp, or alternating between Temp1 and Temp2 regis-
	 * ters as determined by the nPackCfg register setting. The
	 * Temp register is the input to the fuel gauge algorithm.
	 * Contents of Temp are indeterminate for the first conver-
	 * sion cycle time period after IC power-up.
	 */
	double readTemp();

	/**
	 * Page 65
	 * The Status register maintains all flags related to alert
	 * thresholds and battery insertion or removal. Figure 45
	 * shows the Status register format
	 */
	uint16_t readStatus();

	/**
	 * Reading 2 bytes from the given register address
	 */
	uint16_t readRegister(uint16_t registerAddress);

private:
	TwoWire *wire;
	double shunt;
	uint16_t rSense;
	double factCapacity;
	double factCurrent;

	double readCapacity(uint16_t registerAddress);
	double readPercentage(uint16_t aregisterAddressddr);
	double readVoltage(uint16_t registerAddress);
	double readCurrent(uint16_t registerAddress);
	double readTemperature(uint16_t registerAddress);
	double readResistance(uint16_t registerAddress);
	uint32_t readTime(uint16_t registerAddress);
	double readDouble(uint16_t registerAddress);

	void writeRegister(uint16_t registerAddress, uint16_t value);
	uint8_t updateConfig(uint16_t registerAddress, uint16_t value);

};
#endif  // MAX1720X_H_
