#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
#include "umqtt.h"
#include "stm32f0xx_gpio.h"


#define SIM_PWR GPIO_Pin_5

#define MQTT_URL 				"m11.cloudmqtt.com"

#define MQTT_KEEP_ALIVE			60
#define MQTT_CLIENT_ID			"stmtest"
#define MQTT_USERNAME			"stm"
#define MQTT_PASSWORD			"123"
#define MQTT_TOPIC_GPS_LONG		"gps/longitude"
#define MQTT_TOPIC_GPS_LAT		"gps/latitude"


#endif /* CONFIG_H_INCLUDED */
