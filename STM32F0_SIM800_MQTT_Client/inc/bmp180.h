#ifndef BMP180_H_INCLUDED
#define BMP180_H_INCLUDED


/***************************************************************/
/**\name	DEBUGGING ENABLER       */
/***************************************************************/
#define BMPVERBOSE 0
#define BMPVERBOSE2 0

/***************************************************************/
/**\name	I2C ADDRESS DEFINITION OF BMP180       */
/***************************************************************/
/*BMP180 I2C Address*/

#define BARO1_WRITE 0xEE
#define BARO1_READ 0xEF


/***************************************************************/
/**\name	ERROR CODE DEFINITIONS    */
/***************************************************************/
#define E_BMP_NULL_PTR				((int8_t)-127)
#define E_BMP_COMM_RES				((int8_t)-1)
#define E_BMP_OUT_OF_RANGE			((int8_t)-2)
/***************************************************************/
/**\name	CONSTANTS       */
/***************************************************************/
#define BMP180_RETURN_FUNCTION_TYPE        int8_t
#define   BMP180_INIT_VALUE						((uint8_t)0)
#define   BMP180_INITIALIZE_OVERSAMP_SETTING_U8X	((uint8_t)0)
#define   BMP180_INITIALIZE_SW_OVERSAMP_U8X			((uint8_t)0)
#define   BMP180_INITIALIZE_NUMBER_OF_SAMPLES_U8X	((uint8_t)1)
#define   BMP180_GEN_READ_WRITE_DATA_LENGTH			((uint8_t)1)
#define   BMP180_TEMPERATURE_DATA_LENGTH			((uint8_t)2)
#define   BMP180_PRESSURE_DATA_LENGTH				((uint8_t)3)
#define   BMP180_SW_OVERSAMP_U8X					((uint8_t)1)
#define   BMP180_OVERSAMP_SETTING_U8X				((uint8_t)3)
#define   BMP180_2MS_DELAY_U8X			(2)
#define   BMP180_3MS_DELAY_U8X			(3)
#define   BMP180_AVERAGE_U8X			(3)
#define   BMP180_INVALID_DATA			(0)
#define   BMP180_CHECK_DIVISOR			(0)
#define   BMP180_DATA_MEASURE			(3)
#define   BMP180_CALCULATE_TRUE_PRESSURE		(8)
#define   BMP180_CALCULATE_TRUE_TEMPERATURE		(8)
#define BMP180_SHIFT_BIT_POSITION_BY_01_BIT			(1)
#define BMP180_SHIFT_BIT_POSITION_BY_02_BITS		(2)
#define BMP180_SHIFT_BIT_POSITION_BY_04_BITS		(4)
#define BMP180_SHIFT_BIT_POSITION_BY_06_BITS		(6)
#define BMP180_SHIFT_BIT_POSITION_BY_08_BITS		(8)
#define BMP180_SHIFT_BIT_POSITION_BY_11_BITS		(11)
#define BMP180_SHIFT_BIT_POSITION_BY_12_BITS		(12)
#define BMP180_SHIFT_BIT_POSITION_BY_13_BITS		(13)
#define BMP180_SHIFT_BIT_POSITION_BY_15_BITS		(15)
#define BMP180_SHIFT_BIT_POSITION_BY_16_BITS		(16)
/***************************************************************/
/**\name	REGISTER ADDRESS DEFINITION       */
/***************************************************************/
/*register definitions */

#define BMP180_PROM_START__ADDR		(0xAA)
#define BMP180_PROM_DATA__LEN		(22)

#define BMP180_CHIP_ID_REG			(0xD0)
#define BMP180_VERSION_REG			(0xD1)

#define BMP180_CTRL_MEAS_REG		(0xF4)
#define BMP180_ADC_OUT_MSB_REG		(0xF6)
#define BMP180_ADC_OUT_LSB_REG		(0xF7)

#define BMP180_SOFT_RESET_REG		(0xE0)

/* temperature measurement */
#define BMP180_T_MEASURE			(0x2E)
/* pressure measurement*/
#define BMP180_P_MEASURE			(0x34)
/* TO be spec'd by GL or SB*/
#define BMP180_TEMP_CONVERSION_TIME  (5)

#define BMP180_PARAM_MG		(3038)
#define BMP180_PARAM_MH		(-7357)
#define BMP180_PARAM_MI		(3791)

/****************************************************/
/**\name	ARRAY SIZE DEFINITIONS      */
/***************************************************/
#define	BMP180_TEMPERATURE_DATA_BYTES	(2)
#define	BMP180_PRESSURE_DATA_BYTES		(3)
#define	BMP180_TEMPERATURE_LSB_DATA		(1)
#define	BMP180_TEMPERATURE_MSB_DATA		(0)
#define	BMP180_PRESSURE_MSB_DATA		(0)
#define	BMP180_PRESSURE_LSB_DATA		(1)
#define	BMP180_PRESSURE_XLSB_DATA		(2)

