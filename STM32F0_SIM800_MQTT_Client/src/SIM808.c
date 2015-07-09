/**
 *
 * Driver for SIM808 network
 * Brandon Piner 2015
 *
 *
 */

#include "SIM808.h"
#include "stm32f0xx_conf.h"
#include <string.h>
char receivedString[200];
unsigned char receivedStringLen;

void Sim808_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);//CTS
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);//RTS
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);//TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);//RX
    GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    USART_InitTypeDef USART_InitStruct;
        USART_InitStruct.USART_BaudRate =  115200;
        USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        USART_InitStruct.USART_StopBits = USART_StopBits_1;
        USART_InitStruct.USART_Parity = USART_Parity_No;
        USART_InitStruct.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
    USART_Init(USART2, &USART_InitStruct);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

int Sim808_connect()
{
    unsigned char timeoutCount=0;
    char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";

    while(timeoutCount<5)
    {
        flushReceiveBuffer();
        USART_SendString("AT+CIPSHUT");
        Delay(15);
        DEBUG_Send(receivedString);
        if(strstr(receivedString, "SHUT OK") != NULL) {
            timeoutCount=10;
        }
        else timeoutCount++;
    }
    timeoutCount=0;
    while(timeoutCount<5)
    {
        flushReceiveBuffer();
        USART_SendString(connString);
        Delay(100);
        DEBUG_Send(receivedString);
        if((strstr(receivedString, "CONNECT OK") != NULL)||(strstr(receivedString, "ALREADY CONNECT") != NULL) ){
            timeoutCount=10;
        }
        else timeoutCount++;
    }
    timeoutCount=0;
    while(timeoutCount<5)
    {
        flushReceiveBuffer();
        USART_SendString("AT+CIPSTATUS");
        Delay(300);
        if(strstr(receivedString, "CONNECT OK") != NULL) {
            timeoutCount=10;
        }
        else timeoutCount++;
    }
    return 1;
}
int Sim808_send(const uint8_t* data, size_t length)
{
    uint16_t i;
    for (i=0; i<length; i++ )
    {
        USART_SendData(USART2, data[i]);
        while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    }
    USART_SendData(USART2,0x0D);//end the message
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    return 1;
}
int Sim808_receive(const uint8_t* data, size_t length )
{

    return 1;
}

