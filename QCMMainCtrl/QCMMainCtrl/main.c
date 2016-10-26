/*
 * QCMMainCtrl.c
 *
 * Created: 23/10/2016 19.59.47
 * Author : Experian
 */ 
#define F_CPU 12000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "usbdrv/usbconfig.h"
#include "usbdrv/usbdrv.h"
#include "../../CommonLibs/i2c_atmega.h"
#include "../../CommonLibs/commonValues.h"

#define  LED_ON		PORTB|=1<<PB0
#define  LED_OFF	PORTB&=~(1<<PB0)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum
{
	USB_NOP,
	USB_SEND_DATA
}commandFromHost;

typedef struct dataToSend
{
	freqData_t sensor;
	freqData_t ref;
	thermData_t temp;
}dataToSend_t;

volatile dataToSend_t USBData;

volatile uint8_t s_isSlavesReady = FALSE;

volatile uint8_t LEDDelayCounter = 0;

void initTimer1 ()
{
	TCCR1A = 0x00;
	TIMSK |= (1<<TOIE1); 
}

void initTimer0 ()
{
	TCCR0 = 0x00;
	TCNT0 = 0x00;
	TIMSK |= (1<<TOIE0);
}

void startTimer0 ()
{
	TCCR0 = 0x05;
}

void stopTimer0 ()
{
	TCCR0 = 0x00;
	TCNT0 = 0x00;
}

void setTimer1Value(uint16_t timerVal)
{
	//timerVal in millisecond
	TCCR1B = 0x04;
	TCNT1 = 0xffff - (uint16_t)(((float)timerVal/1000.0)/(256.0/(float)F_CPU));
}

void turnOffTimer1()
{
	TCCR1B = 0x00;
	TCNT1 = 0x00;
}

//==============================I2C FUNCTIONS=============================================
void setSpecificI2c_prepComm (uint8_t cmd, uint8_t* payLoad, uint8_t payLoadSize)
{
	uint8_t i = 0;
	switch (cmd)
	{
		case SLAVE_SEND_MEASUREMENT_DATA:
		{
			s_remainingData = payLoadSize;
		}break;
		case SLAVE_SEND_THERMAL_DATA:
		{
			s_remainingData = payLoadSize;
		}break;
		case TEST_COMM:
		{
			if(payLoad)
			{
				for (i = 0; i < payLoadSize; ++i)
				{
					s_payLoad[i] = payLoad[i];
				}
				s_remainingData = payLoadSize - 1;
				s_data = s_payLoad[0];
			}
			else
			{
				//Invalid payLoad pointer!!!
				for (i = 0; i < payLoadSize; ++i)
				{
					//If you see this continuously in your comm line
					//it means you gave an invalid payLoad pointer!!!
					s_payLoad[i] = 0xaa;
				}
				s_remainingData = payLoadSize - 1;
				s_data = s_payLoad[0];
			}
		}break;
	}
}

void setSpecificI2c_restartDataDir(uint8_t cmd)
{
	switch (cmd)
	{
		case SLAVE_SEND_MEASUREMENT_DATA:
		{
			TWDR |= READ;
		}break;
		case SLAVE_SEND_THERMAL_DATA:
		{
			TWDR |= READ;
		}break;
		case TEST_COMM:
		{
			TWDR |= WRITE;
		}break;
	}
}

void i2c_processCommand(uint8_t cmd)
{
	switch(cmd)
	{
		case SLAVE_SAY_READY:
		{
			s_isSlavesReady = TRUE;
		}break;
	}
}
//==============================I2C FUNCTIONS=============================================

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *request = (void*) data;

	//Set timer here so that the USB comm will not happen the same time as i2c communication
	setTimer1Value(500);
	
	LED_ON;
	startTimer0();	//for LED blinking effect
	
	switch (request->bRequest)
	{
		case USB_SEND_DATA:
		{
			usbMsgPtr = (usbMsgPtr_t) &USBData;
			return sizeof(USBData);
		}break;
	}
	return 0;
}

ISR(TIMER1_OVF_vect, ISR_NOBLOCK)
{
	i2c_prepComm(0x51,SLAVE_SEND_MEASUREMENT_DATA,0,sizeof(USBData.sensor));
	i2c_start();
	while(s_isI2CBusy);
	USBData.sensor.freqVal = *((uint32_t *) s_dataStorage);	//TYPE ALIASING WARNING!!
	i2c_prepComm(0x52,SLAVE_SEND_MEASUREMENT_DATA,0,sizeof(USBData.ref));
	i2c_start();
	while(s_isI2CBusy);
	USBData.ref.freqVal = *((uint32_t *) s_dataStorage);	//TYPE ALIASING WARNING!!
	i2c_prepComm(0x52,SLAVE_SEND_THERMAL_DATA,0,sizeof(USBData.temp));
	i2c_start();
	while(s_isI2CBusy);
	USBData.temp.thermalVal = *((uint16_t *) s_dataStorage);

	turnOffTimer1();	//turn off timer here, wait for another USB transmission before ask new data from slaves.
	

}

ISR(TIMER0_OVF_vect, ISR_NOBLOCK)
{
	LEDDelayCounter++;
	if(LEDDelayCounter > 22)
	{
		LED_OFF;
		LEDDelayCounter = 0;
		stopTimer0();
	}
}

int main(void)
{
	uint8_t i = 0;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	DDRB = 0x01;
	DDRC = 0x00;
	DDRD = 0x00;
	initTimer0();
	initTimer1();
	i2c_Init(300000,0x50);
	sei();
	while(!s_isSlavesReady);

	wdt_enable(WDTO_1S);
	usbInit();
	usbDeviceDisconnect();
	for (i=0;i<250;++i)
	{
		wdt_reset();
		_delay_ms(2);
	}
	usbDeviceConnect();
	
    while (1) 
    {
		wdt_reset();
		usbPoll();
    }
}
