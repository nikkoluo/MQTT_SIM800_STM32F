
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
uint16_t receivedDebugLen;

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
    #if DEBUG
    uint16_t Length = strlen(StringToSend);
    uint16_t i;
    for (i=0; i<Length; i++ )
    {
        USART_SendData(USART1, StringToSend[i]);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
    }
    #endif
}
void debugSend2(char StringToSend[], int len)
{
    #if DEBUG
    uint16_t i;
    for (i=0; i<len; i++ )
    {
        USART_SendData(USART1, StringToSend[i]);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
    }
    #endif
}
void debugFlushRx(void)
{
    uint16_t i;
    for(i=0; i<50; i++) receivedDebug[i]=0;//flush buffer
    receivedDebugLen=0;
}
char debugReceive()
{
    if(receivedDebugLen>0)
    {
        delayMilliIT(1);
        return 1;
    }
    else
    {
        return 0;
    }
}
void USART1_IRQHandler (void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE)!= RESET)
    {
        receivedDebug[receivedDebugLen++] = USART_ReceiveData(USART1);
    }

}

/** \brief debug printf to serial of a 16 bit unsigned int
 *
 * \param text to send before the value
 * \param the 16 bit unsigned value
 *
 */
void _printfU(char* text ,uint16_t x)
{
    #if DEBUG
    char debugString[100]="";
    sprintf(debugString, "%s %u\n",text, x);
    debugSend(debugString);
    #endif
}

/** \brief debug printf to serial of a 16 bit signed int
 *
 * \param text to send before the value
 * \param the 16 bit signed value
 *
 */
void _printfS(char* text ,int16_t x)
{
    #if DEBUG
    char debugString[100]="";
    sprintf(debugString, "%s %d\n",text, x);
    debugSend(debugString);
    #endif
}


/** \brief debug printf to serial of a 32 bit unsigned int
 *
 * \param text to send before the value
 * \param the 16 bit unsigned value
 *
 */
void _printfLngU(char* text ,uint32_t x)
{
    #if DEBUG
    char debugString[100]="";
    sprintf(debugString, "%s %u\n",text, x);
    debugSend(debugString);
    #endif
}

/** \brief debug printf to serial of a 32 bit signed int
 *
 * \param text to send before the value
 * \param the 16 bit signed value
 *
 */
void _printfLngS(char* text ,int32_t x)
{
    #if DEBUG
    char debugString[100]="";
    sprintf(debugString, "%s %d\n",text, x);
    debugSend(debugString);
    #endif
}
