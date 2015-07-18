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


uint8_t mqtt_txbuff[200];
uint8_t mqtt_rxbuff[150];

struct umqtt_connection mqtt = {
	.txbuff = {
		.start = mqtt_txbuff,
		.length = sizeof(mqtt_txbuff),
	},
	.rxbuff = {
		.start = mqtt_rxbuff,
		.length = sizeof(mqtt_rxbuff),
	},
//	.message_callback = handle_message,
};


char rxData [300];
extern char rxBuf[300];
extern uint16_t rxBufLen;



typedef enum  {
    STATE_OFF,
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
//set to off if you dont want to activate the sim808
tcp_state current_state = STATE_INITIAL;

int main(void)
{
    uint16_t index=0;
////initialize
    initDelay();
    gpioInit();
    debugInit();
    simInit();

    debugSend("begin\n");


    simSend("AT");
    delayMilliIT(10);
    if(rxBufLen>0) debugSend(rxBuf);
    flushReceiveBuffer();

    simUpdateState();
    if(current_state!=STATE_CONNECTED) simConnect1();

    delayMilliIT(2000);


    nethandler_umqtt_init(&mqtt);

    uint8_t counter=0, sendCount=0, strtosend[20], alive=1;
    while(alive==1)
    {
        counter++;
        delayMilliIT(2000);
        if(rxBufLen>0)//receive any unexpected data
        {
            debugSend("\n\n");
            debugSend(rxBuf);
            debugSend("\n\n");
            flushReceiveBuffer();
        }
        debugSend("alive\n");
        if(counter==5)//wait 10 sec then send connect packet
        {
            debugSend2(mqtt_txbuff,mqtt.txbuff.datalen);
            simTransmit(mqtt_txbuff,mqtt.txbuff.datalen);
            debugSend("-Sent-");
        }
        if(counter==8)//wait 100 sec then exit
        {
            if(sendCount>50) sendCount=0;
            counter=6;
            mqtt.txbuff.pointer= mqtt.txbuff.start;
            mqtt.txbuff.datalen=0;
            sprintf(strtosend, "%d", sendCount++);
            umqtt_publish(&mqtt, "gps/test", strtosend, 2);
            simTransmit(mqtt_txbuff,mqtt.txbuff.datalen);
            debugSend("-Sent-");
        }
        if(sendCount>=5)
        {
            alive=0;//kill
        }
        //simUpdateState();
        //if(current_state!=STATE_CONNECTED) simConnect1();
    }
    simSend("AT+CIPSHUT");
    simSend("AT+CIPSHUT");
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

void simConnect1()
{
   //char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";
    char connString[]="AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"";
    int flag_Connected=0;
    if(current_state!=STATE_OFF)
    {
        flushReceiveBuffer();
        simSend("AT+CIPMUX=0");
        delayMilliIT(300);
        debugSend(rxBuf);

        flushReceiveBuffer();
        simSend("AT+CIPMODE=1");//1 for transparent mode 0 for non-transparent (normal)
        delayMilliIT(300);
        debugSend(rxBuf);

        flushReceiveBuffer();
        simSend("AT+CIPSHUT");
        delayMilliIT(300);
        debugSend(rxBuf);
        //Start the flow diagram
        while (current_state!=STATE_CONNECTED)
        {
            simUpdateState();
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
    }

}


void simUpdateState()
{
    if(current_state!=STATE_OFF)
    {
        flushReceiveBuffer();
        simSend("AT+CIPSTATUS");
        delayMilliIT(100);
        debugSend(rxBuf);
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

    }

}
