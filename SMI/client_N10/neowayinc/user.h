#ifdef __EXAMPLE_HELLWORLD__

#include "neoway_openplatform.h"
#include "file_system.h"

/**************************Function declaration****************************************/
extern void RTC_Function(void);
extern void extract_message(char *);
extern char* strstr(char *, char *);
extern void Neoway_ms_delay(U16 );
extern char* hextoascii(char *text);
extern void check_in_msg_task(void);
extern void call_fun_change(U8 *, U16);
extern void Read_fixed_file(void);
extern U32 asctoint(char *str);
extern void fun_LED_off(void);
extern void fun_RTC_Change(void);
void Reset_RTC_Fun(void);
/**************************************************************************************/
/**********************message cmd Declaration****************************************/
typedef enum{
	NOCMD=0,
	CAMN,
	CDMN,
	CSPD,
	CSPH
}Neo_msgcmd_enum;

extern Neo_msgcmd_enum cmd;
/**************************************************************************************/

/**********************RTC Variable Declaration****************************************/
extern NeowayRtcStruct  rtc_time;
extern U32 Hour,Min,Sec;
extern float temp_H,temp_M,temp_S;
/**************************************************************************************/
/**********************Messages Variable Declaration**********************************/
extern U8 message_string[50], Rec_Mbl_No[14], Rec_Msg[200], Local_msg_buf[1024], msg_buf[1024];
extern U32 no_of_sms;
/**************************************************************************************/
/*******************************control flags******************************************/
extern U16 MSG_EXTRCTN, MSG_EXTRCTN_CHECK, SEND_MSG, DEV_IN_SLEEP;
extern U16 RTC_TIME_CHANGE, TYPE_FIRST, TYPE_HOUR, TYPE_DAY;
/**************************************************************************************/
/*******************************file name buffer's**************************************/
extern U8 file_name_str1[10];
extern U8 file_name_str2[10];
extern U8 file_name_str3[10];
/**************************************************************************************/

/*******************************mobile no buffer's**************************************/
extern U8 authorised_mobile_no[14], desti_mob_number[11]; 
extern U8 number_of_spd[4], number_of_sph[4];
extern U8 msg_data_changed[160], MSG_Send_Command[30];
/**************************************************************************************/


#endif