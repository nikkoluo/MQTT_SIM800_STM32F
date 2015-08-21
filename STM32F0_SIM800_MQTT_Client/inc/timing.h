#ifndef TIMING_H_INCLUDED
#define TIMING_H_INCLUDED

#include <stdint.h>


void initDelay();
void delayMilliIT(volatile uint32_t nTime);
void delayMilli(volatile uint32_t nTime);


#endif /* DELAY_H_INCLUDED */
