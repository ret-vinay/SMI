#ifdef __EXAMPLE_HELLWORLD__

#include "neoway_openplatform.h"
#include "user.h"
#include "smi.h"
#include "neoway_message.h"

/**************************Function declaration****************************************/
void RTC_Function(void);
void extract_message(char *);
char* strstr(char *, char *);
void Neoway_ms_delay(U16 );
char* hextoascii(char *text);
void check_in_msg_task(void);
void call_fun_change(U8 *Rec_Msg, U16 sige);
void Read_fixed_file(void);
U32 asctoint(char *str);
void fun_LED_off(void);
void fun_RTC_Change(void);
void Reset_RTC_Fun(void);
/**************************************************************************************/
/**********************message cmd Declaration****************************************/
Neo_msgcmd_enum cmd=NOCMD;
/**************************************************************************************/

/**********************RTC Variable Declaration****************************************/
NeowayRtcStruct  rtc_time;
static U32 Hour=0,Min=0,Sec=0;
float temp_H=0,temp_M=0,temp_S=0;
/**************************************************************************************/
/**********************Messages Variable Declaration**********************************/
U8 message_string[50]={0}, Local_msg_buf[1024]={0}, msg_buf[1024]={0};
U8 Rec_Mbl_No[14]={0}, Rec_Msg[200]={0};
U32 no_of_sms;
/**************************************************************************************/
/*******************************control flags******************************************/
U16 MSG_EXTRCTN = 0, MSG_EXTRCTN_CHECK = 0, SEND_MSG = 0, DEV_IN_SLEEP = 0;
static U16 RTC_TIME_CHANGE = 0, TYPE_FIRST = 0, TYPE_HOUR = 0, TYPE_DAY = 0; 
/**************************************************************************************/
/*******************************file name buffer's**************************************/
U8 file_name_str1[10]="amn.txt";
U8 file_name_str2[10]="dmn.txt";
U8 file_name_str3[10]="spd.txt";
/**************************************************************************************/

/*******************************mobile no buffer's**************************************/
U8 authorised_mobile_no[14]={0}, desti_mob_number[11]={0}; 
U8 number_of_spd[4]={0}, number_of_sph[4]={0};
U8 msg_data_changed[160]={0}, MSG_Send_Command[30]={0};
/**************************************************************************************/

/*****************************************************************************
 * Definition:   RTC_Function
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void RTC_Function(void)
{	
	Neoway_Print("neoway_RTC_task\r");
	Neoway_GetTime(&rtc_time);	//	get system time
	Neoway_Print("current time is %dH::%dM::%dS",rtc_time.rtc_hour,rtc_time.rtc_min,rtc_time.rtc_sec);
	
	rtc_time.rtc_hour=00;
	rtc_time.rtc_min=00;
	rtc_time.rtc_sec=00;
	
	Neoway_SetTime(&rtc_time);		//set system time
	
	// rtc_time.rtc_hour = 00;
	// rtc_time.rtc_min = 01;  //  for testing purpose
	// rtc_time.rtc_sec = 59;
	
	rtc_time.rtc_hour = Hour;
	rtc_time.rtc_min = Min;
	rtc_time.rtc_sec = Sec;
	
	stop_timers();
	fun_LED_off();
	fun_LED_off();
	DEV_IN_SLEEP = 1;
	Neoway_Print("Device wakeup after %dH::%dM::%dS",rtc_time.rtc_hour,rtc_time.rtc_min,rtc_time.rtc_sec);
	Neoway_SetAlarm(&rtc_time,3);  //Set the alarm to be triggered after required time
	Neoway_Sleep(NEOWAY_TRUE);
}
/*****************************************************************************
 * Definition:   hextoascii
 * Parameter:   char *text
 * Return value:   void
 *****************************************************************************/
char* hextoascii(char *text)
{
	char convertion_buffer[50]={0};
	char  *ptr = text;
	int k=0;
	while ( *ptr )
	{
		char ch;
		int n=0;
		if ( sscanf(ptr, "%2x%n", &ch, &n) != 1 )
		{
			break;
		}
		ptr += n;
		convertion_buffer[k]=ch;
		k++;
	}
	convertion_buffer[k]='\0';
	// Neoway_Print("convertion_buffer = %s",convertion_buffer);
	memset(Rec_Msg, 0, sizeof(Rec_Msg));
	memcpy(Rec_Msg, convertion_buffer, strlen(convertion_buffer));
	// Neoway_Print("Rec_Msg = %s",Rec_Msg);
	ptr = NULL;
	return convertion_buffer;
}
/*****************************************************************************
 * Definition:   asctoint
 * Parameter:   char *str
 * Return value:   U32
 *****************************************************************************/
