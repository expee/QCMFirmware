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

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum
{
	NOP,
	SEND_DATA
}commandFromHost;

typedef struct dataToSend
{
	freqData_t sensor;
	freqData_t ref;
	thermData_t temp;
}dataToSend_t;

volatile dataToSend_t data;

volatile uint8_t s_firstUSBTransmission = FALSE;
volatile uint8_t s_isSlavesReady = FALSE;

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *request = (void*) data;

	if (!s_firstUSBTransmission)
	{
		//Set timer here so that the USB comm will not happen the same time as i2c communication
		setTimer1Value();
		s_firstUSBTransmission = TRUE;
	}

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
	switch(cmd)
	{
		case SLAVE_SAY_READY:
		{
			s_isSlavesReady = TRUE;
		}break;
	}
}

void initTimer1 ()
{
	
}

void setTimer1Value(uint16_t timerVal)
{
	
}

void turnOffTimer1 ()
{
	
}

ISR(TIMER1_OVF_vect)
{
	i2c_prepComm(0x51,SEND_MEASUREMENT_DATA,0,sizeof(data.sensor));
	i2c_start();
	while(s_isI2CBusy);
	data.sensor.freqVal = *((uint32_t *) s_dataStorage);	//TYPE ALIASING WARNING!!
	i2c_prepComm(0x52,SEND_MEASUREMENT_DATA,0,sizeof(data.ref));
	i2c_start();
	while(s_isI2CBusy);
	data.ref.freqVal = *((uint32_t *) s_dataStorage);	//TYPE ALIASING WARNING!!
	i2c_prepComm(0x52,SEND_THERMAL_DATA,0,sizeof(data.temp));
	i2c_start();
	while(s_isI2CBusy);
	data.temp.thermalVal = *((uint16_t *) s_dataStorage);

	turnOffTimer1();	//turn off timer here, wait for another USB transmission before ask new data from slaves.
}

int main(void)
{
	uint8_t i = 0;
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
