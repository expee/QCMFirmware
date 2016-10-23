/*
	Made by Experian
	for Atmega6 and Atmega16
*/
#ifndef I2C_ATMEGA_H
#define I2C_ATMEGA_H

#include <stddef.h>
#include <avr/interrupt.h>

#define WRITE					0x00
#define READ					0x01
#define MAX_DATA_LENGTH			8
#define MAX_NUMBER_OF_RETRIES	100

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

enum
{
	START_OK = 0x08,
	RESTART_OK = 0x10,
	SLA_W_ACK = 0x18,
	SLA_W_NACK = 0x20,
	DATA_TRANSMITTED_ACK = 0x28,
	DATA_TRANSMITTED_NACK = 0x30,
	ABR_LOST = 0x38,
	SLA_R_ACK = 0x40,
	SLA_R_NACK = 0x48,
	DATA_RECEIVED_ACK = 0x50,
	DATA_RECEIVED_NACK = 0x58,
	AS_SLAVE_OWN_SLA_W_RECEIVED = 0x60,
	AS_SLAVE_ABR_LOST_ON_SLA_W = 0x68,
	AS_SLAVE_GENERAL_CALL_RECEIVED = 0x70,
	AS_SLAVE_ABR_LOST_ON_GENERAL_CALL = 0x78,
	AS_SLAVE_DATA_RECEIVED_ACK = 0x80,
	AS_SLAVE_DATA_RECEIVED_NACK = 0x88,
	AS_SLAVE_DATA_RECEIVED_GEN_CALL_ACK = 0x90,
	AS_SLAVE_DATA_RECEIVED_GEN_CALL_NACK = 0x98,
	AS_SLAVE_STOP_COND_RECEIVED = 0xa0,
	AS_SLAVE_SLA_R_RECEIVED_ACK = 0xa8,
	AS_SLAVE_ABR_LOST_ON_SLA_R = 0xb0,
	AS_SLAVE_DATA_TRANSMITTED_ACK = 0xb8,
	AS_SLAVE_DATA_TRANSMITTED_NACK = 0xc0,
	AS_SLAVE_LAST_DATA_BYTE_TRANSMITTED_ACK = 0xc8,
	NO_RELEVANT_STATE = 0xf8,
	BUS_ERROR = 0x00,
	ERROR_RETRY_LIMIT_REACHED = 0xff
}i2c_condition;

//========================================================================================================
/*This enum must be defined by the user, this is just an example
enum
{
	NO_COMMAND = 0,
	SEND_MEASUREMENT_DATA,
	SEND_THERMAL_DATA,
	TEST_COMM
}i2cCommand;
*/
//========================================================================================================

volatile uint8_t s_slaveAddr = 0;
volatile uint8_t s_data = 0;
volatile uint8_t s_rw = 0;
volatile uint8_t s_TWIStat = 0;
volatile uint8_t s_remainingData = 0;
volatile uint8_t s_isI2CBusy = 0;
volatile uint8_t s_numOfData = 0;
volatile uint8_t s_cmd = 0;
volatile uint8_t s_retries = 0;
volatile uint8_t s_isCmdSent = 0;

volatile int8_t s_dataSeqNumber = 0;

volatile uint8_t s_payLoad[MAX_DATA_LENGTH];
volatile uint8_t s_dataStorage[MAX_DATA_LENGTH];

volatile uint8_t imHere = 0;

void i2c_Init(uint32_t speed, uint8_t selfAddr)
{
	if (speed < 60000) speed = 60000;
	else if (speed > 400000) speed = 400000;
	TWAR = (selfAddr << 1);
	TWAR &= ~(1<<TWGCE);
	TWSR = 0x00;
	speed = ((F_CPU/speed)-16)/2;
	TWBR = (uint8_t) speed;
	TWCR |= (1<<TWEA);
	TWCR |= (1<<TWIE);
	TWCR |= (1<<TWEN);
}

void i2c_start(void)
{
	TWCR |= (1<<TWSTA);
	TWCR |= (1<<TWINT);
}

//========================================================================================================
//These 3 functions need to be defined by the user, body example is given as below
void setSpecificI2c_prepComm (uint8_t cmd, uint8_t* payLoad, uint8_t payLoadSize);
	/*switch (cmd)
	{
		case CMD_1:
		{
			do something with the payload here
		}break;
		case CMD_2:
		{
			do something with the payload here
		}break;
	}*/