U32 asctoint(char *str) 
{ 
    U32 res=0,i=0;  
    for (i=0;str[i]!='\0';++i) 
        res = res*10+str[i]-'0';  
    return res; 
}
/*****************************************************************************
 * Definition:   strstr
 * Parameter:   char *str1, char *str2
 * Return value:   char
 *****************************************************************************/
char* strstr(char *str1, char *str2)
{
	 while(*str1 != '\0')
    {
        char *p = (char*)str1;
        char *q = (char*)str2;
        char *res = NULL;
        if(*p == *q)
        {
            res = p;
            while(*p && *q && *p++ == *q++);

            if(*q == '\0')
                return res;
        }
        str1++;
    }
    return NULL;
}
/*****************************************************************************
 * Definition:  extract_message
 * Parameter:   U8 *rec_msg_buf
 * Return value:   void
 *****************************************************************************/
void extract_message(char *rec_msg_buf)
{
	U16 i=0;
	U8  *ptr = NULL;
	memset(Rec_Mbl_No, 0, sizeof(Rec_Mbl_No));
	memset(Rec_Msg, 0, sizeof(Rec_Msg));
	// Neoway_Print("message_extract_function");
	if((ptr=strstr(rec_msg_buf,"+91")))
	{
		for(i=0;ptr[i]!='"';i++)
		{	
			Rec_Mbl_No[i]=ptr[i];
		}
	}
	Rec_Mbl_No[i]='\0';
	Neoway_Print("Rec_Mbl_No = %s",Rec_Mbl_No);
	ptr = NULL;
	if((ptr=strstr(rec_msg_buf,"\r")))
	{
		ptr += 2;
		hextoascii(ptr);
		Neoway_ms_delay(1);
		Neoway_Print("Rec_Msg = %s",Rec_Msg);
	}
	if(strcmp(Rec_Mbl_No, AMN)==0 || strcmp(Rec_Mbl_No, DMN)==0){
		MSG_EXTRCTN_CHECK = 1;
	}
	else{
		MSG_EXTRCTN_CHECK = 0;
		MSG_EXTRCTN = 0;
	}
	memset(rec_msg_buf, 0, sizeof(rec_msg_buf));
	ptr = NULL;
}
/*****************************************************************************
 * Definition:  check_in_msg_task
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void check_in_msg_task(void)
{
	U8  *ptr = NULL;
	Neoway_Print("enter to check_in_msg_task fun");
	if((ptr=strstr(Rec_Msg,"CAMN"))){
		cmd=CAMN;
	}		
	else if((ptr=strstr(Rec_Msg,"CDMN"))){
		cmd=CDMN;
	}
	else if((ptr=strstr(Rec_Msg,"CSPD"))){
		cmd=CSPD;
	}
	else if((ptr=strstr(Rec_Msg,"CSPH"))){
		cmd=CSPH;
	}
	else{
		cmd=NOCMD;
	}
	
	switch(cmd)
	{
		case CAMN:
			Neoway_Print("enter to case CAMN");	
			call_fun_change(Rec_Msg, 18);
			break;
		case CDMN:
			Neoway_Print("enter to case CDMN");
			call_fun_change(Rec_Msg, 15);
			break;
		case CSPD:
			Neoway_Print("enter to case CSPD");
			call_fun_change(Rec_Msg, 7);
			break;
		case CSPH:
			Neoway_Print("enter to case CSPH");
			call_fun_change(Rec_Msg, 7);
			break;
		case NOCMD:
			SEND_MSG = 1;
			Neoway_Print("enter to case NOCMD");	
			Neoway_Print("No Commands Match");
			memset(msg_data_changed,0,sizeof(msg_data_changed));
			strcpy(msg_data_changed,"Command Format Wrong!!! Please enter valid format\0");
			break;
		default :		
			break;
	}
	ptr = NULL;
}
/*****************************************************************************
 * Definition:  Neoway_ms_delay
 * Parameter:   U16 delay_time
 * Return value:   void
 *****************************************************************************/
void Neoway_ms_delay(U16 d)
{
	for( ; d != 0; d--){
		Neoway_us_delay(1000000);
	}
}
/*****************************************************************************
 * Definition:  call_fun_change
 * Parameter:   U8 *Rec_Msg, U16 sige
 * Return value:   void
 *****************************************************************************/
