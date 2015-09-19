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
#include "timing.h"
#include "umqtt.h"
#include "config.h"


char rxBuf[300];
uint16_t rxBufLen =0;
extern uint8_t msgTimout;

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
    uint16_t i;
    sprintf(sendStr, "AT+CIPSEND=%u", length);
    flushReceiveBuffer();
    simSend(sendStr);
    while(simAvailable()==0);//read the response
    if(strstr(rxBuf, ">") != NULL)//ready to send
    {
        flushReceiveBuffer();
        for (i=0; i<length; i++)
        {
            USART_SendData(USART2, stringToSend[i]);
            while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
        }
        while(simAvailable()==0);//read the response
        if(strstr(rxBuf, "SEND OK") != NULL)
        {
            debugSend(" acknowledge\n");
        }
    }
    if(strstr(rxBuf, "ERROR") != NULL)
    {
        debugSend(" send acknowledge not received");
        NVIC_SystemReset();
    }


}

/*
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
}*/


void flushReceiveBuffer()
{
    uint16_t i;
    for (i=0; i<300; i++) rxBuf[i]=0;
    rxBufLen=0;
}

uint8_t simAvailable(void)
{
    if((rxBufLen>0)&&(msgTimout>=5))
    {
        return 1;
    }
    else
    {
        return 0;
    }


}

void USART2_IRQHandler (void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE)!= RESET)
    {
        rxBuf[rxBufLen++] = USART_ReceiveData(USART2);
        msgTimout=0;
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
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend(sendCommand);
    while(simAvailable()==0)
    {
        counter++;
        if (counter > timeout)
        {
            simSendRaw('A');
            return 0;
        }
    }

    if(rxBufLen>0)
    {
        if(strstr(rxBuf, checkResponse) != NULL) return 1;
        else return 2;//indicates an error or unexpected result
    }
}

uint8_t simNetReg(){

    if((simCheckResult("AT+CREG?", "0,1", 500000)==1)||(simCheckResult("AT+CREG?", "0,5", 500000)==1)) return 1;
    else return 0;
}
uint8_t simGPRSAttached()
{
    if(simCheckResult("AT+CGATT?", "1", 500000)==1) return 1;
    else return 0;
}

void simPinCheck()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CPIN?");
    while(simAvailable()==0)
    {
        counter++;
        if (counter > 500000)
        {
            simSendRaw('A');
            return 0;
        }
    }
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}

void simBatteryCheck(struct sim808_t * sim808)
{
    char ptrBat;
    char Batt[3];
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CBC");
    while(simAvailable()==0)
    {
        counter++;
        if (counter > 500000)
        {
            simSendRaw('A');
            return 0;
        }
    }
    if(rxBufLen>0)
    {
        //debugSend(rxBuf);
        /**

        +CBC: 0,75,4004

        OK
        */
        memcpy(&Batt[0], &rxBuf[10], 2);//pointer to the battery percentage
        debugSend2(Batt, 2);
        sim808->battery = 10*(Batt[0] - '0') + Batt[1] - '0';
        USART_SendData(USART1,  sim808->battery);

    }
    else debugSend("No response");
}

void simGPSStart()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CGPSPWR=1");
    while(simAvailable()==0)
    {
        counter++;
        if (counter > 500000)
        {
            simSendRaw('A');
            return 0;
        }
    }
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}

void simGPSRestartCold()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CGPSRST=0");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}

void simGPSSResetWarm()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CGPSRST=2");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}
void simGPSInfo()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CGPSINF=0");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}
void simGPSStatus()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CGPSSTATUS?");
    while(simAvailable()==0)
    {
        counter++;
        if (counter > 500000)
        {
            simSendRaw('A');
            return 0;
        }
    }
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}

void simGSMLoc(char * gsmLocString)
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CIPGSMLOC=1");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}

void simParseGSMLoc(struct sim808_t * sim808)
{
    static char gsmLocString[100]="CIPGSMLOC: 0,19.667806,49.978185,2014/03/20,14:12:27", locationCode[20]="", longitudeCoord[20]="", latitudeCoord[20]="", remainder[30]="";
    //simGSMLoc(&gsmLocString);
    debugSend("\n+-+-Response ");
    debugSend(gsmLocString);

    sscanf(gsmLocString, "%[^,],%[^,],%[^,],%[^,]", &locationCode, &longitudeCoord, &latitudeCoord, &remainder);
    debugSend("\nLoc code: ");
    debugSend(locationCode);
    debugSend("\nLongitude: ");
    debugSend(longitudeCoord);
    debugSend("\nLatitude code: ");
    debugSend(latitudeCoord);
}

void simEnableCharge()
{

    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+ECHARGE=1");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");

    counter=0;
    flushReceiveBuffer();
    simSend("AT&W");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}

void simSignalQuality()
{
    uint32_t counter=0;
    flushReceiveBuffer();
    simSend("AT+CSQ");
    delayMilliIT(30);
    if(rxBufLen>0)
    {
        debugSend(rxBuf);
    }
    else debugSend("No response");
}
uint8_t simNoEcho()
{
    if(simCheckResult("ATE0", "OK", 500000)==1) return 1;
    else return 0;
}
uint8_t simResetIPSession()
{
    ///can take up to 65 seconds to respond
    if(simCheckResult("AT+CIPSHUT", "SHUT OK", 4000000000)==1) return 1;
    else
    {
///+PDP: DEACT is returned then send AT+CIPSHUT again to make PDP context come back to original state
        if(strstr(rxBuf, "DEACT") != NULL)
        {
            if(simCheckResult("AT+CIPSHUT", "SHUT OK", 4000000000)==1) return 1;
            else return 0;
        }
        else return 0;
    }
}

