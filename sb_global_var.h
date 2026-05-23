
#ifndef SB_GLOBAL_VAR_H_
#define SB_GLOBAL_VAR_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "sb_const.h"

extern char RxBuffer[RX_IND_MAX];
extern uint8_t RxInd, RxTimeout, RelayDeadTimer;
extern bool bool_msgRcvOK, bool_sec_flag, bool_msec50_flag;

#endif	// SB_GLOBAL_VAR_H_

