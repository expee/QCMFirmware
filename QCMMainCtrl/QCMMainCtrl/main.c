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

enum
{
	NOP,
	SEND_DATA
}commandFromHost;

enum
{
	NO_COMMAND = 0,
	SEND_MEASUREMENT_DATA,
	SEND_THERMAL_DATA,
	TEST_COMM
}i2cCommand;

typedef struct dataToSend
{
	uint32_t sensorFreqVal;
	uint32_t refFreqVal;
	uint8_t tempVal;
}dataToSend_t;

volatile dataToSend_t data;

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *request = (void*) data;
	switch (request->bRequest)
	{
		case SEND_DATA:
			{
				usbMsgPtr = (usbMsgPtr_t) &data;
				return sizeof(data);
			}break;
	}
	return 0;
}

void setSpecificI2c_prepComm (uint8_t cmd, uint8_t* payLoad, uint8_t payLoadSize)
{
	uint8_t i = 0;
	switch (cmd)
	{
		case SEND_MEASUREMENT_DATA:
		{
			s_remainingData = payLoadSize;
		}break;
		case SEND_THERMAL_DATA:
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
		case SEND_MEASUREMENT_DATA:
		{
			TWDR |= READ;
		}break;
		case SEND_THERMAL_DATA:
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
	
}

ISR(TIMER0_OVF_vect)
{
	i2c_prepComm(0x51,SEND_MEASUREMENT_DATA,0,sizeof(data.sensorFreqVal));
	i2c_start();
	while(s_isI2CBusy);
	data.sensorFreqVal = *((uint32_t *) s_dataStorage);	//TYPE ALIASING WARNING!!
	i2c_prepComm(0x52,SEND_MEASUREMENT_DATA,0,sizeof(data.refFreqVal));
	i2c_start();
	while(s_isI2CBusy);
	data.refFreqVal = *((uint32_t *) s_dataStorage);	//TYPE ALIASING WARNING!!
	i2c_prepComm(0x52,SEND_THERMAL_DATA,0,sizeof(data.tempVal));
	i2c_start();
	while(s_isI2CBusy);
	data.tempVal = *((uint8_t *) s_dataStorage);
	//TODO: reset the timer!!
}

int main(void)
{
	uint8_t i = 0;
	sei();
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
