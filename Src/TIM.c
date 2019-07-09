/*
  TIM.c - Timer Implementation
  Part of STM32F4_HAL

  Copyright (c)	2017 Patrick F.

  STM32F4_HAL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  STM32F4_HAL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with STM32F4_HAL.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "TIM.h"
#include "misc.h"


/**
 * Timer 1
 * Outputs ~10 KHz on D11
 * Used for Variable Spindle PWM
 **/
void
TIM1_Init(void) {
    TIM_HandleTypeDef TIM_Handle = {0};
    TIM_OC_InitTypeDef TIM_OCInitStructure = {0};

	/* TIM1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* Time base configuration */
    TIM_Handle.Instance = TIM1;
    TIM_Handle.Init.Prescaler = 48;		// 2 MHz
    TIM_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM_Handle.Init.Period = 100-1;		// ~10 KHz
    TIM_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    TIM_Handle.Init.RepetitionCounter = 0;
    TIM_Handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&TIM_Handle);

	/* PWM1 Mode configuration: Channel1 */
    TIM_Handle.Init.OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.Pulse = 0;
    TIM_OCInitStructure.OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.OCNIdleState = TIM_OCIdleState_Set;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}


/**
 * Timer 9
 * Base clock: 24 MHz
 * Used for Stepper Interrupt
 * On CC1, Main Stepper Interuppt is called
 * On Update, Stepper Port Reset is called
 **/
void
TIM9_Init(void) {
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	/* TIM9 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = 4-1;		// 24 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);

	/* Output Compare Toggle Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 0x0FFF;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC1Init(TIM9, &TIM_OCInitStructure);

    // Enable register preload, to ensure all new register values are only updated after an update event
	TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM9, ENABLE);

	/* Enable the TIM9 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM IT enable */
	TIM_ITConfig(TIM9, TIM_IT_CC1 | TIM_IT_Update, ENABLE);

	/* TIM disable counter */
	TIM_Cmd(TIM9, DISABLE);
}
