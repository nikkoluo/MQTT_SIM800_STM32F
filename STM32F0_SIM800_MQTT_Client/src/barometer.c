#include "barometer.h"


void I2CInit()
{
    //Make sure GPIOInit is called before this funciton is called
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_Timing = ;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress
    I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Disable;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);




}
