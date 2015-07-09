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
#include "umqtt.h"
#include "config.h"
#include "Debug.h"
#include "SIM808.h"

#define LINEMAX 50
uint8_t writeflag=0;


int main(void)
{
////initialise
    LCD5110_init();
    LCD5110_clear();

    //init_USART();
    init_DebugUSART();
    init_SIMUSART();

    uint8_t  i;
    DEBUG_Send("hello world");
    while(1)
    {
         if ( Sim808_connect())
        {
             Sim808_receive();
             Sim808_send();
         }
        if(strstr(receivedDebug, "X_conn") != NULL)
        {
            SIM_Connection();
            flushDEBUGBuffer();
        }
        if(strstr(receivedDebug, "X_mqttcon") != NULL)
        {
            DEBUG_Send("hello");
            flushDEBUGBuffer();

        }
        if ((USART_GetFlagStatus(USART2, USART_FLAG_IDLE))&&(receivedStringLen>1))
        {
            //LCD5110_write_string(receivedString);
            Delay(1);
            DEBUG_Send(receivedString);
            writeflag=0;
            for(i=0; i<50; i++) receivedString[i]=0;//flush buffer
            receivedStringLen=0;
        }
    }
    return 0;
}

/*void init_USART(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);//TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);//RX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);//CTS
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);//RTS
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);//TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);//RX
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
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
    USART_Init(USART2, &USART_InitStruct);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}
*/
void Init_SIM(void)
{
    USART_SendString("AT+CSQ");
    Delay(2);
    if(strstr(receivedString, "OK") != NULL) {
        flushReceiveBuffer();
        DEBUG_Send("DONE CSQ--");
        USART_SendString("AT+CGATT?");
        Delay(2);
        while(strstr(receivedString, "1") == NULL)
        {
            flushReceiveBuffer();
            Delay(2);
            USART_SendString("AT+CGATT=1");
        }
        if(strstr(receivedString, "1") != NULL) {
            flushReceiveBuffer();
            DEBUG_Send("DONE ATT--");
            Delay(2);
        }
    }
}


void flushDEBUGBuffer(void)
{
    uint16_t i;
    for(i=0; i<50; i++) receivedDebug[i]=0;//flush buffer
    receivedDebugLen=0;
}
void Delay(__IO uint32_t nCount) //in millisecond
{
	nCount = nCount * 5940 *2;//381;
	while(nCount--)
	{
	}
}


void USART2_IRQHandler (void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE)!= RESET)
    {
        receivedString[receivedStringLen++] = USART_ReceiveData(USART2);
        if(receivedString[receivedStringLen-1]==0x0d) writeflag=1;
    }

}
void USART1_IRQHandler (void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE)!= RESET)
    {
        receivedDebug[receivedDebugLen++] = USART_ReceiveData(USART1);
        if(receivedDebug[receivedDebugLen-1]==0x0d)
        {
            USART_ClearFlag(USART1, USART_FLAG_RXNE);
            DEBUG_Send("check\n");
            //USART_SendString("AT");
            flushDEBUGBuffer();
        }
    }

}


