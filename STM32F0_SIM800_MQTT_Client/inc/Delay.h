#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#include <stdint.h>

struct timing {
    uint8_t *counter;

};

void initDelay();
void delayMilliIT(volatile uint32_t nTime);
void delayMilli(volatile uint32_t nTime);


#endif /* DELAY_H_INCLUDED */
