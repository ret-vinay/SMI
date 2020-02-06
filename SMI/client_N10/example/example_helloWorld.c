#ifdef __EXAMPLE_HELLWORLD__

#include "neoway_openplatform.h"
#include "neoway_message.h"
#include "smi.h"
#include "fota.h"
#include "user.h"

/*******************************sim detect variable***************************/
U16 sim_detect= 0 ;
/*******************************control flags***************************/
U16 NOSIM_LED_FLAG=0, FIRST_TIME = 0, WAKEUP_FLAG = 0;
/*******************************UART Data Receive variable***************************/
U8 Uart_Buf[1024];
/*******************************data variables***************************/
float b_voltage,s_voltage;

/*****************************************************************************
 * Definition	:   Neoway_UserInit
 * Parameter	:   void
 * Return value	:   void
 *****************************************************************************/
void Neoway_UserInit(void) 
{
	Neoway_StopWatchdog();
	// Neoway_OpenUart1_toClient_Flag(0);
	Neo_Uart_Init();	
	Neo_gpio_Init();	
	Neo_AtInit();
	Neoway_MessageTimersInit();
	Neoway_StartTimer(&LED_Glow_Timer_Task3,216);	
}
/*****************************************************************************
 * Definition  	:   Neoway_UserTask1
 * Parameter  	:   NeowayMsgTypeStruct msg_type, NeowayModuleTypeEnum mod_type
 * Return value	:   void
 *****************************************************************************/
void Neoway_UserTask1(NeowayMsgTypeStruct msg_type,NeowayModuleTypeEnum mod_type)
{
	switch(msg_type.msg_id)
	{
		case NEOWAY_MSG_ID_TIMER_EXPIRY:
			{
				NeowayTimerId * current_time_id=Neoway_GetTimerId(msg_type);//Obtain the ID of the timer which times out.
				if(Neoway_IsTimeExpiredValid(current_time_id)!=NEOWAY_RET_OK)
				{
					break;
				}
				if(current_time_id==&neoway_at_stream)	//Initialize the AT commands for sending.
				{
					// Neoway_Print("neoway_at_stream running");
					sim_detect=Neoway_GpioRead(SIM_DET_PIN);
					if(sim_detect==1){
						Neo_SendNextAT();
					}
					else if(sim_detect==0){
						Neoway_Print("SIM Card is Absent\r");
						NOSIM_LED_FLAG = 1;
					}			
					Neoway_StartTimer(&neoway_at_stream,108);
				}
				else if (current_time_id==&neoway_to_send_at)
				{
					// Neoway_Print("neoway_to_send_at running");
					Neo_ATSendCmd();
					if(gNeo_send_at_info_state.total_item!=0)
						Neoway_StartTimer(&neoway_to_send_at,108);
				}
				else if (current_time_id==&neoway_at_send_overtime)
				{
					Neo_ATSendOvertime();
				}
			}
			break;
		case NEOWAY_FOTA_START_NOTIFY:
			update_available(); // check for Firmware Update over the Air
			break;
		case NEOWAY_FOTA_END_NOTIFY:	// end of Firmware Update over the Air
			FOTA_LED_FLAG = 0;			
			Read_fixed_file();// call file read function here from file system
			Neoway_ms_delay(1);
			memset(MSG_Send_Cmd,0,sizeof(MSG_Send_Cmd));
			sprintf(MSG_Send_Cmd,"at+cmgs=\"%s\"\r",desti_mob_number);
			// Neoway_Print("MSG_Send_Cmd-> %s\r",MSG_Send_Cmd);
			Neoway_StartTimer(&neoway_at_stream,21);// Start to send AT commands.
			break;
		case NEOWAY_MSG_ID_DATA_SEND_REQUIRE:
			gNeo_MSG_send_count--;
			Neo_ATSendData();
			break;
		case NEOWAY_MSG_ID_AT_SEND_OK:
			Neo_ATSendOK();
			break;
		case NEOWAY_MSG_ID_AT_SEND_ERROR:
			Neo_ATSendError();
			break;
		default :
			break;
	}
}
/*****************************************************************************
 * Definition  	:   Neoway_UserTask2
 * Parameter  	:   NeowayMsgTypeStruct msg_type, NeowayModuleTypeEnum mod_type
 * Return value	:   void
 *****************************************************************************/
