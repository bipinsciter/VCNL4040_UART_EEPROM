/***********************************************************************************************************************
    @file    main.c
    @author  FAE Team
    @date    17-Nov-2023
    @brief   THIS FILE PROVIDES ALL THE SYSTEM FUNCTIONS.
  **********************************************************************************************************************
    @attention

    <h2><center>&copy; Copyright(c) <2023> <MindMotion></center></h2>

      Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
    following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
       the following disclaimer in the documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
       promote products derived from this software without specific prior written permission.

      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *********************************************************************************************************************/

/* Define to prevent recursive inclusion */
#define _MAIN_C_

/* Files include */
#include "stdlib.h"
#include "stdio.h"
#include "platform.h"
#include "gpio_led_toggle.h"
#include "main.h"
#include "VCNL4040.h"
#include "i2cmaster.h"
#include "usart_interrupt.h"
#include "tim1_timebase.h"
#include "sb_variables.h"

/**
  * @addtogroup MM32G0001_LibSamples
  * @{
  */

/**
  * @addtogroup GPIO
  * @{
  */

/**
  * @addtogroup VCNL4040
  * @{
  */

/* Private typedef ****************************************************************************************************/
#define AVG_SAMPLE 10
#define RELAY_CONFIRM_ON_CNT 5
#define RELAY_CONFIRM_OFF_CNT 5
#define UART_DIS_TIME_SEC 600
#define CALL_PRESS_TIME_SEC 5
#define EMERGENCY_SENSE_SEC 15
#define EMERGENCY_AUTO_RESTORE_SEC 120
#define EMERGENCY_FORCE_RESTORE_SEC 5

/* Private define *****************************************************************************************************/

/* Private macro ******************************************************************************************************/

/* Private variables **************************************************************************************************/
//uint8_t gu8_counter=0;
uint16_t ps=0, final_ps=0, last_final_ps=0, threshold=0;
uint16_t raw_cnt[AVG_SAMPLE]={0};
uint32_t gu32_temp=0,raw_cnt_avg=0;
bool relayMsgOn=0,relayMsgOff=0,emergencyTriggered=0;
uint8_t raw_cnt_ind=0, i=0, RelayOnCnt=0, RelayOffCnt=0,CalPressTimer=CALL_PRESS_TIME_SEC,test_cnt=0;
uint8_t emergencySenseTimer=0,emergencyRestoreTimer=0,emergencyForceRestoreTryTimer=0,emergencyForceRestoreTry=0;
uint16_t UartDisableTimer=UART_DIS_TIME_SEC;


#define FLASH_PAGE_NUMBER                   (16)
#define FLASH_PAGE_SIZE                     (1024)
#define FLASH_PAGE_BASE                     (0x08000000)
#define FLASH_SimulateEEPROM_PAGE_START     (FLASH_PAGE_BASE + FLASH_PAGE_SIZE * (FLASH_PAGE_NUMBER - 1))

/***********************************************************************************************************************
  * @brief
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void FLASH_SimulateEEPROM_ErasePage(uint32_t Address)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(Address);
    FLASH_ClearFlag(FLASH_FLAG_EOP);
    FLASH_Lock();
}

/***********************************************************************************************************************
  * @brief
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void FLASH_SimulateEEPROM_ProgramWord(uint32_t Address, uint32_t Data)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ProgramWord(Address, Data);
    FLASH_ClearFlag(FLASH_FLAG_EOP);
    FLASH_Lock();
}

void boot_data(void)
{
	uint32_t Data = 0;
	Data = *(volatile uint32_t *)FLASH_SimulateEEPROM_PAGE_START;

	if(Data==0xFFFFFFFF)
	{
		Data=DEFAULT_THRESHOLD;
		FLASH_SimulateEEPROM_ProgramWord(FLASH_SimulateEEPROM_PAGE_START, Data);
	}

	threshold = Data;
}

//static uint8_t conter=0;

/***********************************************************************************************************************
  * @brief  This function is main entrance
  * @note   main
  * @param  none
  * @retval none
  *********************************************************************************************************************/
