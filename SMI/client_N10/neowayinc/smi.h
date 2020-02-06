/***********************************************************************
* File Name	: smi.h
* Author	: Vinayaka 
*			  Embedded Software Engineer, 
* 			  Royal Elegance Technologies, Bangalore.
************************************************************************/
#ifdef __EXAMPLE_HELLWORLD__

#include "neoway_openplatform.h"

#define RED_LED_PIN NEOWAY_GPIO_33
#define GREEN_LED_PIN NEOWAY_GPIO_35
#define SIM_DET_PIN NEOWAY_GPIO_31

/**********************Battry Variable Declaration**********************/
extern float Cur_Bat_Voltage, Pre_Bat_Voltage, sensor_Voltage;
extern U32 BAT_Percent, Current_percentage, Previous_percentage, battery_low_flag, Pre_Bat_Voltage_flag;
extern U8 symbol;
extern U32 batery_voltage_level, sensor_Voltage_level;

typedef enum{
	BAT_LOW=0,
	BAT_HIGH=1,
}Neo_highlow_enum;

extern Neo_highlow_enum BAT_LOW_FLAG;
/************************Function Declaration****************************/

extern void Neo_Uart_Init();
extern void Neo_gpio_Init();
extern void asciitohex(char *str);
extern float GET_BAT_VOLTAGE(void);
extern float moisture_sense(void);
extern void moisture_cmp(float vtg_level, float moist_level, char* msg_buf);

#endif