void Neoway_UserTask2(NeowayMsgTypeStruct msg_type,NeowayModuleTypeEnum mod_type)
{
	switch(msg_type.msg_id)
	{
		case NEOWAY_MSG_ID_TIMER_EXPIRY:
			{	
				NeowayTimerId * current_time_id=Neoway_GetTimerId(msg_type);
				if(current_time_id==&neoway_msg_send_timer_task2)
				{
					if(Neoway_IsTimeExpiredValid(&neoway_msg_send_timer_task2)==NEOWAY_RET_OK)
					{
						memset(message,0,sizeof(message));
						moisture_cmp(b_voltage, s_voltage, message);
						if(FIRST_TIME == 0){
							FIRST_TIME = 1;
							strcat(message_data, message);
							memset(message, 0, sizeof(message));
							memcpy(message, message_data, strlen(message_data));
							gNeo_at_init_state_machine1=SM_MSG_CSCS_SEND_OK;
						}
						else if(WAKEUP_FLAG == 1){
							WAKEUP_FLAG = 0;
							gNeo_at_init_state_machine1=SM_MSG_CSCS_SEND_OK;
						}
						else{
						}
						Neoway_Print("%s\r",message);
						// gNeo_at_init_state_machine1=SM_MSG_CSCS_SEND_OK;
						// gNeo_at_init_state_machine1=SM_MSG_IDLE;
					}
				}
			}
			break;
		case NEOWAY_MSG_ID_RTC_WAKEUP:
			Neoway_Sleep(NEOWAY_FALSE);	//  Exit from the sleep mode.
			Neoway_Sleep(NEOWAY_FALSE);	//  Exit from the sleep mode.
			WAKEUP_FLAG = 1;
			DEV_IN_SLEEP = 0;
			Neoway_Print("Device wakeup!!!");
			Neoway_GpioWrite(RED_LED_PIN,1);
			start_timer();
			gNeo_at_init_state_machine1=SM_MSG_IDLE;
			// Neoway_SendMsgTask(NEOWAY_MOD_USER2,NEOWAY_MOD_USER2,NEOWAY_MSG_READ_SENSORS,NULL,0);
			break;
		case NEOWAY_MSG_READ_SENSORS:
			b_voltage = GET_BAT_VOLTAGE();
			s_voltage = moisture_sense();
			// Neoway_Print("stops working"); // to check message part commented bellow line
			Neoway_StartTimer(&neoway_msg_send_timer_task2,216);
			break;
		case NEOWAY_MSG_ID_MSG_SENT:		
			Neoway_ms_delay(3);
			fun_LED_off();
			stop_timers();
			if(DEV_IN_SLEEP == 0){
				RTC_Function();
			}
			else{
				Neoway_Print("Device Already In Sleep Mode");
			}
			break;
		case NEOWAY_MSG_ID_MSG_RECEIVED:
			Neoway_Print("receive_msg_buf = %s",receive_msg_buf);
			if(MSG_EXTRCTN == 0){
				MSG_EXTRCTN = 1;
				memset(Local_msg_buf, 0, sizeof(Local_msg_buf));
				memcpy(Local_msg_buf, receive_msg_buf, strlen(receive_msg_buf));
				memset(receive_msg_buf, 0, sizeof(receive_msg_buf));
				memset(receive_msg_buf, 0, sizeof(receive_msg_buf));
				// Neoway_Print("receive_msg_buf = %s",receive_msg_buf);
				extract_message(Local_msg_buf);
				Neoway_ms_delay(2);
				if(MSG_EXTRCTN_CHECK == 1){
					Neoway_SendMsgTask(NEOWAY_MOD_USER2,NEOWAY_MOD_USER2,NEOWAY_MSG_ID_MSG_CHECK,NULL,0);
				}
				else{
					Neoway_Print("User was not Authorised");
				}			
			}
			else{
			}
			break;
		case NEOWAY_MSG_ID_MSG_CHECK:
			if(MSG_EXTRCTN == 1){
				check_in_msg_task();
				Neoway_ms_delay(3);
				MSG_EXTRCTN = 0;
				MSG_EXTRCTN_CHECK = 0;
			}
			else{
				Neoway_Print("Message Extraction Not Completed");
			}
			if(SEND_MSG == 1)
				Neoway_SendMsgTask(NEOWAY_MOD_USER2,NEOWAY_MOD_USER2,NEOWAY_MSG_ID_MSG_SEND,NULL,0);
			else{
			}
			break;
		case NEOWAY_MSG_ID_MSG_SEND:
			// memset(MSG_Send_Cmd,0,sizeof(MSG_Send_Cmd));
			// sprintf(MSG_Send_Cmd,"at+cmgs=\"%s\"\r",desti_mob_number);
			memset(message, 0, sizeof(message));
			memcpy(message, msg_data_changed, strlen(msg_data_changed));
			message_count--;
			Neoway_Print("message_count %d",message_count);
			if(message_count < 0){
				message_count = 0;
				Neoway_Print("message_count %d",message_count);
			}
			else{
			}
			gNeo_at_init_state_machine1=SM_MSG_CSCS_SEND_OK;
			start_timer();
			break;
		default :
			break;
	}
}
/*****************************************************************************
 * Definition: Neoway_UserTask3
 * Parameter: NeowayMsgTypeStruct msg_type,NeowayModuleTypeEnum mod_type
 * Return value: void
 * Comments: Running USERTASK3 for sending the AT commands
 *****************************************************************************/
