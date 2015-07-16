
/**
 *
 * STM UART Debug interface
 * Brandon Piner 2015
 *
 *
 */

#include "Debug.h"
#include "stm32f0xx_conf.h"

char receivedDebug[200];
unsigned char receivedDebugLen;

void debugInit(void)
{

    /*
        USART 1
            TX - PA9
            RX - PA10
    */
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

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

void debugSend(char StringToSend[])
{
    uint16_t Length = strlen(StringToSend);
    uint16_t i;
    for (i=0; i<Length; i++ )
    {
        USART_SendData(USART1, StringToSend[i]);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
    }
}
void debugSend2(char StringToSend[], int len)
{
    uint16_t i;
    for (i=0; i<len; i++ )
    {
        USART_SendData(USART1, StringToSend[i]);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
    }
}
void debugFlushRx(void)
{
    uint16_t i;
    for(i=0; i<50; i++) receivedDebug[i]=0;//flush buffer
    receivedDebugLen=0;
}
void debugReceive()
{

}
void USART1_IRQHandler (void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE)!= RESET)
    {
        receivedDebug[receivedDebugLen++] = USART_ReceiveData(USART1);
    }

}
