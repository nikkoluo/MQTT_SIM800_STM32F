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
#include "Delay.h"
#include "umqtt.h"
#include "config.h"


char rxBuf[300];
uint16_t rxBufLen;

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
void simSendRaw(const char* data)
{
    size_t length = strlen(data);
    uint16_t i;
    for (i=0; i<length; i++ )
    {
        USART_SendData(USART2, data[i]);
        while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    }
}

void simTransmit(char * stringToSend, uint16_t length)
{
    char sendStr[20];
    uint8_t ready=0;
    sprintf(sendStr, "AT+CIPSEND=%u", length);
    //debugSend(sendStr);

    flushReceiveBuffer();
    simSend(sendStr);
    while(!ready)
    {
        delayMilli(50);
        if(strstr(rxBuf, ">") != NULL)
        {
            ready=1;
        }
        debugSend(rxBuf);
        debugSend("waiting");
    }
    debugSend("----about to send--\n");
    uint16_t i;
    for (i=0; i<length; i++)
    {
        USART_SendData(USART2, stringToSend[i]);
        while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
    }
}
void simEnterDataMode()
{
    uint8_t wait=1;
    flushReceiveBuffer();
    simSend("ATO");
    while(wait)
    {
        delayMilli(100);
        if(strstr(rxBuf, "CONNECT") != NULL)
        {
            wait=0;
        }
        if(strstr(rxBuf, "ERROR") != NULL)
        {
            flushReceiveBuffer();
            simSend("ATO");
        }
        debugSend(rxBuf);
        debugSend("waiting------\n");
    }
    debugSend("exited----------\n");
    flushReceiveBuffer();
}
void simExitDataMode()
{
    delayMilliIT(1000);
    simSendRaw("+++");
    delayMilliIT(1000);
}
void flushReceiveBuffer()
{
    uint16_t i;
    for (i=0; i<300; i++) rxBuf[i]=0;
    rxBufLen=0;
}

void USART2_IRQHandler (void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE)!= RESET)
    {
        rxBuf[rxBufLen++] = USART_ReceiveData(USART2);
    }

}
void nethandler_umqtt_init(struct umqtt_connection *conn)
{

	umqtt_init(conn);
	umqtt_circ_init(&conn->txbuff);
	umqtt_circ_init(&conn->rxbuff);

	umqtt_connect(conn, 30, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);

}
