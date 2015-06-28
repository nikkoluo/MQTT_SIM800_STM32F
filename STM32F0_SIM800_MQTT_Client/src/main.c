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


#define LINEMAX 50
uint8_t writeflag=0;
char receivedString[50];
unsigned char receivedStringLen;
int main(void)
{


////initialise
    LCD5110_init();
    LCD5110_clear();
    LCD5110_write_string("Online:");

    init_USART();
    init_GPIO();

    uint8_t Switch0, Switch1, i;
    char greeting[20];
    for (i=0; i<2; i++)
        greeting[i] = "AT"[i];
    USART_SendString(greeting);
    while(1)
    {
        Switch0 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
        if (Switch0 == 0)
        {
            LCD5110_LCD_delay_ms(200);
            //USART_SendString("ATD0722552972;");
            LCD5110_clear();
            LCD5110_set_XY(0,0);

        }
        Switch1 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
        if (Switch1 == 0)
        {
            LCD5110_LCD_delay_ms(200);
            SIM_Hangup();


        }
        if (writeflag)
        {
            LCD5110_write_string(receivedString);
            writeflag=0;
            for(i=0; i<30; i++) receivedString[i]=0;//flush buffer
            receivedStringLen=0;
        }
    }
}

void init_USART(void)
{

    /*
        USART 1
            TX - PA9
            RX - PA10
        USART 2
            TX - PA2
            RX - PA3
    */
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);
    GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_2|GPIO_Pin_3;
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
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    USART_Init(USART2, &USART_InitStruct);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

void USART_SendString(char StringToSend[], int index)
{
    uint16_t Length = strlen(StringToSend);
    uint16_t i;
    for (i=0; i<Length; i++ )
    {
        USART_SendData(USART2, StringToSend[i]);
        while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    }
    USART_SendData(USART2,0x0D);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
}

void init_GPIO(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}
 void SIM_AT(void)
{
    USART_SendData(USART2, 0x41);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x54);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x0d);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

}
 void SIM_Call(void)
{
//atd0722552972;    41 54 44 30 37 32 35 35 32 39 37 32 3b
    USART_SendData(USART2, 0x41);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x54);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x44);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x30);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x37);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    USART_SendData(USART2, 0x32);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x32);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x35);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

     USART_SendData(USART2, 0x35);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x32);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x39);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x37);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x32);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x3b);

    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x0d);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

}
void SIM_Hangup(void)
{
    USART_SendData(USART2, 0x41);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x54);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x48);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));

    USART_SendData(USART2, 0x0d);
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
}
void Delay(__IO uint32_t nCount) //in millisecond
{
	nCount = nCount * 5940 *2;//381;
	while(nCount--)
	{
	}
}

/*void USART1_IRQHandler (void)
{
  static char rx_buffer[LINEMAX];   // Local holding buffer to build line
  static int rx_index = 0;

  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // Received character?
  {
	char rx =  USART_ReceiveData(USART1);

	if (!USART_GetFlagStatus(USART1, USART_FLAG_RXNE))//(rx == '\r') || (rx == '\n')) // Is this an end-of-line condition, either will suffice?
	{
		//if (rx_index != 0) // Line has some content?
		{
			//USART_SendString(rx_buffer, rx_index);
			strcpy(receivedString , rx_buffer);
			memset(rx_buffer,0,strlen(rx_buffer));
			rx_index = 0;
			writeflag=1;
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

}*/

void USART2_IRQHandler (void)
{
    //static char rx_buffer[LINEMAX];   // Local holding buffer to build line
    //static int rx_index = 0;
    if(USART_GetITStatus(USART2, USART_IT_RXNE)!= RESET)
    {
        receivedString[receivedStringLen++] = USART_ReceiveData(USART2);
        if(receivedString[receivedStringLen-1]==0x0d) writeflag=1;
        //if (rx_index>=19) rx_index=0;
    }

    /*if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // Received character?
    {
        char rx =  USART_ReceiveData(USART2);
        if (!USART_GetFlagStatus(USART2, USART_FLAG_RXNE))//(rx == '\r') || (rx == '\n')) // Is this an end-of-line condition, either will suffice?
        {
            {
                strcpy(receivedString , rx_buffer);
                memset(rx_buffer,0,strlen(rx_buffer));
                rx_index = 0;
                writeflag=1;
                USART_ClearITPendingBit(USART2, USART_IT_RXNE);
            }
        }
        else
        {
            if (rx_index == LINEMAX) // If overflows pull back to start
                rx_index = 0;

              rx_buffer[rx_index++] = rx; // Copy to buffer and increment
        }

    }*/
}