uint8_t simMUX()
{
    if(simCheckResult("AT+CIPMUX=0", "OK", 500000)==1) return 1;
    else return 0;
}

uint8_t simAPN()
{
    if(simCheckResult("AT+CSTT=\"internet\"", "OK", 500000)==1) return 1;
    else return 0;
}

uint8_t simBringUpWireless()
{
    //can take up to 85 seconds
    if(simCheckResult("AT+CIICR", "OK", 4258096384)==1) return 1;
    else return 0;
}
uint8_t simPing(void)
{
    uint32_t counter=0;

    flushReceiveBuffer();
    simSend("AT");
    while(simAvailable()==0)
    {
        counter++;
        if (counter > 300000)
        {
            return 0;
        }
    }

    if(rxBufLen>0)
    {
        if(strstr(rxBuf, "\r\nOK\r\n") != NULL) return 1;
    }
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


void simUpdateState(tcp_state* current_state)
{
    if(simCheckResult("AT+CIPSTATUS", "OK", 20000000)==1)
    {
        if(strstr(rxBuf, "INITIAL") != NULL) *current_state = STATE_INITIAL;
        else if((strstr(rxBuf, "START") != NULL)) *current_state = STATE_START;
        else if((strstr(rxBuf, "CONFIG") != NULL)) *current_state = STATE_CONFIG;
        else if((strstr(rxBuf, "GPRSACT") != NULL)) *current_state = STATE_GPRSACT;
        else if((strstr(rxBuf, "IP STATUS") != NULL)) *current_state = STATE_STATUS;
        else if((strstr(rxBuf, "TCP CONNECTING") != NULL)) *current_state = STATE_CONNECTING;
        else if((strstr(rxBuf, "CONNECT OK") != NULL)||(strstr(rxBuf, "ALREADY CONNECT") != NULL)) *current_state = STATE_CONNECTED;
        else if((strstr(rxBuf, "CLOSING") != NULL)) *current_state = STATE_CLOSING;
        else if((strstr(rxBuf, "CLOSED") != NULL)) *current_state = STATE_CLOSED;
        else if((strstr(rxBuf, "PDP") != NULL)) *current_state = STATE_PDPDEACT;
    }
    //debugSend(rxBuf);
}



void checkInitalStatus(tcp_state* current_state)
{
    uint8_t countdown;
    char pass=0;
    #if SIMVERBOSE
    debugSend("checking the sim808's state");
    #endif
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
    countdown=0;
    pass = 0;
    while((countdown<5)&&(pass==0))
    {
        countdown++;
        delayMilliIT(6000);
        if(simNetReg()==0)
        {
            ///if the sim is not registered then Reboot microcontroller
            debugSend("Not registered yet...\n");
            debugSend(rxBuf);
        }
        else pass = 1;
    }
    if(pass==0)
    {
        debugSend("Rebooting\n");
        NVIC_SystemReset();
    }

    ///the device is registered so check if GPRS is attached.
    debugSend("device is registered. Checking GPRS\n");

    countdown=0;
    pass = 0;
    while((countdown<5)&&(pass==0))
    {
        countdown++;
        delayMilliIT(2000);
        if(simGPRSAttached()==0)
        {
            ///if GPRS is NOT attached then reboot microcontroller
            debugSend("GPRS not attached\n");
            simSend("AT+CGATT=1");///Try attach the GPRS service
            delayMilliIT(100);
            debugSend(rxBuf);
            delayMilliIT(100);
        }
        else pass = 1;
    }
    if(pass==0)
    {
        debugSend("Rebooting\n");
        NVIC_SystemReset();
    }
    debugSend("GPRS is attached\n");

    countdown=0;
    pass = 0;
    while((countdown<5)&&(pass==0))
    {
        countdown++;
        delayMilliIT(2000);
        if(simResetIPSession()==0)
        {
            ///Not able to reset the IP Session
            debugSend("Not able to reset the IP session - rebooting\n");
        }
        else pass = 1;

    }
    if(pass==0)
    {
        debugSend("Rebooting\n");
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



void whatStateAmIIn(tcp_state* current_state)
{
    #ifdef SIMVERBOSE
    debugSend("\nDebug state \n");
    switch(*current_state)
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
    #endif
}



void simConnect(tcp_state* current_state)
{
    tcp_state tempState = *current_state;
    debugSend("position 1");
    char connString[]="AT+CIPSTART=\"TCP\",\"m11.cloudmqtt.com\",\"14672\"";
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
    while(tempState!=STATE_START)
    {
        debugSend(rxBuf);
        delayMilliIT(50);
        simUpdateState(&tempState);
        whatStateAmIIn(&tempState);
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

    while(tempState != STATE_GPRSACT)
    {
        simUpdateState(&tempState);
        whatStateAmIIn(&tempState);
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
    //*current_state = (tcp_state)tempState;
}