void call_fun_change(U8 *Rec_Msg, U16 sige)
{
	U8 *var= Rec_Msg;
	static U8 temp_data[20]={0}, data[20]={0}, MSGCMD[5]={0};
	U16 i=0,j=0;
	
	memset(temp_data, 0, sizeof(temp_data));
	memset(data, 0, sizeof(data));
	Neoway_Print("enter to call_fun_change");	
	Neoway_Print("var %s\r",var);
	// var+=5;
	strncpy(temp_data,var,sige);
	Neoway_Print("temp_data-> %s\r",temp_data);
	for(i=0;temp_data[i]!='\0';i++)
	{	
		if(i<=4){
			if(temp_data[i]>='A' && temp_data[i]<='Z'){
				MSGCMD[i]=temp_data[i];
				// Neoway_Print("i->%d MSGCMD[i]->%c\r",i,MSGCMD[i]);
			}
			else if(temp_data[i]>=' '){
				MSGCMD[i]='\0';
				// Neoway_Print("i->%d MSGCMD[i]->%c\r",i,MSGCMD[i]);
			}
			else{
			}
		}
		else if(i>=5){
			if(temp_data[i]>='0' && temp_data[i]<='9' || temp_data[i]<='+'){
				data[j]=temp_data[i];
				// Neoway_Print("i->%d data[i]->%c\r",i,data[j]);
				j++;
			}
			else{
			}
		}
	}
	data[j]='\0';
	Neoway_Print("msgcmd->%s  len->%d\r",MSGCMD,strlen(MSGCMD));
	Neoway_Print("data->%s  len->%d\r",data,strlen(data));
	if(strlen(data)==13 && strcmp(MSGCMD, "CAMN")==0)
	{
		//Neoway_Print("var %s",var);
		memset(authorised_mobile_no, 0, sizeof(authorised_mobile_no));
		strncpy(authorised_mobile_no,data,13);
		Neoway_Print("Recieve Authorised No.-> %s\r",authorised_mobile_no);	
		Neoway_Write_To_File(file_name_str1, authorised_mobile_no, 13);
		// Neoway_Read_File_CAMN();
		memset(msg_data_changed,0,sizeof(msg_data_changed));
		sprintf(msg_data_changed,"Authorised mobile Number Set to %s\0",authorised_mobile_no);
	}
	else if(strlen(data)==10 && strcmp(MSGCMD, "CDMN")==0)
	{
		memset(desti_mob_number, 0, sizeof(desti_mob_number));
		strncpy(desti_mob_number,data,10);							
		Neoway_Print("Recieve Destination No.-> %s\r",desti_mob_number);										
		Neoway_Write_To_File(file_name_str2, desti_mob_number, 10);
		// Neoway_Read_File_CDMN();
		memset(MSG_Send_Cmd,0,sizeof(MSG_Send_Cmd));
		sprintf(MSG_Send_Cmd,"at+cmgs=\"%s\"\r",desti_mob_number);
		memset(msg_data_changed,0,sizeof(msg_data_changed));
		sprintf(msg_data_changed,"Destination mobile Number Set to %s\0",desti_mob_number);
	}
	else if(strlen(data)==2 && strcmp(MSGCMD, "CSPD")==0)
	{
		memset(number_of_spd, 0, sizeof(number_of_spd));
		strncpy(number_of_spd,data,2);
		Neoway_Print("Recieve SMS per Day No.-> %s\r",number_of_spd);							
		Neoway_Write_To_File(file_name_str3, number_of_spd, 2);
		// Neoway_Read_File_CSPD();
		no_of_sms=asctoint(number_of_spd);
		memset(msg_data_changed,0,sizeof(msg_data_changed));
		sprintf(msg_data_changed,"Number of SMS/Day Set to %s\0",number_of_spd);
		TYPE_DAY = 1;	// to calculate new wakeup time
		TYPE_HOUR = 0;
		message_count = 0; // reset message count because SPD changed
		DEV_IN_SLEEP = 0;// SPD changed RTC Function Changed control to recall RTC Function
		fun_RTC_Change();
	}
	else if(strlen(data)==2 && strcmp(MSGCMD, "CSPH")==0)
	{
		memset(number_of_sph, 0, sizeof(number_of_sph));
		strncpy(number_of_sph,data,2);
		Neoway_Print("Recieve SMS per Hour No.-> %s\r",number_of_sph);							
		no_of_sms=asctoint(number_of_sph);
		memset(msg_data_changed,0,sizeof(msg_data_changed));
		sprintf(msg_data_changed,"Number of SMS/Hour Set to %s\0",number_of_sph);
		TYPE_HOUR = 1;// to calculate new wakeup time
		message_count = 0; // reset message count because SPH changed
		DEV_IN_SLEEP = 0;// SPD changed RTC Function Changed control to recall RTC Function
		fun_RTC_Change();
	}
	else	
	{
		memset(msg_data_changed,0,sizeof(msg_data_changed));
		if(strcmp(MSGCMD, "CAMN")==0){
			sprintf(msg_data_changed,"please enter correct format CAMN<space><+91mobilenumber>\0");
		}
		else if(strcmp(MSGCMD, "CDMN")==0){
			sprintf(msg_data_changed,"please enter correct format CDMN<space><mobilenumber>\0");
		}
		else if(strcmp(MSGCMD, "CSPD")==0){
			sprintf(msg_data_changed,"please enter correct format CSPD<space><2digitnumber>\0");
		}
		else if(strcmp(MSGCMD, "CSPH")==0){
			sprintf(msg_data_changed,"please enter correct format CSPH<space><2digitnumber>\0");
		}
		else{
			strcpy(msg_data_changed,"Command Format Wrong!!!\0");
		}
	}
	Neoway_Print("msg_data_changed: %s\r",msg_data_changed);
	SEND_MSG = 1;
	var=NULL;
}
/*****************************************************************************
 * Definition:  Read_fixed_file
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void Read_fixed_file(void)
{
	Neoway_Read_File(file_name_str1, authorised_mobile_no, 13);
	Neoway_Read_File(file_name_str2, desti_mob_number, 10);
	Neoway_Read_File(file_name_str3, number_of_spd, 2);
	no_of_sms=asctoint(number_of_spd);
	Neoway_Print("*********************************************");
	Neoway_Print("Authorised Mobile No: %s",authorised_mobile_no);
	Neoway_Print("Destination Mobile No: %s",desti_mob_number);
	Neoway_Print("Number of SMS per Day: %d",no_of_sms);
	Neoway_Print("*********************************************");
	TYPE_FIRST = 1;
	fun_RTC_Change();
}
/*****************************************************************************
 * Definition:  fun_LED_off
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void fun_LED_off(void)
{
	Neoway_GpioWrite(GREEN_LED_PIN,0);
	Neoway_GpioWrite(RED_LED_PIN,0);
	Neoway_GpioWrite(GREEN_LED_PIN,0);
	Neoway_GpioWrite(RED_LED_PIN,0);
}
/*****************************************************************************
 * Definition:  fun_RTC_Change
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void fun_RTC_Change(void)
{
	float temp_no_of_sms = (float)no_of_sms;
	
	Neoway_Print("Enter to fun_RTC_Change\r");
	
	if(TYPE_DAY == 1 || TYPE_FIRST == 1){
		TYPE_DAY = 0; TYPE_FIRST = 0;
		temp_H = (24/temp_no_of_sms);	
		temp_M = (((24/temp_no_of_sms)-(U32)temp_H)*60);	
		temp_S = (((((24/temp_no_of_sms)-(U32)temp_H)*60)-(U32)temp_M)*60);
	}
	else if(TYPE_HOUR == 1){
		// TYPE_HOUR = 0;
		temp_H = 0;
		temp_M = (60/temp_no_of_sms);	
		temp_S = (((60/temp_no_of_sms)-(U32)temp_M)*60);
	}
	
	Hour=(U32)temp_H;
	Min=(U32)temp_M;
	Sec=(U32)temp_S;
	
	// rtc_time.rtc_hour = Hour;
	// rtc_time.rtc_min = Min;
	// rtc_time.rtc_sec = Sec;
	
	Neoway_Print("wakeup timings %dH::%dM::%dS",Hour,Min,Sec);
	// Neoway_Print("Device wakeup after %dH::%dM::%dS",rtc_time.rtc_hour,rtc_time.rtc_min,rtc_time.rtc_sec);

}
/*****************************************************************************
 * Definition:   Reset_RTC_Fun
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void Reset_RTC_Fun(void)
{	
	Neoway_Print("Enter to Reset_RTC_Fun\r");
	if(message_count >= no_of_sms){
		Neoway_Print("neoway_RTC_task\r");
		Neoway_GetTime(&rtc_time);	//	get system time
		Neoway_Print("current time is %dH::%dM::%dS",rtc_time.rtc_hour,rtc_time.rtc_min,rtc_time.rtc_sec);
		if((rtc_time.rtc_hour == 23) && (rtc_time.rtc_min == 59)){
			message_count = 0;
		}
		else if(TYPE_HOUR == 1){
			TYPE_HOUR = 0;
			fun_LED_off();
			stop_timers();
			rtc_time.rtc_hour = 23;
			rtc_time.rtc_min = 59;
			rtc_time.rtc_sec = 59;
			Neoway_Print("Device wakeup after %dH::%dM::%dS",rtc_time.rtc_hour,rtc_time.rtc_min,rtc_time.rtc_sec);
			Neoway_SetAlarm(&rtc_time,3);  //Set the alarm to be triggered after required time
			Neoway_Sleep(NEOWAY_TRUE);
		}
		else{
			message_count = 0;
		}
	}
}

#endif