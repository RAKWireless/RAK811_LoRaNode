/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: MCU RTC timer and low power modes management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <math.h>
#include "board.h"
#include "timer-board.h"

volatile bool TimerIdleSleep = false;
TIM_HandleTypeDef    TimHandle;
/**
  * @brief  This function handles TIM interrupt request.
  * @param  None
  * @retval None
  */
void TIM6_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}

/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef *htim)
{
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* TIMx Peripheral clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();
    
    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set the TIMx priority */
    HAL_NVIC_SetPriority(TIM6_IRQn, 3, 0);
    
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIM6_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    e_printf("haha\r\n");
    TimerIdleSleep = true;

}

void TimerIdleInit(void)
{

    uint32_t uwPrescalerValue = (uint32_t)(SystemCoreClock / 1000) - 1;
    
    
    
    

    /* Set TIMx instance */
    TimHandle.Instance = TIM6;
    
    /* Initialize TIMx peripheral as follows:
    + Period = 10000 - 1
    + Prescaler = (SystemCoreClock/10000) - 1
    + ClockDivision = 0
    + Counter direction = Up
    */
    TimHandle.Init.Period            = 5000 - 1;
    TimHandle.Init.Prescaler         = uwPrescalerValue;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_DOWN;
    
    HAL_TIM_OnePulse_Init(&TimHandle, TIM_OPMODE_SINGLE);
    
    __HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
    /* Enable the TIM Update interrupt */
    __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
}


void TimerIdleStart( void )
{
    HAL_TIM_Base_Start(&TimHandle);
    //__HAL_TIM_ENABLE(&TimHandle);
}

void TimerIdleClear( void )
{
    __HAL_TIM_SET_COUNTER(&TimHandle, 0);
}

void TimerIdleStop( void )
{
    HAL_TIM_Base_Stop(&TimHandle);
    //__HAL_TIM_DISABLE(&TimHandle);
}

