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

char rxData [300];
extern char rxBuf[300];
extern uint16_t rxBufLen;

static uint8_t mqtt_txbuff[200];
static uint8_t mqtt_rxbuff[150];
static struct umqtt_connection mqtt = {
	.txbuff = {
		.start = mqtt_txbuff,
		.length = sizeof(mqtt_txbuff),
	},
	.rxbuff = {
		.start = mqtt_rxbuff,
		.length = sizeof(mqtt_rxbuff),
	},
	//.message_callback = handle_message,
};




int main(void)
{
    uint16_t index=0;
////initialize
    initDelay();
    gpioInit();
    debugInit();
    simInit();

    uint8_t  i;
    debugSend("hello world\n");
    delayMilliIT(1000);
    debugSend("hello world2\n");


    simSend("AT");
    delayMilliIT(30);
    if (rxBufLen>0) debugSend(rxBuf);

    //char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";
    char connString[]="AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"";
    simSend(connString);
    delayMilliIT(100);

    if((strstr(rxBuf, "CONNECT OK") != NULL)||(strstr(rxBuf, "ALREADY CONNECT") != NULL) )
    {
        char txBuf[]=fi;
        int cidlen = strlen("stm32test");
        uint8_t fixed;
        uint8_t remlen[4];
        uint8_t variable[10];
        uint8_t payload[2 + cidlen];

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

        umqtt_circ_push(&conn->txbuff, &fixed, 1);
        umqtt_circ_push(&conn->txbuff, remlen, umqtt_encode_length(sizeof(variable) + sizeof(payload), remlen));
        umqtt_circ_push(&conn->txbuff, variable, sizeof(variable));
        umqtt_circ_push(&conn->txbuff, payload, sizeof(payload));


    }

    return 0;
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
