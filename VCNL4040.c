#include "VCNL4040.h"
#include "i2cmaster.h"

//-------------------------------
// Write 16-bit register to VCNL4040 
//-------------------------------
void VCNL4040_WriteWord(uint8_t reg, uint16_t value) 
{
	I2C1_Start();							// Start condition
	Write_Byte_I2C1(VCNL4040_ADDRESS);		// Write device address
	Write_Byte_I2C1(reg);					// Write address of register
	Write_Byte_I2C1(value & 0xFF);			// DATA_L
	Write_Byte_I2C1((value >> 8) & 0xFF);	// DATA_H
	I2C1_Stop();              				// Send a STOP condition on the TWI bus.
}

//-------------------------------
// Read 16-bit register value from VCNL4040 
//-------------------------------
uint16_t VCNL4040_ReadWord(uint8_t addr)
{
   	uint16_t Data=0;
	
	I2C1_Start();				// Start condition
	Write_Byte_I2C1(VCNL4040_ADDRESS);			// Write device address
	Write_Byte_I2C1(addr);			// Write address of register
	I2C1_Start();				// Start condition
	Write_Byte_I2C1(VCNL4040_ADDRESS+1);			// Write device address
	Data = Read_Byte_I2C1(ACK);
	Data |= ((uint16_t)Read_Byte_I2C1(NO_ACK)<<8);
	I2C1_Stop();              // Send a STOP condition on the TWI bus.		
	
	return Data;
}

// Init VCNL4040: enable ALS + PS 
void VCNL4040_Init(void) 
{
    // PS_CONF1: PS duty 1/40, 16-bit, enable proximity
    VCNL4040_WriteWord(VCNL4040_PS_CONF1_2, 0x083E);
	
    // PS_CONF2: LED current 200mA, gain=1x
    VCNL4040_WriteWord(VCNL4040_PS_CONF3_MS, 0x0770);
}

