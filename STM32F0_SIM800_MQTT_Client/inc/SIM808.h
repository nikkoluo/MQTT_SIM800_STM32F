#ifndef SIM808_H_INCLUDED
#define SIM808_H_INCLUDED
#include <stdint.h>
void simInit(void);
int simConnect();
void simSend(const char* data);
int simReceive(const uint8_t* data, uint8_t length);
int simAvailable(void);
int simRead(void);

#endif /* SIM808_H_INCLUDED */