void Neoway_UserTask3(NeowayMsgTypeStruct msg_type,NeowayModuleTypeEnum mod_type)
{
	// Neoway_Print("Neoway_UserTask3 running");
	switch(msg_type.msg_id)
	{
		case NEOWAY_MSG_ID_TIMER_EXPIRY:
			{
				NeowayTimerId * current_time_id=Neoway_GetTimerId(msg_type);//Obtain the ID of the timer which times out.
				/**************check the current timer valid or not***************/
				if(current_time_id==&LED_Glow_Timer_Task3)
				{
					if(Neoway_IsTimeExpiredValid(&LED_Glow_Timer_Task3)==NEOWAY_RET_OK)
					{
						/* Neoway_Print("****LED_Glow_Timer_Task3 Running****");*/
						if(GSM_CONNECTED == 1){
							Neoway_GpioWrite(GREEN_LED_PIN,1);
						}
						else {
						}
						if(DATA_SENDING_FLAG == 1){
							Neoway_GpioWrite(GREEN_LED_PIN,1);
							Neoway_us_delay(100000);
							Neoway_GpioWrite(GREEN_LED_PIN,0);
							Neoway_us_delay(100000);
						}
						else {
						}
						if(FOTA_LED_FLAG == 1){
							Neoway_GpioWrite(RED_LED_PIN,1);
							Neoway_GpioWrite(GREEN_LED_PIN,1);
							Neoway_us_delay(100000);
							Neoway_GpioWrite(GREEN_LED_PIN,0);
							Neoway_GpioWrite(RED_LED_PIN,0);
							Neoway_us_delay(100000);
						}
						else {
						}
						if(BAT_LOW_FLAG == BAT_LOW){
							Neoway_GpioWrite(RED_LED_PIN,1);
							Neoway_us_delay(100000);
							Neoway_GpioWrite(RED_LED_PIN,0);
							Neoway_us_delay(100000);
						}
						else {
							Neoway_GpioWrite(RED_LED_PIN,1);
						}	
					}
					if(DEV_IN_SLEEP == 0 || SEND_MSG == 1){
						Neoway_StartTimer(&LED_Glow_Timer_Task3,20);	
					}
					else{
						Neoway_StopTimer(&LED_Glow_Timer_Task3);
						Neoway_Print("Device in Sleep mode cant callback LED Timer Task3");
					}					
				}
				break;
			}
		default :
			break;
	}
}
/*****************************************************************************
 * Definition  	:   Neoway_UartReceive
 * Parameter  	:   NeowayUartPortEnum port,U8 *buffer,U16 lenth, NeowayModuleTypeEnum mod_id
 * Return value	:   void
 *****************************************************************************/
void Neoway_UartReceive(NeowayUartPortEnum port,U8 *buffer,U16 lenth, NeowayModuleTypeEnum mod_id)
{
	memset(Uart_Buf, 0, sizeof(Uart_Buf));
	memcpy(Uart_Buf, buffer, lenth);
	// Neoway_UartSend(NEOWAY_UART_PORT_1,Uart_Buf,strlen(Uart_Buf));	
}
/*****************************************************************************
 * Definition  	:   Neoway_VirtualUartReceive
 * Parameter  	:   U8 *buff_ptr, U16 len, NeowayModuleTypeEnum mod_id
 * Return value	:   void
 *****************************************************************************/
void Neoway_VirtualUartReceive(U8 *buff_ptr, U16 len, NeowayModuleTypeEnum mod_id)
{
	if(memcmp(buff_ptr+2,"+CALV",5)==0)
	{
		Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER2,NEOWAY_MSG_ID_RTC_WAKEUP,NULL,0);
	}
	else{
		Neo_AT_ReceiveManage(buff_ptr,len,mod_id);// Process the UART receive function.
	}
}
/*****************************************************************************
 * Definition  	:   Neoway_RegisterCallbackFunction
 * Parameter  	:   void
 * Return value	:   void
 *****************************************************************************/
void Neoway_RegisterCallbackFunction(void)
{
	Neoway_RegisterCallBack(NEOWAY_KB_ID_USER_TASK_1,(U32)Neoway_UserTask1);
	Neoway_RegisterCallBack(NEOWAY_KB_ID_USER_TASK_2,(U32)Neoway_UserTask2);
	Neoway_RegisterCallBack(NEOWAY_KB_ID_USER_TASK_3,(U32)Neoway_UserTask3);
	Neoway_RegisterCallBack(NEOWAY_KB_ID_UART_RECEIVE,(U32)Neoway_UartReceive);
	Neoway_RegisterCallBack(NEOWAY_KB_ID_VIRTUAL_UART_RECEIVE,(U32)Neoway_VirtualUartReceive);
}

#endif