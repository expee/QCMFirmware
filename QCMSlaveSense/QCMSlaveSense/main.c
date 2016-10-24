/*
 * QCMSlaveSense.c
 *
 * Created: 23/10/2016 23.21.42
 * Author : Experian
 */ 

#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../../CommonLibs/i2c_atmega.h"

#define LE_HIGH		PORTB|=1<<PB0
#define MR_HIGH		PORTB|=1<<PB1
#define S1_HIGH		PORTB|=1<<PB2
#define	S2_HIGH		PORTB|=1<<PB3
#define S3_HIGH		PORTB|=1<<PB4
#define LE_LOW		PORTB&=~(1<<PB0)
#define MR_LOW		PORTB&=~(1<<PB1)
#define S1_LOW		PORTB&=~(1<<PB2)
#define	S2_LOW		PORTB&=~(1<<PB3)
#define S3_LOW		PORTB&=~(1<<PB4)

#define Z1			((PINC>>PC5)&0x01)
#define Z2			((PINC>>PC6)&0x01)
#define Z3			((PINC>>PC7)&0x01)

enum commandSet
{
	NOP = 0,
	SEND_MEASUREMENT_DATA,
	SEND_THERMAL_DATA,
	TEST_COMM,
	TURN_ON_RTC,
	SET_RTC_OUTPUT,
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

volatile freqData_t freqDataToSend;
volatile thermData_t thermalDataToSend;

volatile uint32_t freqBuff0 = 0, freqBuff1 = 0, freqBuff2 = 0;

//==============================I2C FUNCTIONS=============================================
void setSpecificI2c_prepComm (uint8_t cmd, uint8_t* payLoad, uint8_t payLoadSize)
{
	
}

void setSpecificI2c_restartDataDir(uint8_t cmd)
{
	
}

void i2c_processCommand(uint8_t cmd)
{
	uint8_t i = 0;
	switch (cmd)
	{
		case SEND_MEASUREMENT_DATA:
		{
			for (i = 0; i < sizeof(freqData_t); i++)
			{
				s_payLoad[i] = freqDataToSend.dataTrain[i];
			}
		}break;
		case SEND_THERMAL_DATA:
		{
			for (i = 0; i < sizeof(thermData_t); i++)
			{
				s_payLoad[i] = thermalDataToSend.dataTrain[i];
			}
		}break;
		case TEST_COMM:
		{
			//NOP
		}break;
	}
}
//==============================I2C FUNCTIONS=============================================

void initExternalInt (void)
{
	//INT0 triggers on falling edge
	MCUCR |= (1<<ISC01)|(1<<ISC00);
	//Activate INT0 Interrupt
	GICR |=(1<<INT0);
}

void initADC ()
{
	//TODO : Define this
}

ISR(INT0_vect)
{
	LE_HIGH;
	LE_LOW;
	MR_HIGH;
	MR_LOW;
	uint8_t i;
	for (i = 0; i < 8; ++i)
	{
		(i&0x01)?(S1_HIGH):(S1_LOW);
		((i>>1)&0x01)?(S2_HIGH):(S2_LOW);
		((i>>2)&0x01)?(S3_HIGH):(S3_LOW);
		freqBuff0|=Z1<<i;
		freqBuff1|=Z2<<i;
		freqBuff2|=Z3<<i;
	}
	freqDataToSend.freqVal = (freqBuff2 << 16)|(freqBuff1 << 8)|(freqBuff0);
	freqBuff0 = 0;
	freqBuff1 = 0;
	freqBuff2 = 0;
}

int main(void)
{
	uint8_t RTCTurnOnData[2] = {0x00,0x00};
	uint8_t RTCOutEnableData[2] = {0x07,0x10};
	DDRA = 0x00;
	DDRB = 0x1f;
	DDRC = 0x00;
	DDRD = 0x00;
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x04;	//Enable pull-up resistor on PD2
	initADC();
	initExternalInt();
	i2c_Init(300000,0x52);
	sei();
	_delay_ms(100);
	
	//Turn on RTC and set the output to 1Hz
	i2c_prepComm(0x68,TURN_ON_RTC,RTCTurnOnData,sizeof(RTCTurnOnData));
	i2c_start();
	while(s_isI2CBusy);
	i2c_prepComm(0x68,SET_RTC_OUTPUT,RTCOutEnableData,sizeof(RTCOutEnableData));
	i2c_start();
	while(s_isI2CBusy);
	
	//All operation will begin after this command is sent to Master Controller
	i2c_prepComm(0x50,SLAVE_SAY_READY,0,0);
	i2c_start();
	while(s_isI2CBusy);
	
	/* Replace with your application code */
	while (1)
	{
		
	}
}

