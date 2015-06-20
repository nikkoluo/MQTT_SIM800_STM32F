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
#include <stdio.h>
#include <string.h>


#define LINEMAX 20
uint8_t writeflag=0;
char receivedString[20];
int main(void)
{


////initialise
    LCD5110_init();
    LCD5110_clear();
    LCD5110_write_string("Online:");

    init_USART();
    char greeting[] = "Online:";
    USART_SendString(greeting,7);
    while(1)
    {
        if (writeflag==1)
        {

            writeflag=0;
        }
    }
}

void init_USART(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
	 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
	 	GPIO_InitTypeDef GPIO_InitStruct;
	 		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	 		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	 		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	 		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	 		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	 	GPIO_Init(GPIOA, &GPIO_InitStruct);
	 	USART_InitTypeDef USART_InitStruct;
	 		USART_InitStruct.USART_BaudRate =  115200;//Add baud rate
	 		USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	 		USART_InitStruct.USART_StopBits = USART_StopBits_1;
	 		USART_InitStruct.USART_Parity = USART_Parity_No;
	 		USART_InitStruct.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	 		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	 	USART_Init(USART1, &USART_InitStruct);
	 	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	 	USART_Cmd(USART1, ENABLE);


}

void USART_SendString(char StringToSend[], int index)
{
    uint16_t Length = strlen(StringToSend);
    uint16_t i;
    for (i=0; i<index; i++ )
    {
        USART_SendData(USART1, StringToSend[i]);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
    }
    USART_SendData(USART1,0xA);
    while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
}
void USART_ReceiveString(void)
{
    uint16_t receivedByte=0;
    uint8_t j=0;
    for (j=0; j<20; j++)
    {
        receivedString[j]="";
    }
    uint16_t i=0;
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
    {
        receivedByte = USART_ReceiveData(USART1);
        receivedString[i]=receivedByte;
        i++;
    }
    receivedString[i]='\0';
//must stop this function from always running
    //LCD5110_write_string(receivedString);
}
void USART1_IRQHandler (void)
{
  static char rx_buffer[LINEMAX];   // Local holding buffer to build line
  static int rx_index = 0;

  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // Received character?
  {
	char rx =  USART_ReceiveData(USART1);

	if ((rx == '\r') || (rx == '\n')) // Is this an end-of-line condition, either will suffice?
	{
		//if (rx_index != 0) // Line has some content?
		{
			USART_SendString(rx_buffer, rx_index);
			rx_index = 0;
			USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		}
	}
	else
	{
		if (rx_index == LINEMAX) // If overflows pull back to start
			rx_index = 0;

		  rx_buffer[rx_index++] = rx; // Copy to buffer and increment
	}
  }

}
