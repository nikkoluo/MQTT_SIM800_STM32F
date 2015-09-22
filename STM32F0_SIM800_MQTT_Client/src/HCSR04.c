/**
  ******************************************************************************
  * @file    HCSR04.c
  * @author  Brandon Piner
  * @version V1.0
  * @date    21-September-2015
  * @brief   This file provides firmware functions to use a HCSR04
  *          ultrasonic distance sensor
  */


#include "HCSR04.h"
#include "Debug.h"
uint32_t Distance=0;
void HCSR04_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
    /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* TIM2_CH1 pin (PA.00) and TIM2_CH2 pin (PA.01) configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  TIM_ICInitTypeDef  TIM_ICInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* TIM2 chennel2 configuration : PA.01 */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect TIM pin to AF2 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_2);

    /* Enable the TIM2 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

 /* ---------------------------------------------------------------------------
    TIM2 configuration: PWM Input mode
     The external signal is connected to TIM2 CH2 pin (PA.01)
     TIM2 CCR2 is used to compute the frequency value
     TIM2 CCR1 is used to compute the duty cycle value

    In this example TIM2 input clock (TIM2CLK) is set to APB1 clock (PCLK1), since
    APB1 prescaler is set to 1.
      TIM2CLK = PCLK1 = HCLK = SystemCoreClock

    External Signal Frequency = SystemCoreClock / TIM2_CCR2 in Hz.
    External Signal DutyCycle = (TIM2_CCR1*100)/(TIM2_CCR2) in %.
  Note:
  SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f0xx.c file.
  Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
  function to update SystemCoreClock variable value. Otherwise, any configuration
  based on this variable will be incorrect.
  --------------------------------------------------------------------------- */

  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);

  /* Select the TIM2 Input Trigger: TI2FP2 */
  TIM_SelectInputTrigger(TIM2, TIM_TS_TI2FP2);

  /* Select the slave Mode: Reset Mode */
  TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM2,TIM_MasterSlaveMode_Enable);

  /* TIM enable counter */
  TIM_Cmd(TIM2, ENABLE);

  /* Enable the CC2 Interrupt Request */
   TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
}

void HCSR04_DeInit()
{
    TIM_Cmd(TIM2, DISABLE);
}


/** \brief Sends 10us pulse on trigger pin
 *
 * \param
 * \param
 * \return
 *
 */
void HCSR04_Read(HCSR04_t * HCSR04)
{
    uint32_t i;
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
    for(i=0; i<40; i++);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
    HCSR04->Distance = Distance;
}


void TIM2_IRQHandler(void)
{

    if(TIM_GetITStatus(TIM2, TIM_IT_CC1)!=RESET)
    {
        Distance = TIM_GetCapture1(TIM2);
        //_printfLngU("D:", Distance);

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
    }
}
