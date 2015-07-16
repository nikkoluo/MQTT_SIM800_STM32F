#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

void debugInit(void);
void debugSend(char StringToSend[]);
void debugSend2(char StringToSend[], int len);
void debugReceive();
#endif /* DEBUG_H_INCLUDED */
