#include "Delay.h"
#include "stm32f0xx_conf.h"
#include <stdint.h>
static volatile uint32_t TimingDelay;
uint8_t msgTimout=0;


void initDelay()
{
    //Delay init with NVIC
    if (SysTick_Config(SystemCoreClock / 1000))
		while (1);
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void delayMilli(volatile uint32_t nTime)
{
    TimingDelay = nTime *8000;
	while (--TimingDelay != 0);
}

void delayMilliIT(volatile uint32_t nTime)
{
    TimingDelay = nTime;
	while (TimingDelay != 0);
}

void TimingDelay_Decrement(void) {
	if (TimingDelay > 0x00) {
		TimingDelay--;
	}
}

void delayIncCounter(struct timing *tim)
{

}

void SysTick_Handler(void) {
	TimingDelay_Decrement();
	if(msgTimout<20) msgTimout++;
}
