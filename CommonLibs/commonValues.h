#ifndef COMMON_VALUES_H
#define COMMON_VALUES_H

enum commandSet
{
	NOP = 0,
	SLAVE_SEND_MEASUREMENT_DATA,
	SLAVE_SEND_THERMAL_DATA,
	TEST_COMM,
	RTC_TURN_ON,
	RTC_SET_OUTPUT,
	SLAVE_SAY_READY
};

typedef union freqData
{
	volatile uint32_t freqVal;
	volatile uint8_t dataTrain[4];
}freqData_t;

typedef union thermData
{
	volatile uint16_t thermalVal;
	volatile uint8_t dataTrain[2];
}thermData_t;


#endif //COMMON_VALUES_H