/**< Servo library for STM32F0
Copyright Brandon Piner 2015*/

#include "servo.h"
#include "stm32f0xx_conf.h"
#include <stdio.h>

void servoInit()
{

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);



    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 120;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV4;
    TIM_TimeBaseInitStruct.TIM_Period = 8000;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 900;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM3, &TIM_OCInitStruct);



    TIM_Cmd(TIM3, ENABLE);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM3, ENABLE);
}

void servoSet(uint16_t nsOnTime)
{
    TIM3->CCR1 = nsOnTime*0.4;
}

void servoUp()
{
    servoSet(1795);
}


void servoDown()
{
    servoSet(1360);
}


void servoDrop()
{
    servoSet(1040);
}
