#include "stm32f0xx_conf.h"
#include <stdio.h>
#include "BMP180.h"


void I2CInit()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_1);
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //Make sure GPIOInit is called before this funciton is called
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_Timing = 0x0010020B;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_InitStruct.I2C_DigitalFilter=0;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
}


/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/

static void writeCommand(char reg, char value)
{
    uint8_t i;
    I2C_TransferHandling(I2C1, BARO1_WRITE, 2, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXIS) == RESET);

	I2C_SendData(I2C1, reg);
	for(i=0;i<200;i++);
	I2C_SendData(I2C1, value);
    I2C_GenerateSTOP(I2C1, ENABLE);

}

void read8(uint8_t reg, uint8_t *value)
{
    uint8_t i;
    I2C_TransferHandling(I2C1, BARO1_WRITE, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXIS) == RESET);
	I2C_SendData(I2C1, reg);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_TransferHandling(I2C1, BARO1_READ, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Read);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_RXNE) == RESET);
	i = I2C_ReceiveData(I2C1);
    while(I2C_GetFlagStatus(I2C1, I2C_ISR_TC) == RESET);
    I2C_GenerateSTOP(I2C1, ENABLE);
    *value =i;
    #if BMPVERBOSE
    _printfLngS("read8: ", i);
    #endif
}

void read16(uint8_t reg, uint32_t *value)
{
    uint16_t i1=0, i2=0, i;
    uint16_t temp;
    read8(reg, &i1);
    //_printfU("-MSB ", i1);
    for(i=0; i<200; i++);
    read8(reg+1, &i2);
    //_printfU("-LSB ", i2);
    temp = (i1<<8) + i2;
    #if BMPVERBOSE
    _printfU("-16BYTE ", temp);
    #endif
	*value = temp;
}
void readRawTemperature(int32_t *temperature)
{
    uint32_t i;
    int32_t t;
    writeCommand(BMP180_CTRL_MEAS_REG, BMP180_T_MEASURE);
    for(i=0; i<20000; i++);//delay
    read16(BMP180_ADC_OUT_MSB_REG, &t);
    *temperature = t;
    /*uncomment to enable debugging*/
    #if BMPVERBOSE2
    _printfLngS("raw temperature: ", t);
    #endif
}

void readRawPressure( int32_t *pressure, uint8_t oss)
{
    #if BMPVERBOSE2
    debugSend("starting raw pressure read\n");
    #endif
    uint8_t  p8;
    uint32_t p16;
    int32_t  p32;
    uint32_t i;

    writeCommand(BMP180_CTRL_MEAS_REG, BMP180_P_MEASURE + (oss << 6));
    #if BMPVERBOSE
    _printfU("still good:",1);
    #endif
    for(i=0; i<100000; i++) ;//delay
    #if BMPVERBOSE
    _printfU("still good:",2);
    #endif
    read16(BMP180_ADC_OUT_MSB_REG, &p16);
    #if BMPVERBOSE
    _printfLngU("check pre <<8:",p16);
    #endif
    p16 = (uint32_t)(p16<<8);
    #if BMPVERBOSE
    _printfLngU("check post <<8:",p16);
    #endif
    p32 = (uint32_t)p16;
    #if BMPVERBOSE
    _printfLngU("check p16 = p32:",p32);
    #endif
    read8(BMP180_ADC_OUT_MSB_REG+2, &p8);
    #if BMPVERBOSE
    _printfU("check p8:",p8);
    #endif
    p32 += p8;
    #if BMPVERBOSE
    _printfLngU("check p32 += p8:",p32);
    #endif
    p32 >>= (8 - oss);
    #if BMPVERBOSE
    _printfLngU("check p32 >>= (8 - oss):",p32);
    _printfU("still good:",6);
    #endif
    *pressure = p32;
    /*uncomment to enable debugging*/
    #if BMPVERBOSE2
    _printfLngS("raw pressure: ", p32);
    #endif
}

