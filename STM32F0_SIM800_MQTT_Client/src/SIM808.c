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




char rxBuf[300];
uint16_t rxBufLen ,marker;

void simInit(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitTypeDef USART_InitStruct;
        USART_InitStruct.USART_BaudRate =  115200;
        USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        USART_InitStruct.USART_StopBits = USART_StopBits_1;
        USART_InitStruct.USART_Parity = USART_Parity_No;
        USART_InitStruct.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART_InitStruct);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

int simConnect()
{
    unsigned char timeoutCount=0;
    char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";

    while(timeoutCount<5)
    {
        flushReceiveBuffer();
        USART_SendString(connString);
        Delay(100);
        DEBUG_Send(rxBuf);
        if((strstr(rxBuf, "CONNECT OK") != NULL)||(strstr(rxBuf, "ALREADY CONNECT") != NULL) ){
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
        if(strstr(rxBuf, "CONNECT OK") != NULL) {
            timeoutCount=10;
        }
        else timeoutCount++;
    }
    return 1;
}

void simSend(const char* data)
{
    size_t length = strlen(data);
    uint16_t i;
    for (i=0; i<length; i++ )
    {
        USART_SendData(USART2, data[i]);
        while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    }
    USART_SendData(USART2,0x0D);//end the message
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
}

void flushReceiveBuffer()
{
    uint16_t i;
    for (i=0; i<300; i++) rxBuf[i]=0;
    rxBufLen=0;
    marker =0;
}

void USART2_IRQHandler (void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE)!= RESET)
    {
        rxBuf[rxBufLen++] = USART_ReceiveData(USART2);
    }

}
