#ifndef _VCNL4040_H
#define _VCNL4040_H

#include "hal_conf.h"

#define VCNL4040_ADDRESS (0x60<<1) // 7-bit I2C address
#define VCNL4040_ALS_CONF 0x00
#define VCNL4040_PS_CONF1_2 0x03
#define VCNL4040_PS_CONF3_MS 0x04
#define VCNL4040_PS_DATA 0x08
#define VCNL4040_ALS_DATA 0x09
#define VCNL4040_ID 0x0C

void VCNL4040_Init(void);
void VCNL4040_WriteWord(uint8_t reg, uint16_t value);
uint16_t VCNL4040_ReadWord(uint8_t addr);

#endif
