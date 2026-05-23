/***********************************************************************************************************************
    @file    mm32g0001_it.c
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
#define _MM32G0001_IT_C_

/* Files include */
#include "platform.h"
#include "mm32g0001_it.h"
#include "sb_global_var.h"

/**
  * @addtogroup MM32G0001_LibSamples
  * @{
  */

/**
  * @addtogroup Peripheral
  * @{
  */

/**
  * @addtogroup Peripheral_SampleFunction
  * @{
  */

/* Private typedef ****************************************************************************************************/

/* Private define *****************************************************************************************************/

/* Private macro ******************************************************************************************************/

/* Private variables **************************************************************************************************/

/* Private functions **************************************************************************************************/

/***********************************************************************************************************************
  * @brief  This function handles NMI exception
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void NMI_Handler(void)
{
}

/***********************************************************************************************************************
  * @brief  This function handles Hard Fault exception
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/***********************************************************************************************************************
  * @brief  This function handles SVCall exception
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void SVC_Handler(void)
{
}

/***********************************************************************************************************************
  * @brief  This function handles PendSVC exception
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void PendSV_Handler(void)
{
}

/***********************************************************************************************************************
  * @brief  This function handles SysTick Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void SysTick_Handler(void)
{
    if (0 != PLATFORM_DelayTick)
    {
        PLATFORM_DelayTick--;
    }
}

/***********************************************************************************************************************
  * @brief  This function handles TIM1_BRK_UP_TRG_COM Handler
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    if (RESET != TIM_GetITStatus(TIM1, TIM_IT_Update))
    {
		static uint8_t mcnt=0;
		
		bool_msec50_flag=1;
		//---------------------------------------------
		mcnt++;
		if(mcnt>=20)
		{
			mcnt=0;
			bool_sec_flag=1;
		}
		//---------------------------------------------
		
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}

/*****************************************
  * @brief  This function handles USART1 Handler
  * @note   none
  * @param  none
  * @retval none
  ***************************************/
void USART1_IRQHandler(void)
{
    uint8_t RxData = 0;

    if (RESET != USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        RxData = (uint8_t)USART_ReceiveData(USART1);

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		
		if(!bool_msgRcvOK)
		{
			if(RxData=='\n')
			{
				RxBuffer[RxInd++]=RxData;
				RxTimeout=0;
				bool_msgRcvOK = 1;
			}
			else
			{
				RxBuffer[RxInd++]=RxData;
				RxTimeout=4;
			}
			if(RxInd>=RX_IND_MAX) RxInd=0;
		}
    }

//    if (RESET != USART_GetITStatus(USART1, USART_IT_TXIEN))
//    {
//        USART_ClearITPendingBit(USART1, USART_IT_TXIEN);

//        if (0 == USART_TxStruct.CompleteFlag)
//        {
//            USART_SendData(USART1, USART_TxStruct.Buffer[USART_TxStruct.CurrentCount++]);

//            if (USART_TxStruct.CurrentCount == USART_TxStruct.Length)
//            {
//                USART_TxStruct.CompleteFlag = 1;

//                USART_ITConfig(USART1, USART_IT_TXIEN, DISABLE);
//            }
//        }
//    }
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/********************************************** (C) Copyright MindMotion **********************************************/

