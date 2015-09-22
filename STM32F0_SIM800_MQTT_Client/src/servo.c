/**< Servo library for STM32F0
Copyright Brandon Piner 2015*/

#include "servo.h"
#include "stm32f0xx_conf.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void servoSet(uint16_t nsOnTime);
/* Private functions ---------------------------------------------------------*/
void servoInit()
{
    GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin =GPIO_Pin_15;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 120;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV4;
    TIM_TimeBaseInitStruct.TIM_Period = 8000;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 00;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM2, &TIM_OCInitStruct);

    TIM_Cmd(TIM2, ENABLE);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM2, ENABLE);

}
void servo_DeInit()
{
    GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin =GPIO_Pin_15;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
    TIM_Cmd(TIM2, DISABLE);
    TIM_DeInit(TIM2);*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
      /* Set the default configuration */
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFFFFFF;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0x0000;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Inactive;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
  //  TIM_OC1Init(TIM2, &TIM_OCInitStruct);
   // TIM_CtrlPWMOutputs(TIM2, DISABLE);
}
void servoSet(uint16_t nsOnTime)
{
    TIM2->CCR1 = nsOnTime*0.4;
}

void servoUp()
{
    servoSet(1965);
}


void servoDown()
{
    servoSet(1545);
}


void servoDrop()
{
    servoSet(1145);
}

void servoNone()
{
    servoSet(1770);
}