void setSpecificI2c_restartDataDir(uint8_t cmd);

	/*switch (cmd)
	{
		case CMD_1:
		{
			TWDR |= READ;	this means CMD_1 is a read-data-from-slave command
		}break;
		case CMD_2:
		{
			TWDR |= WRITE;	this means CMD_1 is a write-data-to-slave command
		}break;
	}*/

void i2c_processCommand(uint8_t cmd);
	/*switch (cmd)
	{
		case CMD_1:
		{
			do something to do what's told by the master, maybe fill the data array?
		}break;
		case CMD_2:
		{
			do something to do what's told by the master, maybe turn on a LED?
		}break;
	}*/
//========================================================================================================

void i2c_prepComm (uint8_t slaveAddr, uint8_t cmd, uint8_t* payLoad, uint8_t payLoadSize)
{
	//Default prepComm init for any command given
	s_isI2CBusy = 1;
	s_slaveAddr = slaveAddr;
	s_numOfData = payLoadSize;
	s_cmd = cmd;
	s_isCmdSent = 0;
	
	//specific prepComm init per command given
	setSpecificI2c_prepComm (s_cmd, payLoad, s_numOfData);
}

void i2c_stop (void)
{
	TWCR |= (1<<TWSTO);
	TWCR |= (1<<TWINT);
}


void i2c_deInit()
{
	TWCR = 0x00;
	TWBR = 0x00;
	TWSR = 0x00;
}

void i2c_resetStatus ()
{
	uint8_t i = 0;
	s_data = 0;
	s_remainingData = 0;
	s_isI2CBusy = 0;
	s_numOfData = 0;
	s_cmd = 0;
	s_retries = 0;
	s_dataSeqNumber = 0;
	for(; i < MAX_DATA_LENGTH; i++) s_payLoad[i] = 0;
}

void i2c_saveData()
{
	uint8_t i = 0;
	for(; i < MAX_DATA_LENGTH; i++) s_dataStorage [i] = s_payLoad[i];
}

