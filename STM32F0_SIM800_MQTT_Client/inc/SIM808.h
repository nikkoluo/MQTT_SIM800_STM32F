#ifndef SIM808_H_INCLUDED
#define SIM808_H_INCLUDED

void Sim808_init(void);
int Sim808_connect();
int Sim808_send(const uint8_t* data, size_t length);
int Sim808_receive(const uint8_t* data, size_t length);


#endif /* SIM808_H_INCLUDED */
