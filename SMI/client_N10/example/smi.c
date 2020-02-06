/***********************************************************************
* File Name	: smi.c
* Author	: Vinayaka 
*			  Embedded Software Engineer, 
* 			  Royal Elegance Technologies, Bangalore.
************************************************************************/
#ifdef __EXAMPLE_HELLWORLD__

#include "neoway_openplatform.h"
#include "smi.h"

/*************************Battery Variables*****************************/ 

float Cur_Bat_Voltage=0, Pre_Bat_Voltage=0, sensor_Voltage=0;
U32 BAT_Percent=0, Current_percentage=0, Previous_percentage=0, battery_low_flag=0, Pre_Bat_Voltage_flag=0;
U8 symbol='\x25';
U32 batery_voltage_level, sensor_Voltage_level=0;

Neo_highlow_enum BAT_LOW_FLAG = BAT_HIGH;

/*************************UART Init function*****************************/ 
void Neo_Uart_Init(){
	Neoway_UartOpen(NEOWAY_UART_PORT_1);
	Neoway_SetBaudrate(NEOWAY_UART_PORT_1, NEOWAY_UART_BAUD_9600);
	
	// Neoway_UartOpen(NEOWAY_UART_PORT_2);
	// Neoway_SetBaudrate(NEOWAY_UART_PORT_2, NEOWAY_UART_BAUD_9600);
}
/*****************************************************************************
 * Definition:   Neo_gpio_Init
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void Neo_gpio_Init(){
	Neoway_GpioModeConfigure(RED_LED_PIN,NEOWAY_MODE_GPIO);      
	Neoway_GpioDirectionConfigure(RED_LED_PIN,1);   //Set GPIO_33 output
	Neoway_GpioWrite(RED_LED_PIN,1);		      // for module turn-ON indication
	
	Neoway_GpioModeConfigure(GREEN_LED_PIN,NEOWAY_MODE_GPIO);   //Set GPIO_35 output   
	Neoway_GpioDirectionConfigure(GREEN_LED_PIN,1);   // for signal strength indication
	// Neoway_GpioWrite(GREEN_LED_PIN,1);	
	
	Neoway_GpioModeConfigure(SIM_DET_PIN,NEOWAY_MODE_GPIO);    //Set GPIO_13 output 
	Neoway_GpioDirectionConfigure(SIM_DET_PIN,0);       // for sim detection
}
/*****************************************************************************
 * Definition:   asciitohex
 * Parameter:   char *str
 * Return value:   void
 *****************************************************************************/
void asciitohex(char *str)
{
    unsigned char strH[200];
    int i,j;
     
    memset(strH,0,sizeof(strH));			/*set strH with nulls*/
    for(i=0,j=0;i<strlen(str);i++,j+=2)		/*converting str character into Hex and adding into strH*/
    { 
        sprintf((char*)strH+j,"%02X",str[i]); // %02x means print at least 2 digits
    }
    strH[j]='\0'; 							/*adding NULL in the end*/
    strcpy(str,strH);
    return;
}
/*****************************************************************************
 * Definition:   GET_BAT_VOLTAGE
 * Parameter:   void
 * Return value:   float
 *****************************************************************************/
float GET_BAT_VOLTAGE(void)
{
	U16 adc1_reading=0;
	int x;
	float temp1=0;
	Neoway_Print("Battery voltage reading...");
	for(x=0;x<10;x++)
	{
		adc1_reading = Neoway_ReadAdcValue(1);
		temp1 += ((float)adc1_reading/2797.0)*2.8*2.05;
		// Neoway_Print("temp1 = %f volts\r",temp1);
		Neoway_us_delay(1000000);
	}
	Cur_Bat_Voltage=temp1/10.0;
	Neoway_Print("Cur_Bat_Voltage = %f volts\r",Cur_Bat_Voltage);
	if(Pre_Bat_Voltage_flag==0)
	{
		Pre_Bat_Voltage_flag=1;
		Pre_Bat_Voltage = Cur_Bat_Voltage;;
	}
	else if(Cur_Bat_Voltage < Pre_Bat_Voltage)
	{
		Pre_Bat_Voltage = Cur_Bat_Voltage;
	}
	else if(Cur_Bat_Voltage > Pre_Bat_Voltage)
	{
		Pre_Bat_Voltage = Pre_Bat_Voltage;
	}
	if(Pre_Bat_Voltage<3.5)
		Pre_Bat_Voltage=3.5;
	else if(Pre_Bat_Voltage> 4.15)
		Pre_Bat_Voltage=4.15;
	batery_voltage_level=((Pre_Bat_Voltage-3.5)/(4.15-3.5))*100;	
	Neoway_Print("Battery_Voltage_Level=%d%c",batery_voltage_level,symbol);
	// Neoway_Print("Battery_Voltage=%fvolts, Battery_Level=%d%c adc1_reading=%d\r",Pre_Bat_Voltage,batery_voltage_level,symbol,adc1_reading);
	if(batery_voltage_level > 30 && BAT_LOW_FLAG == BAT_LOW){
		BAT_LOW_FLAG = BAT_HIGH;
	}
	if(batery_voltage_level < 30 && BAT_LOW_FLAG == BAT_HIGH){
		BAT_LOW_FLAG = BAT_LOW;
	}
	return Pre_Bat_Voltage;
}
/*****************************************************************************
 * Definition:   moisture_sense
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
float moisture_sense(void)
{
	unsigned int adc0_reading=0; 
	int x;
	float temp1=0,temp2=0;
	Neoway_Print("reading moisture sensor data...");
	for(x=0;x<10;x++)
	{
		adc0_reading=Neoway_ReadAdcValue(0);
		temp1=(adc0_reading/2797.0)*2.8;
		temp2 = temp2+temp1;
	}
	sensor_Voltage = temp2/10.0;
	// Neoway_Print("sensor_Voltage is = %f volts",sensor_Voltage);
	// sensor_Voltage_level=(int)((sensor_Voltage - minimum_cal_value)/(maximum_cal_value - minimum_cal_value)) * 100;
	Neoway_Print("sensor_Voltage is = %f volts\r",sensor_Voltage); //  moisture_Level=%d%c",sensor_Voltage,sensor_Voltage_level,symbol);

	return sensor_Voltage;
}
/*****************************************************************************
 * Definition:   moisture_cmp
 * Parameter:   float x, char* msg_buf
 * Return value:   void
 *****************************************************************************/
void moisture_cmp(float vtg_level, float moist_level, char* msg_buf)
{
	if(vtg_level <= 3.5) {
		sprintf(msg_buf,"moisture sensor battery is low %d%c\0",batery_voltage_level,symbol);
	}
	else if(sensor_Voltage < 1.75) {
		sprintf(msg_buf,"Battery=%d%c, moisture %0.2fV High Moisture\0",batery_voltage_level,symbol,sensor_Voltage);
	}
	else if(sensor_Voltage < 2.25) {	
		sprintf(msg_buf,"Battery=%d%c, moisture %0.2fV No Need For Irrigation\0",batery_voltage_level,symbol,sensor_Voltage);
	}
	else if(sensor_Voltage > 2.26) {	
		sprintf(msg_buf,"Battery=%d%c, moisture %0.2fV Low Moisture\0",batery_voltage_level,symbol,sensor_Voltage);
	}	
}

#endif