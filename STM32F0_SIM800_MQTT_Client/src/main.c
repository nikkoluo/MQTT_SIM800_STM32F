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
#include "Delay.h"

#define LINEMAX 50

#define umqtt_build_header(type, dup, qos, retain) \
	(((type) << 4) | ((dup) << 3) | ((qos) << 1) | (retain))


char rxData [300];
extern char rxBuf[300];
extern uint16_t rxBufLen;

typedef enum  {
	STATE_INITIAL,
	STATE_START,
	STATE_CONFIG,
	STATE_GPRSACT,
	STATE_STATUS,
	STATE_CONNECTING,
	STATE_CONNECTED,
	STATE_CLOSING,
	STATE_CLOSED,
	STATE_PDPDEACT,
} tcp_state;


int main(void)
{
    tcp_state current_state = STATE_INITIAL;
    uint16_t index=0;
    uint8_t flag_inital=0, flag_start=0, flag_config=0, flag_Connected=0 ;
////initialize
    initDelay();
    gpioInit();
    debugInit();
    simInit();

    debugSend("begin");
char connString[]="AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"";


    simSend("AT");
    delayMilliIT(10);
    if(rxBufLen>0) debugSend(rxBuf);
    flushReceiveBuffer();

    flushReceiveBuffer();
    simSend("AT+CIPMUX=0");
    delayMilliIT(300);
    debugSend(rxBuf);

    flushReceiveBuffer();
    simSend("AT+CIPMODE=0");
    delayMilliIT(300);
    debugSend(rxBuf);

    flushReceiveBuffer();
    simSend("AT+CIPSHUT");
    delayMilliIT(300);
    debugSend(rxBuf);
//Start the flow diagram
    while (flag_Connected==0)
    {
        flushReceiveBuffer();
        simSend("AT+CIPSTATUS");
        delayMilliIT(300);
        debugSend(rxBuf);
        debugSend("--only here--");
        if(strstr(rxBuf, "INITIAL") != NULL) current_state = STATE_INITIAL;
        else if((strstr(rxBuf, "START") != NULL)) current_state = STATE_START;
        else if((strstr(rxBuf, "CONFIG") != NULL)) current_state = STATE_CONFIG;
        else if((strstr(rxBuf, "GPRSACT") != NULL)) current_state = STATE_GPRSACT;
        else if((strstr(rxBuf, "IP STATUS") != NULL)) current_state = STATE_STATUS;
        else if((strstr(rxBuf, "TCP CONNECTING") != NULL)) current_state = STATE_CONNECTING;
        else if((strstr(rxBuf, "CONNECT OK") != NULL)||(strstr(rxBuf, "ALREADY CONNECT") != NULL)) current_state = STATE_CONNECTED;
        else if((strstr(rxBuf, "CLOSING") != NULL)) current_state = STATE_CLOSING;
        else if((strstr(rxBuf, "CLOSED") != NULL)) current_state = STATE_CLOSED;
        else if((strstr(rxBuf, "PDP") != NULL)) current_state = STATE_PDPDEACT;

        switch(current_state)
        {
            case STATE_INITIAL:
                debugSend("--initial--\n");
                flushReceiveBuffer();
                simSend("AT+CSTT=\"internet\"");
                delayMilliIT(30);
                debugSend(rxBuf);
                break;
            case STATE_START:
                debugSend("--start--");
                flushReceiveBuffer();
                simSend("AT+CIICR");
                debugSend("--sent--\n");
                delayMilliIT(30);
                debugSend(rxBuf);
                break;
            case STATE_CONFIG:
                debugSend("--config--");
                debugSend("CONFIG");
                break;
            case STATE_GPRSACT:
                flushReceiveBuffer();
                simSend("AT+CIFSR");
                delayMilliIT(30);
                debugSend(rxBuf);
                break;
            case STATE_STATUS:
                flushReceiveBuffer();
                debugSend("\n--statusing--\n");
                simSend(connString);
                delayMilliIT(30);
                debugSend(rxBuf);
                break;
            case STATE_CONNECTING:
                debugSend("\n--connecting--\n");
                break;
            case STATE_CONNECTED:
                debugSend("--CONNECTED!!");
                flag_Connected=1;
                break;
            case STATE_CLOSING:
                debugSend("closing connection--\n");
                break;
            case STATE_CLOSED:
                flushReceiveBuffer();
                simSend("AT+CIPSHUT");
                delayMilliIT(30);
                debugSend(rxBuf);
                flag_Connected=0;
                break;
            case STATE_PDPDEACT:
                debugSend("kak man..\n");
                flushReceiveBuffer();
                simSend("AT+CIPSHUT");
                delayMilliIT(30);
                debugSend(rxBuf);
                flag_Connected=0;
                break;
        }
    }


    uint8_t  i;

    //char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";
//    char connString[]="AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"";
   // simSend(connString);
    delayMilliIT(4000);
    debugSend(rxBuf);
    flushReceiveBuffer();

    //if((strstr(rxBuf, "CONNECT OK") != NULL)||(strstr(rxBuf, "ALREADY CONNECT") != NULL) )
    {
        //flag_Connected=1;

        char txBuf[40], str[30];
        char cid[] ="stm32test";
        int cidlen = strlen(cid);
        uint8_t fixed;
        uint8_t remlen[4];
        uint8_t variable[10];
        uint8_t payload[2 + cidlen];
        int i=0;
        for (;i<40;i++) txBuf[i]=0;

        fixed = umqtt_build_header(UMQTT_CONNECT, 0, 0, 0);

        variable[0] = 0; /* UTF Protocol name */
        variable[1] = 4;
        variable[2] = 'M';
        variable[3] = 'Q';
        variable[4] = 'T';
        variable[5] = 'T';
        variable[6] = 4;
        variable[7] = 0b00000010; /* Clean session flag */
        variable[8] = 0; /* Keep Alive timer MSB */
        variable[9] = 60;/* Keep Alive timer LSB*/

        payload[0] = cidlen >> 8;
        payload[1] = cidlen & 0xff;
        memcpy(&payload[2], cid, cidlen);

        txBuf[0]=fixed;
        int head=0;
        int encLen=umqtt_encode_length(sizeof(variable) + sizeof(payload), remlen);
        for(i=0; i<encLen; i++)
        {
            txBuf[i+1]=remlen[i];
        }
        head=i+1;
        //txBuf[1]=remlen[0];
        for(i=head; i<(sizeof(variable)+head); i++)
        {
            txBuf[i]=variable[i-head];
        }
        head=i+1;
        for(i=head; i<(sizeof(payload)+head); i++)
        {
            txBuf[i]=payload[i-head];
        }
        head=i;
        sprintf(str, "%d", umqtt_encode_length(sizeof(variable) + sizeof(payload), remlen));
        debugSend2(txBuf,head);

    }

    while(flag_Connected)
    {
        delayMilliIT(2000);
        if(rxBufLen>0)
        {
            debugSend(rxBuf);
            flushReceiveBuffer();
        }
        simSend("AT+CIPSTATUS");
        delayMilliIT(10);
        debugSend(rxBuf);
        flushReceiveBuffer();
        if((strstr(rxBuf, "CONNECT OK") != NULL))
        {
            flag_Connected=1;
        }
        else flag_Connected=0;
    }
    debugSend("-exiting-");
}

void gpioInit()
{
    //USART 1 and 2
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);//CTS2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);//RTS2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);//TX2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);//RX2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);//TX1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);//RX1
    GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_9|GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);


}
int umqtt_encode_length(int len, uint8_t *data)
{
	int digit;
	int i = 0;

	do {
		digit = len % 128;
		len /= 128;
		if (len > 0)
			digit |= 0x80;
		data[i] = digit;
		i++;
	} while (len);

	return i; /* Return the amount of bytes used */
}

