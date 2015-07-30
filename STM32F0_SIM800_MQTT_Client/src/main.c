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
#include "servo.h"

#define LINEMAX 50

#define umqtt_build_header(type, dup, qos, retain) \
	(((type) << 4) | ((dup) << 3) | ((qos) << 1) | (retain))


char *connackAccept = {0x20, 0x02, 0,0};
char *subscribeAccept={0x90, 0x03};
extern uint8_t msgTimout;
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
extern char receivedDebug[200];

//set to off if you dont want to activate the sim808
tcp_state current_state = STATE_OFF;
int main(void)
{
    uint8_t flagNetReg=0, flagAlive=0, strtosend[20];
    uint16_t index=0;
    double lng, lat;
////initialize
    initDelay();
    gpioInit();
    debugInit();
    simInit();

    debugSend("\nbegin\n");
    delayMilliIT(500);

    checkInitalStatus();


    ///check what state the TCP is in.
    whatStateAmIIn();
    if(current_state!= STATE_CONNECTED)
        simConnect();
    ///Authenticate with the MQTT broker
    nethandler_umqtt_init(&mqtt);
    simTransmit(mqtt_txbuff,mqtt.txbuff.datalen);
    recievePacket();
    if(strstr(rxBuf, connackAccept) != NULL)
        debugSend("MQTT Connection accepted");
    else
    {
        debugSend("MQTT Connection refused");
        NVIC_SystemReset();
    }

    servoInit();

    ///Subscribe to "test/action" topic
    mqtt.txbuff.pointer= mqtt.txbuff.start;
    mqtt.txbuff.datalen=0;
    umqtt_subscribe(&mqtt, "test/action");
    simTransmit(mqtt_txbuff,mqtt.txbuff.datalen);
    recievePacket();
    uint8_t counter=0;

    while(1)
    {
        delayMilliIT(100);//why? i dont know..

        counter++;
        if(counter>=200) counter=0;

        ///PING "AT"
        if(simPing()==0)
        {///IF NO PING RESPONSE THEN RESET
            debugSend("ping-NO-response\n");
            NVIC_SystemReset();
        }
        current_state=simUpdateState();
        if(current_state!=STATE_CONNECTED)
        {
            debugSend("connection no longer alive\n");
            NVIC_SystemReset();
        }
        ///Receive any subscribed packets
        recievePacket();
        ///Get GPS Coords


        ///MQTT Tranmit packet
        if(counter%199==0)
        {
            debugSend("\n-transmit-");
            mqtt.txbuff.pointer= mqtt.txbuff.start;
            mqtt.txbuff.datalen=0;
            //sprintf(strtosend, "%d", counter);
            parseData(strtosend, "latcoord", "lngcoor", "placehold", "placehold");
            umqtt_publish(&mqtt, "test/gps", strtosend, strlen(strtosend));
            simTransmit(mqtt_txbuff,mqtt.txbuff.datalen);
        }
    }
}

void gpioInit()
{
    //USART 1 and 2
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);//CTS2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);//RTS2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);//TX2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);//RX2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_1); // TIM3_CH1

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);//TX1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);//RX1
    GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_6|GPIO_Pin_9|GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);


}


void simConnect()
{
    char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";
    //char connString[]="AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"";
    if(simMUX()==0)
    {
        NVIC_SystemReset();
    }
    debugSend(rxBuf);
    delayMilliIT(10);

    if(simAPN()==0)
    {
        NVIC_SystemReset();
    }
    debugSend(rxBuf);
    while(current_state!=STATE_START)
    {
        delayMilliIT(50);
        current_state=simUpdateState();
        whatStateAmIIn();
    }
    debugSend("about to start wireless\n");

    flushReceiveBuffer();
    simSend("AT+CIICR");
    while(simAvailable()==0);

    if(strstr(rxBuf, "OK") != NULL)
    {
        debugSend("successful CIICR");
    }
    else NVIC_SystemReset();

    while(current_state != STATE_GPRSACT)
    {
        current_state = simUpdateState();
        whatStateAmIIn();
    }

    if(simCheckResult("AT+CIFSR", "ERROR", 5000000)==1) NVIC_SystemReset();
    debugSend("here is my IP: \n");
    debugSend(rxBuf);


///Connect to the MQTT server
    flushReceiveBuffer();
    simSend(connString);
    while(simAvailable()==0);//read the response
    if(strstr(rxBuf, "OK") != NULL)
    {
        debugSend(rxBuf);
        flushReceiveBuffer();
        while(simAvailable()==0);
    }
    if(strstr(rxBuf, "CONNECT FAIL") != NULL)
    {
        debugSend("unsuccessful connection\n");
        NVIC_SystemReset();
    }
    debugSend("Connect successful!!\n");
    debugSend(rxBuf);

}