void getPressure(int32_t *pressure, struct bmp180_t* sensorX)
{

  int32_t  ut = 0, up = 0, compp = 0;
  int32_t  x1, x2, b5, b6, x3, b3, p;
  uint32_t b4, b7;

  /* Get the raw pressure and temperature values */
  readRawTemperature(&ut);
  readRawPressure(&up, sensorX->mode);

  /* Temperature compensation */
  int32_t X1 = (ut - (int32_t)sensorX->calib_param.ac6) * ((int32_t)sensorX->calib_param.ac5) >> 15;
  int32_t X2 = ((int32_t)sensorX->calib_param.mc << 11) / (X1+(int32_t)sensorX->calib_param.md);
  b5 = X1 + X2;
#if BMPVERBOSE
_printfLngS("B5: ", b5);
#endif
  /* Pressure compensation */
  b6 = b5 - 4000;
  x1 = ((sensorX->calib_param.b2) * ((b6 * b6) >> 12)) >> 11;
  x2 = (sensorX->calib_param.ac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((((int32_t) sensorX->calib_param.ac1) * 4 + x3) << sensorX->mode) + 2) >> 2;
  x1 = (sensorX->calib_param.ac3 * b6) >> 13;
  x2 = (sensorX->calib_param.b1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (sensorX->calib_param.ac4 * (uint32_t) (x3 + 32768)) >> 15;
  b7 = ((uint32_t) (up - b3) * (50000 >> sensorX->mode));

  if (b7 < 0x80000000)
  {
    p = (b7 << 1) / b4;
  }
  else
  {
    p = (b7 / b4) << 1;
  }

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  compp = p + ((x1 + x2 + 3791) >> 4);
  /* Assign compensated pressure value */
  *pressure = compp;
}

void getTemperature(float *temp, struct bmp180_t* sensorX)
{
  int32_t UT, X1, X2, B5;     // following ds convention
  float t;

  readRawTemperature(&UT);

  X1 = (UT - (int32_t)sensorX->calib_param.ac6) * ((int32_t)sensorX->calib_param.ac5) >> 15;
  X2 = ((int32_t)sensorX->calib_param.mc << 11) / (X1+(int32_t)sensorX->calib_param.md);
  B5 = X1 + X2;
  t = (B5+8) >> 4;
  t /= 10;

  *temp = t;
  #if BMPVERBOSE2
  _printfLngS("temperature: ", t);
  #endif
}

void bmp180_get_calib_param(struct bmp180_t* sensorX)
{
    uint16_t unsignedShortData, i;
    char debugString[100]="";

    sensorX->mode =3;
    read16(0xAA, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.ac1 = (int16_t)unsignedShortData;

    read16(0xAC, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.ac2 = (int16_t)unsignedShortData;

    read16(0xAE, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.ac3 = (int16_t)unsignedShortData;

    read16(0xB0, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.ac4 = unsignedShortData;

    read16(0xB2, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.ac5 = unsignedShortData;

    read16(0xB4, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.ac6 = unsignedShortData;

    read16(0xB6, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.b1 = (int16_t)unsignedShortData;

    read16(0xB8, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.b2 = (int16_t)unsignedShortData;

    read16(0xBA, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.mb = (int16_t)unsignedShortData;

    read16(0xBC, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.mc = (int16_t)unsignedShortData;

    read16(0xBE, &unsignedShortData);
    for(i=0;i<480;i++);
    sensorX->calib_param.md = (int16_t)unsignedShortData;

    /*Uncomment to debug the values */
    #if VERBOSE2
    sprintf(debugString, "AC1 %d AC2 %d AC3 %d AC4 %u AC5 %u AC6 %u B1 %d B2 %d MB %d MC %d MD %d \n", Sensor1.calib_param.ac1,Sensor1.calib_param.ac2,Sensor1.calib_param.ac3, Sensor1.calib_param.ac4, Sensor1.calib_param.ac5, Sensor1.calib_param.ac6, Sensor1.calib_param.b1, Sensor1.calib_param.b2, Sensor1.calib_param.mb, Sensor1.calib_param.mc, Sensor1.calib_param.md);
    debugSend(debugString);
    #endif

}