int main(void)
{
    PLATFORM_Init();
	GPIO_Configure();	
	boot_data();
	I2C1_Init();
	VCNL4040_Init();
	PLATFORM_DelayMS(4000);
	USART_Configure(115200);
	printf("TouchSensor Powered ON\n");
	printf("Threshold=%d\n",threshold);
	TIM1_Configure();
	
	last_final_ps = final_ps = VCNL4040_ReadWord(VCNL4040_PS_DATA);
	
    while(1)
    {
		//---------------------------------------------------------------------------------
		if(bool_msec50_flag)
		{
			ps = VCNL4040_ReadWord(VCNL4040_PS_DATA);
			raw_cnt[raw_cnt_ind++] = ps;
			if(raw_cnt_ind>=AVG_SAMPLE) raw_cnt_ind = 0;
			
			raw_cnt_avg=0;
			for(i=0; i<AVG_SAMPLE; i++) 
			{
				raw_cnt_avg += raw_cnt[i];
			}
			final_ps = raw_cnt_avg/AVG_SAMPLE;
			
			if(abs(last_final_ps-final_ps)<10)
			{	
				if(final_ps>threshold)        
				{
					RelayOffCnt=0;
					if(RelayOnCnt<RELAY_CONFIRM_ON_CNT)RelayOnCnt++;
					if(RelayOnCnt>=RELAY_CONFIRM_ON_CNT)
					{
						relayMsgOff=0;
						if(!relayMsgOn)
						{
							RELAY_ON;
							if(UartDisableTimer) printf("RELAY ON\n");
							relayMsgOn=1;
						}
						
						if(emergencyForceRestoreTry!=2)
						{
							emergencyForceRestoreTry=2;
							if(UartDisableTimer) printf("emergencyForceRestoreTry=%d\n",emergencyForceRestoreTry);
						}
					}
				}
				else
				{
					RelayOnCnt=0;
					
					if(emergencyTriggered) 
					{
						if(emergencyForceRestoreTry!=1)
						{
							emergencyForceRestoreTry=1;
							if(UartDisableTimer) printf("emergencyForceRestoreTry=%d\n",emergencyForceRestoreTry);
						}
					}
					else
					{
						emergencySenseTimer=0;
						
						if(RelayOffCnt<RELAY_CONFIRM_OFF_CNT)RelayOffCnt++;
						if(RelayOffCnt>=RELAY_CONFIRM_OFF_CNT)
						{
							relayMsgOn=0;
							if(!relayMsgOff)
							{
								RELAY_OFF;
								if(UartDisableTimer) printf("RELAY OFF\n");
								relayMsgOff=1;
							}
						}
					}
				}
				
				last_final_ps = final_ps;
				test_cnt=0;
			}
			else
			{
				test_cnt++;
				if(test_cnt>2)
				{
					test_cnt=0;
					
					last_final_ps = final_ps;
				}
			}
			
			if(RxTimeout) 
			{
				RxTimeout--;
				if(!RxTimeout)
				{
					memset(RxBuffer,0,sizeof(RxBuffer));
					RxInd=0;
				}
			}
			
			bool_msec50_flag=0;
		}	
		//---------------------------------------------------------------------------------	
		if(bool_sec_flag)
		{
			if(UartDisableTimer) 
			{
				UartDisableTimer--;
				if(!UartDisableTimer)
				{
					printf("UART Disabled\n");
					
					//Disable UART
					USART_Cmd(USART1, DISABLE);
				}
			}
			
			if(relayMsgOn)
			{
				if(!emergencyTriggered)
				{
					if(emergencySenseTimer<EMERGENCY_SENSE_SEC)emergencySenseTimer++;
					if(emergencySenseTimer>=EMERGENCY_SENSE_SEC)
					{
						emergencySenseTimer=0;
						emergencyTriggered=1;
						emergencyRestoreTimer=EMERGENCY_AUTO_RESTORE_SEC;
						
						if(UartDisableTimer) printf("Emergency Triggered\n");
					}
				}
				else
				{
					if(emergencyForceRestoreTry==2)
					{
						emergencyForceRestoreTryTimer++;
						if(emergencyForceRestoreTryTimer>EMERGENCY_FORCE_RESTORE_SEC)
						{
							emergencyForceRestoreTryTimer=0;
							
							emergencyTriggered=0;
							emergencyForceRestoreTry=0;
							if(UartDisableTimer) printf("Emergency Force Restored\n");
							
							RELAY_OFF;
							PLATFORM_DelayMS(1000);
							RELAY_ON;
						}
					}
					else
					{
						emergencyForceRestoreTryTimer=0;
					}
					
					if(emergencyRestoreTimer)
					{
						emergencyRestoreTimer--;
						if(!emergencyRestoreTimer)
						{
							emergencyTriggered=0;
							emergencyForceRestoreTry=0;
							if(UartDisableTimer) printf("Emergency Timeout Restored\n");
							
							RELAY_OFF;
							PLATFORM_DelayMS(1000);
							RELAY_ON;
						}
					}
				}
			}
			
			if(!CAL_PIN_STAT)
			{
				if(CalPressTimer) 
				{	
					CalPressTimer--;
					if(!CalPressTimer)
					{
						//Erase Flash Page
						FLASH_SimulateEEPROM_ErasePage(FLASH_SimulateEEPROM_PAGE_START);
						
						gu32_temp=final_ps;
						threshold=final_ps;
						
						//Write Threshold Data
						FLASH_SimulateEEPROM_ProgramWord(FLASH_SimulateEEPROM_PAGE_START, gu32_temp);
						
						//Reply with Acknowledgement message
						printf("Threshold=%d Saved\n",final_ps);
					}
				}
			}
			else
			{
				CalPressTimer=CALL_PRESS_TIME_SEC;
			}
						
			if(UartDisableTimer) printf("Count=%d\r\n",final_ps);
			
			bool_sec_flag=0;
		}
		//---------------------------------------------------------------------------------
		if((bool_msgRcvOK==1) && (UartDisableTimer))
		{
			if(!memcmp(RxBuffer,"Save\n",sizeof("Save\n")))
			{
				//Erase Flash Page
				FLASH_SimulateEEPROM_ErasePage(FLASH_SimulateEEPROM_PAGE_START);
				
				gu32_temp=final_ps;
				threshold=final_ps;
				
				//Write Threshold Data
				FLASH_SimulateEEPROM_ProgramWord(FLASH_SimulateEEPROM_PAGE_START, gu32_temp);
				
				//Reply with Acknowledgement message
				printf("Threshold=%d Saved\n",final_ps);
			}
			else if(!memcmp(RxBuffer,"Reset\n",sizeof("Reset\n")))
			{
				printf("Sensor Reset\n");
				PLATFORM_DelayMS(5);
				NVIC_SystemReset();
			}
			
			//Clear UART Rx buffer
			memset(RxBuffer,0,RX_IND_MAX);
			RxInd=0;
			bool_msgRcvOK=0;
		}
		//---------------------------------------------------------------------------------
		//PLATFORM_DelayMS(50);
	}
}

/**
  * 
  */

/**
  * @}
  */

/**
  * @}
  */

/********************************************** (C) Copyright MindMotion **********************************************/