void whatStateAmIIn()
{
    current_state=simUpdateState();
    switch(current_state)
    {
        case STATE_ON:
            debugSend("--on--\n");
            break;
        case STATE_INITIAL:
            debugSend("--initial--\n");
            break;
        case STATE_START:
            debugSend("--start--\n");
            break;
        case STATE_CONFIG:
            debugSend("--config--\n");
            break;
        case STATE_GPRSACT:
            debugSend("\n--GPRS act--\n");
            break;
        case STATE_STATUS:
            debugSend("\n--statusing--\n");
            break;
        case STATE_CONNECTING:
            debugSend("\n--connecting--\n");
            break;
        case STATE_CONNECTED:
            debugSend("\n--connected--\n");
            break;
        case STATE_CLOSING:
            debugSend("closing connection--\n");
            break;
        case STATE_CLOSED:
            debugSend("\n--connecting--\n");
            break;
        case STATE_PDPDEACT:
            debugSend("PDP Deact\n");
            break;
    }
}

void checkInitalStatus(void)
{

///CHECK STATUS OF SIM
    if(simPing())
    {///IF SUCCCESS THEN TEST NETWORK REGISTRATION
        debugSend("ping-resp");
        current_state = STATE_ON;
    }
    else
    {///IF NO PING RESPONSE THEN ??
        debugSend("ping-NO-resp\n");
        NVIC_SystemReset();
    }
    if(simNoEcho()==0)
    {
        ///if the sim is not registered then Reboot microcontroller
        debugSend("Echo not disabled\n");
        debugSend(rxBuf);
        NVIC_SystemReset();
    }

    ///IF STATUS IS FINE THEN INITIALISE THE SIM
    if(simNetReg()==0)
    {
        ///if the sim is not registered then Reboot microcontroller
        debugSend("Not registered so rebooting\n");
        debugSend(rxBuf);
        NVIC_SystemReset();
    }
    ///the device is registered so check if GPRS is attached.
    debugSend("device is registered\n");
    if(simGPRSAttached()==0)
    {
        ///if GPRS is NOT attached then reboot microcontroller
        debugSend("GPRS not attached - rebooting\n");
        simSend("AT+CGATT=1");///Try attach the GPRS service
        delayMilliIT(100);
        debugSend(rxBuf);
        delayMilliIT(100);
        NVIC_SystemReset();
    }
    debugSend("GPRS is attached\n");

    if(simResetIPSession()==0)
    {
        ///Not able to reset the IP Session
        NVIC_SystemReset();
    }
    debugSend("--IP session reset--");


    ///Disable the auto send packet
    flushReceiveBuffer();
    simSend("AT+CIPRXGET=1");
    while(simAvailable()==0);
    if(strstr(rxBuf, "OK") != NULL)
    {
        debugSend("successful manual TCP get\n");

    }
    else
    {
        debugSend(rxBuf);
        NVIC_SystemReset();

    }
}

void recievePacket(void)
{
    uint8_t rxPacketLen=1, i, commaCount=0, commaPos[10], temp[2], mqttPacket[100];
    ///Receive TCP RX
    flushReceiveBuffer();
    simSend("AT+CIPRXGET=2,100");
    while(simAvailable()==0);


    if(rxBuf[16]==0x2C)
    {
        //One digit length
        rxPacketLen = (rxBuf[15])&0x0F;
    }
    else if (rxBuf[17]==0x2C)
    {
        //Two digits long
        rxPacketLen = (rxBuf[16])&0x0F;//ones position
        rxPacketLen += 10*((rxBuf[15])&0x0F);// tens position
    }
    else
    {
        temp[0]=0;
        temp[1]=0;
        rxPacketLen=0;
    }

    memcpy(&mqttPacket[0], &rxBuf[21], rxPacketLen);
    if(rxPacketLen>0)
    {
        debugSend("Packet: ");
        debugSend2(mqttPacket,rxPacketLen);
        USART_SendData(USART1, rxPacketLen);
        debugSend("\n");
        if(mqttPacket[1]==0X11)
        {
            debugSend("- - - going places\n");
        }
        if(mqttPacket[18]=='X')
        {
            servoUp();
            debugSend("- - - going up\n");
        }
        if(mqttPacket[18]=='N')
        {
            debugSend("- - - going down");
            servoDown();

        }
        if(mqttPacket[18]=='P')
        {
            servoDrop();
            debugSend("- - - going drop");
        }
    }




}


void parseData(char *payload, char *latitude , char *longitude, char *altitude, char *battery)
{

    sprintf(payload, "{\"lat\":%d,\"lng\":%d, \"alt\":%d, \"bat\":%d}", -33-rand()%3, 18+rand()%3, 1000+rand()%1000, 80+rand()%30);
}
