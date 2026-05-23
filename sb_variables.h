
#ifndef SB_VARIABLES_H_
#define SB_VARIABLES_H_

#include <stdint.h>
#include <string.h>
#include "sb_global_var.h"
#include "sb_const.h"

char RxBuffer[RX_IND_MAX]={0};
uint8_t RxInd=0,RxTimeout=0,RelayDeadTimer=0;

bool bool_msgRcvOK=0, bool_sec_flag=0, bool_msec50_flag=0;

#endif	// SB_VARIABLES_H_
