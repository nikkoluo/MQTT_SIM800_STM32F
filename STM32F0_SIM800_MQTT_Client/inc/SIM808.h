#ifndef SIM808_H_INCLUDED
#define SIM808_H_INCLUDED
#include <stdint.h>
void Sim808_init(void);
int Sim808_connect();
int Sim808_send(const char* data);
int Sim808_receive(const uint8_t* data, uint8_t length);


#endif /* SIM808_H_INCLUDED */
