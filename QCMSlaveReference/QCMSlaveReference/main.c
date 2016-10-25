/*
 * QCMSlaveReference.c
 *
 * Created: 23/10/2016 23.22.27
 * Author : Experian
 */ 

#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../../CommonLibs/i2c_atmega.h"
#include "../../CommonLibs/commonValues.h"

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


volatile freqData_t refFreq;

volatile uint32_t freqBuff0 = 0, freqBuff1 = 0, freqBuff2 = 0;

//==============================I2C FUNCTIONS=============================================
void setSpecificI2c_prepComm (uint8_t cmd, uint8_t* payLoad, uint8_t payLoadSize)
{
	//slave do nothing here
}

void setSpecificI2c_restartDataDir(uint8_t cmd)
{
	//slave do nothing here
}

void i2c_processCommand(uint8_t cmd)
{
	uint8_t i = 0;
	switch (cmd)
	{
		case SLAVE_SEND_MEASUREMENT_DATA:
		{
			for (i = 0; i < sizeof(freqData_t); i++)
			{
				s_payLoad[i] = refFreq.dataTrain[i];
			}
		}break;
		case SLAVE_SEND_THERMAL_DATA:
		{
			//NOP
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
	refFreq.freqVal = (freqBuff2 << 16)|(freqBuff1 << 8)|(freqBuff0);
	freqBuff0 = 0;
	freqBuff1 = 0;
	freqBuff2 = 0;
}

int main(void)
{
	DDRA = 0x00;
	DDRB = 0x1f;
	DDRC = 0x00;
	DDRD = 0x00;
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x04;	//Enable pull-up resistor on PD2

	i2c_Init(300000,0x51);
	initExternalInt();
	sei();
	
	/* Replace with your application code */
	while (1)
	{
		
	}
}