#define	BMP180_CALIB_DATA_SIZE			(22)
#define	BMP180_CALIB_PARAM_AC1_MSB		(0)
#define	BMP180_CALIB_PARAM_AC1_LSB		(1)
#define	BMP180_CALIB_PARAM_AC2_MSB		(2)
#define	BMP180_CALIB_PARAM_AC2_LSB		(3)
#define	BMP180_CALIB_PARAM_AC3_MSB		(4)
#define	BMP180_CALIB_PARAM_AC3_LSB		(5)
#define	BMP180_CALIB_PARAM_AC4_MSB		(6)
#define	BMP180_CALIB_PARAM_AC4_LSB		(7)
#define	BMP180_CALIB_PARAM_AC5_MSB		(8)
#define	BMP180_CALIB_PARAM_AC5_LSB		(9)
#define	BMP180_CALIB_PARAM_AC6_MSB		(10)
#define	BMP180_CALIB_PARAM_AC6_LSB		(11)
#define	BMP180_CALIB_PARAM_B1_MSB		(12)
#define	BMP180_CALIB_PARAM_B1_LSB		(13)
#define	BMP180_CALIB_PARAM_B2_MSB		(14)
#define	BMP180_CALIB_PARAM_B2_LSB		(15)
#define	BMP180_CALIB_PARAM_MB_MSB		(16)
#define	BMP180_CALIB_PARAM_MB_LSB		(17)
#define	BMP180_CALIB_PARAM_MC_MSB		(18)
#define	BMP180_CALIB_PARAM_MC_LSB		(19)
#define	BMP180_CALIB_PARAM_MD_MSB		(20)
#define	BMP180_CALIB_PARAM_MD_LSB		(21)

/**************************************************************/
/**\name	STRUCTURE DEFINITIONS                         */
/**************************************************************/
/*!
 * @brief This structure holds all device specific calibration parameters
 */
struct bmp180_calib_param_t {
	int16_t ac1;/**<calibration ac1 data*/
	int16_t ac2;/**<calibration ac2 data*/
	int16_t ac3;/**<calibration ac3 data*/
	uint16_t ac4;/**<calibration ac4 data*/
	uint16_t ac5;/**<calibration ac5 data*/
	uint16_t ac6;/**<calibration ac6 data*/
	int16_t b1;/**<calibration b1 data*/
	int16_t b2;/**<calibration b2 data*/
	int16_t mb;/**<calibration mb data*/
	int16_t mc;/**<calibration mc data*/
	int16_t md;/**<calibration md data*/
};
/*!
 * @brief This structure holds BMP180 initialization parameters
 */
struct bmp180_t {
	struct bmp180_calib_param_t calib_param;/**<calibration data*/
	uint8_t mode;/**<power mode*/
	uint8_t chip_id; /**<chip id*/
	uint8_t ml_version;/**<ml version*/
	uint8_t al_version;/**<al version*/
	uint8_t dev_addr;/**<device address*/
	uint8_t sensortype;/**< sensor type*/

	int32_t param_b5;/**<pram*/
	int32_t number_of_samples;/**<sample calculation*/
	int16_t oversamp_setting;/**<oversampling setting*/
	int16_t sw_oversamp;/**<software oversampling*/


	void (*delay_msec)(BMP180_MDELAY_DATA_TYPE);/**< delay function pointer*/
};
/**************************************************************/
/**\name	BIT MASK, LENGTH AND POSITION FOR REGISTERS     */
/**************************************************************/
/**************************************************************/
/**\name	BIT MASK, LENGTH AND POSITION FOR
 CHIP ID REGISTERS     */
/**************************************************************/
#define BMP180_CHIP_ID__POS             (0)
#define BMP180_CHIP_ID__MSK             (0xFF)
#define BMP180_CHIP_ID__LEN             (8)
#define BMP180_CHIP_ID__REG             (BMP180_CHIP_ID_REG)
/**************************************************************/
/**\name	BIT MASK, LENGTH AND POSITION FOR
   ML VERSION  */
/**************************************************************/
#define BMP180_ML_VERSION__POS          (0)
#define BMP180_ML_VERSION__LEN          (4)
#define BMP180_ML_VERSION__MSK          (0x0F)
#define BMP180_ML_VERSION__REG          (BMP180_VERSION_REG)
/**************************************************************/
/**\name	BIT MASK, LENGTH AND POSITION FOR
   AL VERSION  */
/**************************************************************/
#define BMP180_AL_VERSION__POS          (4)
#define BMP180_AL_VERSION__LEN          (4)
#define BMP180_AL_VERSION__MSK          (0xF0)
#define BMP180_AL_VERSION__REG          (BMP180_VERSION_REG)
/**************************************************************/
/**\name	GET AND SET BITSLICE FUNCTIONS*/
/**************************************************************/




#endif /* BMP180_H_INCLUDED */
