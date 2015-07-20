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


char rxBuf[1000];
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
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
    USART_Init(USART2, &USART_InitStruct);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
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
    uint8_t ready=0, error=0, transmitSuccess=0;
    sprintf(sendStr, "AT+CIPSEND=%u", length);
    flushReceiveBuffer();
    simSend(sendStr);
    while((!ready)&&(!error))
    {
        delayMilli(50);
        if(strstr(rxBuf, ">") != NULL)//ready to send
        {
            ready=1;
        }
        if(strstr(rxBuf, "ERROR") != NULL)
        {
            error=1;
        }
        debugSend(rxBuf);
        flushReceiveBuffer();
        debugSend("waiting");
    }
    uint16_t i;
    if(!error)
    {
        debugSend("no error");
        for (i=0; i<length; i++)
        {
            USART_SendData(USART2, stringToSend[i]);
            while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
        }
        delayMilli(50);
    }
    else debugSend("error occurred");

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

	umqtt_connect(conn, 120, MQTT_CLIENT_ID, "stm", "123");

}
/** \brief This sends a command to the sim module and waits for the response.
 *
 * \param sendCommand
 * \param checkResponse
 * \param timeout in milliseconds
 * \return
 *
 */
uint8_t simCheckResult(char *sendCommand, char *checkResponse, uint16_t timeout)
{
    uint16_t counter=0;
    flushReceiveBuffer();
    simSend(sendCommand);
    while((rxBufLen==0)&&(counter<timeout))
    {
        delayMilliIT(30);
        counter++;
    }
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
        if(strstr(rxBuf, checkResponse) != NULL) return 1;
        else return 0;
    }
    else return 0;
}


uint8_t simPing(void)
{
    uint16_t counter=0;
    flushReceiveBuffer();
    simSend("AT");
    delayMilliIT(30);
    while((rxBufLen==0)&&(counter<1000))
    {
        delayMilliIT(30);
        counter++;
    }
    if(rxBufLen>0)
    {
        if(strstr(rxBuf, "\r\nOK\r\n") != NULL) return 1;
    }
    else return 0;
}


void simTCPReceive(void)
{
    flushReceiveBuffer();
    simSend("AT+CIPRXGET=2,1000");
    delayMilliIT(50);
    debugSend("-----\n");
    debugSend(rxBuf);
    debugSend("-----\n");

}

uint8_t simWaitSendSuccess(void)
{
    delayMilliIT(100);
    while(rxBufLen==0)delayMilliIT(100);
    debugSend(rxBuf);
    if(strstr(rxBuf, "SEND OK") != NULL) return 1;
    else return 0;
}