ISR (TWI_vect, ISR_BLOCK)
{
	switch(TWSR&0xf8)
	{
//==================================MASTER TRANSMITTER MODE=======================================
		case START_OK:
		{	
			//load the slave address that's being addressed
			TWDR = (s_slaveAddr << 1);
			TWDR &= 0xfe;	//just to make sure that LSB of TWDR will be zero
			TWDR |= WRITE;	//READ/WRITE bit at START will always set as WRITE to send the command byte properly to the addressed slave
			TWCR &= ~(1<<TWSTA);
			TWCR |= (1<<TWINT);
		}break;
		case RESTART_OK:
		{
			//load the slave address that's being addressed
			TWDR = (s_slaveAddr << 1);
			TWDR &= 0xfe; //just to make sure that LSB of TWDR will be zero
			setSpecificI2c_restartDataDir(s_cmd);
			TWCR &= ~(1<<TWSTA);
			TWCR |= (1<<TWINT);
		}break;
		case SLA_W_ACK:
		{
			//Load the data and send it to the slave
			if(!s_isCmdSent)
				TWDR = s_cmd;
			else
				TWDR = s_data;
			TWCR |= (1<<TWINT);
		}break;
		case SLA_W_NACK:
		{
			//reset s_remainingData and s_data value
			s_remainingData = s_numOfData - 1;
			s_data = s_payLoad[0];
			s_retries++;
			if(s_retries > MAX_NUMBER_OF_RETRIES)
			{
				//send STOP condition and go idle
				i2c_resetStatus();
				TWCR |= (1<<TWSTO);
				TWCR |= (1<<TWINT);
			}
			else
			{
				TWCR |= (1<<TWSTA);
				TWCR |= (1<<TWINT);
			}
		}break;
		case DATA_TRANSMITTED_ACK:
		{
			//command has just been sent and now it's time to restart the comm line
			if(s_isCmdSent == 0)
			{
				s_isCmdSent = 1;
				TWCR |= (1<<TWSTA);
				TWCR |= (1<<TWINT);
			}
			//restart has been done and now it's time to sent the payload
			else
			{
				if (s_remainingData == 0)
				{
					//End transmission
					i2c_resetStatus();
					TWCR |= (1<<TWSTO);
					TWCR |= (1<<TWINT);
				}
				else
				{
					s_data = s_payLoad[s_numOfData-s_remainingData];
					s_remainingData--;
					TWDR = s_data;
					TWCR |= (1<<TWINT);
				}
			}
		}break;
		case DATA_TRANSMITTED_NACK:
		{
			//End transmission
			i2c_resetStatus();
			TWCR |= (1<<TWSTO);
			TWCR |= (1<<TWINT);
		}break;
		case ABR_LOST:
		{
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
//==================================MASTER RECEIVER MODE=======================================
		case SLA_R_ACK:
		{	
			//data will be received and ACK must be given
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case SLA_R_NACK:
		{
			//reset the s_remainingData counter and restart the transmission
			s_remainingData = s_numOfData - 1;
			s_retries++;
			if(s_retries > MAX_NUMBER_OF_RETRIES)
			{
				//send STOP condition and go idle
				i2c_resetStatus();
				TWCR |= (1<<TWSTO);
				TWCR |= (1<<TWINT);
			}
			else
			{
				TWCR |= (1<<TWSTA);
				TWCR |= (1<<TWINT);
			}
		}break;
		case DATA_RECEIVED_ACK:
		{
			//if there's more data to be received try to do so (ACK must be given)
			if(s_remainingData > 1)
			{
				s_payLoad[s_numOfData-s_remainingData] = TWDR;
				s_remainingData--;
				TWCR |= (1<<TWEA);
				TWCR |= (1<<TWINT);
			}
			else if(s_remainingData == 1)
			{
				s_payLoad[s_numOfData-s_remainingData] = TWDR;
				s_remainingData--;
				TWCR &= ~(1<<TWEA);
				TWCR |= (1<<TWINT);
			}
		}break;
		case DATA_RECEIVED_NACK:
		{
			s_payLoad[s_numOfData-s_remainingData] = TWDR;
			//end transmission
			i2c_saveData();
			i2c_resetStatus();
			TWCR |= (1<<TWSTO);
			TWCR |= (1<<TWINT);
		}break;
//==================================SLAVE RECEIVER MODE=======================================
		case AS_SLAVE_OWN_SLA_W_RECEIVED:
		{
			//data byte will be received and ACK will be returned
			//this is in order for the master to initiate restart and send the command byte
			//the sequence number -1 is the command byte, s_cmd == 0 means there's no command yet
			if (s_cmd == 0) s_dataSeqNumber = -1;
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_ABR_LOST_ON_SLA_W:
		{
			//data byte will be received and NOT ACK will be returned
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_GENERAL_CALL_RECEIVED:
		{
			//data byte will be received and NOT ACK will be returned
			//disable general call for now !!TODO Later!!
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_DATA_RECEIVED_ACK:
		{
			if(s_dataSeqNumber == -1)
			{
				s_cmd =TWDR;
				i2c_processCommand(s_cmd);
				s_dataSeqNumber = 0;
			}
			else
			{
				s_payLoad[s_dataSeqNumber] = TWDR;
				s_dataSeqNumber++;
			}
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_DATA_RECEIVED_NACK:
		{
			//end transmission
			i2c_saveData();
			i2c_resetStatus();	
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_DATA_RECEIVED_GEN_CALL_ACK:
		{
			//try to receive more data
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_DATA_RECEIVED_GEN_CALL_NACK:
		{
			i2c_resetStatus();
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_STOP_COND_RECEIVED:
		{
			//below means the command is not just been received, for if it is then s_dataSeqNumber will be 0
			if(s_dataSeqNumber != 0)
			{
				i2c_saveData();
				i2c_resetStatus();
			}
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
//==================================SLAVE TRANSMITTER MODE=======================================
		case AS_SLAVE_SLA_R_RECEIVED_ACK:
		{
			//prepare the data and send it to the master
			s_dataSeqNumber = 0;
			TWDR = s_payLoad[s_dataSeqNumber];
			s_dataSeqNumber++;
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_ABR_LOST_ON_SLA_R:
		{
			//stop transmission
			i2c_resetStatus();
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_DATA_TRANSMITTED_ACK:
		{
			//send more data as long as master doesn't end the transmission
			TWDR = s_payLoad[s_dataSeqNumber];
			s_dataSeqNumber++;
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_DATA_TRANSMITTED_NACK:
		{
			//end transmission
			i2c_resetStatus();
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
		case AS_SLAVE_LAST_DATA_BYTE_TRANSMITTED_ACK:
		{
			//end transmission
			i2c_resetStatus();
			TWCR |= (1<<TWEA);
			TWCR |= (1<<TWINT);
		}break;
	}
}

#endif  //I2C_ATMEGA_H
