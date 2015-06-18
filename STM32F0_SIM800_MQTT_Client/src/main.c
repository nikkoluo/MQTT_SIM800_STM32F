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
	 	GPIO_InitTypeDef* GPIO_InitStruct
	 	{
	 		GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	 		GPIO_Mode = GPIO_Mode_AF;
	 		GPIO_Speed = GPIO_Speed_10MHz;
	 		GPIO_OType = GPIO_OType_PP;
	 		GPIO_PuPd = GPIO_PuPd_UP; 
	 	}
	 	GPIO_Init(GPIO_InitStruct);
	 	USART_InitTypeDef* USART_InitStruct
	 	{
	 		USART_BaudRate =  9600;//Add baud rate
	 		USART_WordLength = USART_WordLength_8b;
	 		USART_StopBits = USART_StopBits_1;
	 		USART_Parity = USART_Parity_Even;
	 		USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	 		USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	 	}
	 	USART_Init(USART1, USART_InitStruct);
	 	USART_ITConfig();
	 	USART_Cmd();


}
