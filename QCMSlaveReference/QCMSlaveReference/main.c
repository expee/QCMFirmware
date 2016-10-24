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

#define RS_DATA		PORTA|=1<<PORTA2
#define RS_INST		PORTA&=~(1<<PORTA2)
#define EN_ON		PORTA|=1<<PORTA0
#define EN_OFF		PORTA&=~(1<<PORTA0)
#define RW_READ		PORTA|=1<<PORTA1
#define RW_WRITE	PORTA&=~(1<<PORTA1)
#define D7_HIGH		PORTA|=1<<PORTA7
#define D6_HIGH		PORTA|=1<<PORTA6
#define D5_HIGH		PORTA|=1<<PORTA5
#define D4_HIGH		PORTA|=1<<PORTA4
#define D7_LOW		PORTA&=~(1<<PORTA7)
#define D6_LOW		PORTA&=~(1<<PORTA6)
#define D5_LOW		PORTA&=~(1<<PORTA5)
#define D4_LOW		PORTA&=~(1<<PORTA4)

#define LE_HIGH		PORTC|=1<<PORTC1
#define MR_HIGH		PORTC|=1<<PORTC0
#define S1_HIGH		PORTC|=1<<PORTC7
#define	S2_HIGH		PORTC|=1<<PORTC6
#define S3_HIGH		PORTC|=1<<PORTC5
#define LE_LOW		PORTC&=~(1<<PORTC1)
#define MR_LOW		PORTC&=~(1<<PORTC0)
#define S1_LOW		PORTC&=~(1<<PORTC7)
#define	S2_LOW		PORTC&=~(1<<PORTC6)
#define S3_LOW		PORTC&=~(1<<PORTC5)

#define Z1			((PINC>>PORTC2)&0x01)
#define Z2			((PINC>>PORTC3)&0x01)
#define Z3			((PINC>>PORTC4)&0x01)

volatile unsigned char x = 0;
volatile uint32_t freqVal = 100000, freqBuff0 = 0, freqBuff1 = 0, freqBuff2 = 0;
volatile unsigned char swt = 0;

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
	//TODO : define this function body
}
//==============================I2C FUNCTIONS=============================================

//==============================TIMER FUNCTIONS=============================================
void initTimer1 ()
{
	//TODO : Make the right initialization
	TCCR1A = 0x00;
	TCCR1B = 0x05;
	TIMSK |=(1<<TOIE1);
	TCNT1 = 0xc2f7;
}
//==============================TIMER FUNCTIONS=============================================

void initExternalInt (void)
{
	
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
	freqVal = (freqBuff2 << 16)|(freqBuff1 << 8)|(freqBuff0);
	freqBuff0 = 0;
	freqBuff1 = 0;
	freqBuff2 = 0;
}

int main(void)
{
	//TODO : correct these
	DDRA = 0x00;
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x00;

	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	sei();
	i2c_Init(300000,0x51);
	/* Replace with your application code */
	while (1)
	{
		
	}
}

