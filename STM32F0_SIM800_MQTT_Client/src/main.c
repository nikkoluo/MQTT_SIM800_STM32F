/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include "stm32f0xx_conf.h"

int main(void)
{
  while(1)
  {
  
  }
}

void init_USART(void)
{
	 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
	 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
	 	GPIO_InitTypeDef GPIO_InitStruct
	 	{
	 		GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	 		GPIO_Mode = GPIO_Mode_AF;
	 		GPIO_Speed = GPIO_Speed_10MHz;
	 		GPIO_OType = GPIO_OType_PP;
	 		GPIO_PuPd = GPIO_PuPd_UP; 
	 	}
	 	GPIO_Init(GPIO_InitStruct);
	 	

}
