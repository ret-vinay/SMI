#ifdef __EXAMPLE_HELLWORLD__

#include "neoway_openplatform.h"
#include "neoway_message.h"
#include "fota.h"
#include "user.h"

/**************************Timer ID's*****************************************/
NeowayTimerId neoway_msg_send_timer_task2;
NeowayTimerId LED_Glow_Timer_Task3;
NeowayTimerId neoway_at_stream;
NeowayTimerId neoway_to_send_at;//Send AT commands in the queue.
NeowayTimerId neoway_at_send_overtime;
/******************************************************************************/

Neo_Send_At_Info gNeo_send_at_info_link[20];//Send SMS. Making call through TCP data is in the saving queue.
NeowaySendAtInfoState gNeo_send_at_info_state;
Neo_Send_At_Info gNeo_at_info={0};

NeowayATInitStateMachineEnum gNeo_at_init_state_machine=SM_IDLE;// When SM_INIT_OK is printed, AT initialization is complete. Other commands can be sent now.
NeowayATInitStateMachineEnum gNeo_at_init_state_machine1=SM_MSG_INIT_OK;
NeowayGSMLedStateMachineEnum gNeo_gsm_state_machine;

NeowayGPRSStateMachineEnum gNeo_gprs0_state_machine=SM_GPRS_INIT_OK;// SM_GPRS_IDLE Channel 0
NeowayGPRSStateMachineEnum gNeo_gprs0_dest_state_machine=SM_GPRS_INIT_OK;
NeowayGPRSStateMachineEnum gNeo_gprs1_state_machine=SM_GPRS_IDLE;//Channel 1
NeowayGPRSStateMachineEnum gNeo_gprs1_dest_state_machine=SM_GPRS_IDLE;

unsigned char gNeo_register_state=0;
unsigned char gNeo_signal_strong=0;
char gNeo_imei_str[16]={0};
char gNeo_local_ip[16]="0.0.0.0";
 
S8 send_at_command[NEO_MAX_SEND_AT_LEN];
S8 send_at_data[NEO_MAX_SEND_DATA_LEN];
U8 message[160], receive_msg_buf[1024]={0}, message_data[160] ="Device ON ";
U8 MSG_Send_Cmd[30];

unsigned char gNeo_MSG_send_count=0;//Count messages sent to prevent the queue from overflow.
Neo_AtResponseDistill gNeo_at_response_parser=
{
	{0},
	0,
	0,
	0,
	0
};

/*************************Function quotation***************************/
void Neoway_MessageTimersInit(void);
void Neo_SearchAtResponse(U8 *Buffaddr, U16 Length, NeowayModuleTypeEnum mod_id);
void Neo_AT_ReceiveManage(U8 *buff_ptr, U16 len, NeowayModuleTypeEnum mod_id);
void Neo_AtInit(void);
NeowayBoolEnum Neo_SendAT(NeowayCmdEnum cmd,S8* send_str,U32 str_length,S8* send_data,U32 data_length);
void Neo_ATSendCmd(void);
void Neo_ATSendData(void);
void Neo_ATSendError(void);
void Neo_ATSendOvertime(void);
void Neo_ATSendOK(void);
void Neo_SendNextAT(void);
void stop_timers(void);
void start_timer(void);
/*****************Print the global varables*******************/
BOOL	gNeo_isWatchDog = FALSE;
BOOL	gNeo_isTcplinkOk = FALSE;

/*******************************LED control flags***************************/
U16 GSM_CONNECTED=0,DATA_SENDING_FLAG=0;
S32 message_count = 0;
/***************************************************************************/

void Neoway_MessageTimersInit(void)
{
	Neoway_InitTimer(&neoway_at_stream,NEOWAY_MOD_USER1);
	Neoway_InitTimer(&neoway_to_send_at, NEOWAY_MOD_USER1);
	Neoway_InitTimer(&neoway_at_send_overtime, NEOWAY_MOD_USER1);
	Neoway_InitTimer(&neoway_msg_send_timer_task2,NEOWAY_MOD_USER2);
	Neoway_InitTimer(&LED_Glow_Timer_Task3,NEOWAY_MOD_USER3);
}


void Neo_SearchAtResponse(U8 *Buffaddr, U16 Length, NeowayModuleTypeEnum mod_id)
{	
	if(Length==0||Length==1)
	{
		return ;
	}
	else if(memcmp(&Buffaddr[0],"MODEM:STARTUP",13)==0)
	{
		// Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER1,NEOWAY_FOTA_START_NOTIFY,NULL,0);
		Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER1,NEOWAY_FOTA_END_NOTIFY,NULL,0);
	}
	else if(memcmp(&Buffaddr[0],"OK",2)==0)
	{
		Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER1,NEOWAY_MSG_ID_AT_SEND_OK,NULL,0);
	}
	else if(memcmp(&Buffaddr[0],"ERROR",5)==0)
	{
		Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER1,NEOWAY_MSG_ID_AT_SEND_ERROR,NULL,0);
	}
	else if(memcmp(&Buffaddr[0],"+CSQ",4)==0)
	{
		if(Buffaddr[7]==',')//One bit
		{
			gNeo_signal_strong=Buffaddr[6]-0x30;
		}
		else if(Buffaddr[6]=='9')
		{
			gNeo_signal_strong=0;
		}
		else
		{
			gNeo_signal_strong=(Buffaddr[6]-0x30)*10+(Buffaddr[7]-0x30);
		}
	}
	else if(memcmp(&Buffaddr[0],"+CREG",5)==0)
	{
		if(Buffaddr[9]=='1'||Buffaddr[9]=='5')
		{
			gNeo_register_state=Buffaddr[9]-0x30;
		}
		else
		{
			gNeo_register_state=0;
		}
	}
	else if(memcmp(&Buffaddr[0],"+XIIC",5)==0)
	{
	}
	else if(memcmp(&Buffaddr[0],"+TCPCLOSE",9)==0)
	{
	}
	else if(memcmp(&Buffaddr[0],"+TCPSETUP",9)==0)
	{
	}
	else if(memcmp(&Buffaddr[0],"+TCPSEND",8)==0)
	{
	}
	else if(memcmp(&Buffaddr[0],"GPRS DISCONNECTION",18)==0)
	{
	}
	else if(memcmp(&Buffaddr[0],"+CMT:",5)==0)
	{
		U16 i=0;
		memset(receive_msg_buf, 0, sizeof(receive_msg_buf));
		for(i=0;i<strlen(Buffaddr);i++){
			receive_msg_buf[i] = Buffaddr[i];
		}
		// memcpy(receive_msg_buf, Buffaddr, strlen(Buffaddr));
		Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER2,NEOWAY_MSG_ID_MSG_RECEIVED,NULL,0);
		memset(&Buffaddr, 0, sizeof(&Buffaddr));memset(&Buffaddr, 0, sizeof(&Buffaddr));
	}
	Buffaddr=NULL;
	Length=0;
}

void Neo_AT_ReceiveManage(U8 *buff_ptr, U16 len, NeowayModuleTypeEnum mod_id)
{
	//Process the UART receive function.
	U16 i;
	//	Neoway_UartSend(0,(U8*)&mod_id,2);
	memcpy(&gNeo_at_response_parser.buffer[gNeo_at_response_parser.last_lenth],buff_ptr,len);
	len=len+gNeo_at_response_parser.last_lenth;
	
	for(i=gNeo_at_response_parser.last_lenth;i<len;i++)
	{
		if(gNeo_at_response_parser.buffer[i]=='>')
		{
			if(gNeo_MSG_send_count<=1)
			{
				gNeo_MSG_send_count++;
				Neoway_SendMsgTask(mod_id,NEOWAY_MOD_USER1,NEOWAY_MSG_ID_DATA_SEND_REQUIRE,NULL,0);
			}
			gNeo_at_response_parser.tag_num=0;
			gNeo_at_response_parser.column++;
		}
		else if(gNeo_at_response_parser.buffer[i]==0x0D)
		{
			gNeo_at_response_parser.tag_num=1;
			gNeo_at_response_parser.column++;
		}
		else if(gNeo_at_response_parser.buffer[i]==0x0A)
		{
			if(gNeo_at_response_parser.tag_num==1)//0d 0a
			{
				Neo_SearchAtResponse(&gNeo_at_response_parser.buffer[gNeo_at_response_parser.start_position],gNeo_at_response_parser.column-1, mod_id);
				gNeo_at_response_parser.column=0;
				gNeo_at_response_parser.start_position=i+1;
				gNeo_at_response_parser.tag_num=0;				
			}
			else
			{
				gNeo_at_response_parser.column++;
			}
		}
		else
		{
			gNeo_at_response_parser.tag_num=0;
			gNeo_at_response_parser.column++;
		}	
	}
	gNeo_at_response_parser.last_lenth=len-gNeo_at_response_parser.start_position;
	memcpy(&gNeo_at_response_parser.buffer[0],&gNeo_at_response_parser.buffer[gNeo_at_response_parser.start_position],gNeo_at_response_parser.last_lenth);
	gNeo_at_response_parser.start_position=0;
}

void Neo_AtInit(void)
{
//	memset(&gNeo_send_at_info_state,0,sizeof(gNeo_send_at_info_state));
//	memset(&gNeo_send_at_info_link,0,sizeof(gNeo_send_at_info_link));
	gNeo_send_at_info_state.head_position=&gNeo_send_at_info_link[0];
	gNeo_send_at_info_state.end_position=&gNeo_send_at_info_link[0];
	gNeo_send_at_info_state.total_item=0;
}

NeowayBoolEnum Neo_SendAT(NeowayCmdEnum cmd,S8* send_str,U32 str_length,S8* send_data,U32 data_length)
{

	// Neoway_Print("total item = %d AT:%s", gNeo_send_at_info_state.total_item,send_str);
	if(gNeo_send_at_info_state.total_item >= 20)
	{
		return NEOWAY_FALSE;
	}
	
	memset(gNeo_send_at_info_state.end_position,0,sizeof(Neo_Send_At_Info));
	gNeo_send_at_info_state.end_position->cmd=cmd;
	gNeo_send_at_info_state.end_position->at_send_state=AT_IDLE;
	gNeo_send_at_info_state.end_position->str_length = str_length;
	memcpy(gNeo_send_at_info_state.end_position->send_str,send_str,str_length);
//	Neoway_Print("end_position %s", gNeo_send_at_info_state.end_position->send_str);

	if(data_length==0)
	{
		gNeo_send_at_info_state.end_position->send_data[0]=0;//=NULL;
		gNeo_send_at_info_state.end_position->data_length=data_length;
	}
	else
	{
		gNeo_send_at_info_state.end_position->data_length=data_length;
		memcpy(gNeo_send_at_info_state.end_position->send_data,send_data,data_length);
		gNeo_send_at_info_state.end_position->send_data[data_length]=0;
	}
	if(gNeo_send_at_info_state.end_position == (&gNeo_send_at_info_link[19]))
	{
		gNeo_send_at_info_state.end_position = &gNeo_send_at_info_link[0];
	}
	else
	{
		gNeo_send_at_info_state.end_position++;
	}
	gNeo_send_at_info_state.total_item++;
	
	if(gNeo_send_at_info_state.total_item==1)
		Neoway_StartTimer(&neoway_to_send_at,21);

	return NEOWAY_TRUE;
}

void Neo_ATSendCmd(void)
{
	if(gNeo_at_info.at_send_state!=AT_IDLE)//Sending AT commands
	{//Exit directly
		return;
	}
	else if(gNeo_send_at_info_state.total_item!=0)
	{
		memcpy(&gNeo_at_info,gNeo_send_at_info_state.head_position,sizeof(Neo_Send_At_Info));
		gNeo_send_at_info_state.total_item--;
		if(gNeo_send_at_info_state.head_position == (&gNeo_send_at_info_link[19]))
		{
			gNeo_send_at_info_state.head_position = &gNeo_send_at_info_link[0];
		}
		else
		{
			gNeo_send_at_info_state.head_position++;
		}
		switch (gNeo_at_info.cmd)
		{
			case CMD_CMGS:
			case CMD_AT:
			case CMD_INIT:
				gNeo_at_info.at_send_state=AT_SENDING;//Sending AT commands
				Neoway_StartTimer(&neoway_at_send_overtime,1080);//Sending times out after 5 seconds.
				Neoway_VirtualUartSend(NEOWAY_MOD_USER1,gNeo_at_info.send_str,gNeo_at_info.str_length);
				break;
			default :
				gNeo_at_info.cmd=CMD_NULL;
				break;
		}
		return ;
	}
}

void Neo_ATSendData(void)
{
	if(gNeo_at_info.at_send_state==AT_SENDING&&(gNeo_at_info.cmd==CMD_CMGS))
	{
		Neoway_StopTimer(&neoway_at_send_overtime);
		Neoway_StartTimer(&neoway_at_send_overtime,10800);//Sending times out after 50 seconds.
		gNeo_at_info.at_send_state=AT_SENDING_DATA;
		Neoway_VirtualUartSend(NEOWAY_MOD_USER1,gNeo_at_info.send_data,gNeo_at_info.data_length);		
	}
}

void Neo_ATSendError(void)
{
	if(gNeo_at_info.at_send_state==AT_IDLE||gNeo_at_info.cmd==CMD_NULL)
	{
		return ;
	}
	if(gNeo_at_init_state_machine!=SM_INIT_OK && gNeo_at_info.cmd == CMD_INIT)//Initializing. Forbidden sending other commands.
	{
		Neoway_StopTimer(&neoway_at_send_overtime);
		gNeo_at_info.at_send_state=AT_IDLE;
		switch (gNeo_at_init_state_machine)
		{
			case SM_IDLE:
				break;
			case SM_AT_SENDING:
				gNeo_at_init_state_machine=SM_IDLE;
				break;
			case SM_CREG_SENDING:
				gNeo_at_init_state_machine=SM_AT_SEND_OK;
				break;
			case SM_CSQ_SENDING:
				gNeo_at_init_state_machine=SM_CREG_SEND_OK;
				break;
			case SM_GLTS_SENDING:
				gNeo_at_init_state_machine=SM_CSQ_SEND_OK;
				break;
			case SM_CSCS_SENDING:
				gNeo_at_init_state_machine=SM_GLTS_SEND_OK;
				break;
			case SM_CNMI_SENDING:
				gNeo_at_init_state_machine=SM_CSCS_SEND_OK;
				break;
			case SM_XIIC_SENDING:
				gNeo_at_init_state_machine=SM_CNMI_SEND_OK;
				break;
		
			default :
				break;
		}
		return ;
	}
	if(gNeo_at_init_state_machine1!=SM_MSG_INIT_OK)
	{
		Neoway_StopTimer(&neoway_at_send_overtime);
		gNeo_at_info.at_send_state=AT_IDLE;
		switch (gNeo_at_init_state_machine1)
		{
			case SM_MSG_IDLE:
				break;
			case SM_MSG_CSQ_SENDING:
				gNeo_at_init_state_machine1=SM_MSG_IDLE;
				break;
			case SM_MSG_CMGF_SENDING:
				gNeo_at_init_state_machine1=SM_MSG_CSQ_SEND_OK; 
				break;
			case SM_MSG_CSCS_SENDING:
				gNeo_at_init_state_machine1=SM_MSG_CMGF_SEND_OK;
				break;
			case SM_MSG_CMGS_SENDING:
				DATA_SENDING_FLAG = 0;// for control LED blinking while data sending
				gNeo_at_init_state_machine1=SM_MSG_CSCS_SEND_OK;
				break;
			case SM_MSG_CSCS_GSM_SENDING:
				gNeo_at_init_state_machine1=SM_MSG_CMGS_SEND_OK;
				break;
			default :
				break;
		}
		return ;
	}
	switch (gNeo_at_info.cmd)
	{
		case CMD_INIT:
		case CMD_CMGS:
		case CMD_AT:
			Neoway_StopTimer(&neoway_at_send_overtime);
			gNeo_at_info.at_send_state=AT_IDLE;
			break;
		default :
			break;
	}
	return ;
}

void Neo_ATSendOvertime(void)
{
	Neo_ATSendError();
}

void Neo_ATSendOK(void)
{
	if(gNeo_at_info.at_send_state==AT_IDLE)//AT command is not sent.
	{//Exit directly
		return ;
	}
	if(gNeo_at_info.cmd!=CMD_NULL)
	{
		if(gNeo_at_init_state_machine!=SM_INIT_OK && gNeo_at_info.cmd == CMD_INIT)//Initializing. Forbidden sending other commands.
		{
			Neoway_StopTimer(&neoway_at_send_overtime);
			gNeo_at_info.at_send_state=AT_IDLE;
			switch (gNeo_at_init_state_machine)
			{
				case SM_IDLE:
					break;
				case SM_AT_SENDING:
					gNeo_at_init_state_machine=SM_AT_SEND_OK;
					break;
				case SM_CREG_SENDING:
					if(gNeo_register_state==0)
					{
						gNeo_at_init_state_machine=SM_AT_SEND_OK;
					}
					else
					{
						gNeo_at_init_state_machine=SM_CREG_SEND_OK;
					}
					break;
				case SM_CSQ_SENDING:
					if(gNeo_signal_strong<5)
					{
						gNeo_at_init_state_machine=SM_CREG_SEND_OK;
						GSM_CONNECTED = 0;// for control LED blinking when gsm connected
					}
					else
					{
						gNeo_at_init_state_machine=SM_CSQ_SEND_OK;
						GSM_CONNECTED = 1;// for control LED blinking when gsm connected
					}
					break;
				case SM_GLTS_SENDING:
					gNeo_at_init_state_machine=SM_GLTS_SEND_OK;
					break;
				case SM_CSCS_SENDING:
					gNeo_at_init_state_machine=SM_CSCS_SEND_OK;
					break;
				case SM_CNMI_SENDING:
					gNeo_at_init_state_machine=SM_CNMI_SEND_OK;
					break;
				case SM_XIIC_SENDING:
					gNeo_at_init_state_machine=SM_XIIC_SEND_OK;
					// gNeo_at_init_state_machine=SM_INIT_OK;
					gNeo_at_init_state_machine1=SM_MSG_IDLE;
					Neoway_UartSend(NEOWAY_UART_PORT_1,"SM_INIT_OK\r",strlen("SM_INIT_OK\r"));				
					break;
				default :
					break;
			}
			return ;
		}
		if(gNeo_at_init_state_machine1!=SM_MSG_INIT_OK)
		{
			Neoway_StopTimer(&neoway_at_send_overtime);
			gNeo_at_info.at_send_state=AT_IDLE;
			switch (gNeo_at_init_state_machine1)
			{
				case SM_MSG_IDLE:
					break;
				case SM_MSG_CSQ_SENDING:
					if(gNeo_signal_strong<5)
					{
						gNeo_at_init_state_machine1=SM_MSG_IDLE;
						GSM_CONNECTED = 0;// for control LED blinking when gsm connected
					}
					else
					{
						gNeo_at_init_state_machine1=SM_MSG_CSQ_SEND_OK;
						GSM_CONNECTED = 1; // for control LED blinking when gsm connected
					}
					break;
				case SM_MSG_CMGF_SENDING:
					gNeo_at_init_state_machine1=SM_MSG_CMGF_SEND_OK; 
					break;
				case SM_MSG_CSCS_SENDING:
					gNeo_at_init_state_machine1=SM_MSG_CSCS_SEND_OK;
					gNeo_at_init_state_machine1= SM_MSG_INIT_OK;
					Neoway_Print("SM_MSG_INIT_OK");
					Neoway_SendMsgTask(NEOWAY_MOD_USER1,NEOWAY_MOD_USER2,NEOWAY_MSG_READ_SENSORS,NULL,0);
					break;
				case SM_MSG_CMGS_SENDING:
					gNeo_at_init_state_machine1=SM_MSG_CMGS_SEND_OK;
					message_count += 1;
					Neoway_Print("message_count = %d",message_count);
					Neoway_Print("SM_MSG_SENT_OK");
					DATA_SENDING_FLAG = 0;// for control LED blinking while data sending
					gNeo_at_init_state_machine1=SM_MSG_INIT_OK;
					Neoway_SendMsgTask(NEOWAY_MOD_USER1,NEOWAY_MOD_USER2,NEOWAY_MSG_ID_MSG_SENT,NULL,0);										
					break;
				// case SM_MSG_CSCS_GSM_SENDING:
					// gNeo_at_init_state_machine1=SM_MSG_CSCS_GSM_SEND_OK;
					// gNeo_at_init_state_machine1=SM_MSG_INIT_OK;
					// Neoway_SendMsgTask(NEOWAY_MOD_USER1,NEOWAY_MOD_USER2,NEOWAY_MSG_ID_MSG_SENT,NULL,0);										
					// break;
				default :
					break;
			}
			return ;
		}		
		switch (gNeo_at_info.cmd)
		{
			case CMD_CMGS:
				if(gNeo_at_info.at_send_state==AT_SENDING_DATA)
				{
					Neoway_StopTimer(&neoway_at_send_overtime);
					gNeo_at_info.at_send_state=AT_IDLE;
				}
				break;
			case CMD_AT:
			case CMD_INIT:
				Neoway_StopTimer(&neoway_at_send_overtime);
				gNeo_at_info.at_send_state=AT_IDLE;
				break;
			default :
				break;
		}
		return ;
	}
}

void Neo_SendNextAT(void)
{
	if(gNeo_send_at_info_state.total_item!=0||gNeo_at_info.at_send_state!=AT_IDLE)
	{//Busy in initializing AT
		return ;
	}//AT is idle.
	if(gNeo_at_init_state_machine!=SM_INIT_OK)
	{
		switch (gNeo_at_init_state_machine)
		{
			case SM_IDLE:
				gNeo_at_init_state_machine=SM_AT_SENDING;
				Neo_SendAT(CMD_INIT, "at\r",3, NULL, 0);
				break;
			case SM_AT_SEND_OK:
				gNeo_at_init_state_machine=SM_CREG_SENDING;
				Neo_SendAT(CMD_INIT, "at+creg\?\r",9, NULL, 0);
				break;
			case SM_CREG_SEND_OK:
				gNeo_at_init_state_machine=SM_CSQ_SENDING;
				Neo_SendAT(CMD_INIT, "at+csq\r",7, NULL, 0);
				break;
			case SM_CSQ_SEND_OK:
				gNeo_at_init_state_machine=SM_GLTS_SENDING;
				Neo_SendAT(CMD_INIT, "at+GLTS=1\r",10, NULL, 0);
				break;
			case SM_GLTS_SEND_OK:
				gNeo_at_init_state_machine=SM_CSCS_SENDING; 		  
				Neo_SendAT(CMD_INIT, "at+cscs=\"HEX\"\r",14, NULL, 0);// they used "at+cscs=\"HEX\"\r" because to send message in HEX format.
				break;
			case SM_CSCS_SEND_OK:
				gNeo_at_init_state_machine=SM_CNMI_SENDING;
				Neo_SendAT(CMD_INIT, "at+cnmi=2,2,0,0,0\r",18, NULL, 0);
				break;
			case SM_CNMI_SEND_OK:
				gNeo_at_init_state_machine=SM_XIIC_SENDING;
				Neo_SendAT(CMD_INIT, "at+xiic=1\r",10, NULL, 0);
				break;
			case SM_XIIC_SEND_OK:
				gNeo_at_init_state_machine=SM_INIT_OK;				
				break;
			default :
				break;
		}
		return ;
	}
	if(gNeo_at_init_state_machine1!=SM_MSG_INIT_OK)
	{
		switch (gNeo_at_init_state_machine1)
		{
			case SM_MSG_IDLE:
				gNeo_at_init_state_machine1=SM_MSG_CSQ_SENDING;
				Neo_SendAT(CMD_INIT, "at+csq\r",7, NULL, 0);
				break;
			case SM_MSG_CSQ_SEND_OK:
				gNeo_at_init_state_machine1=SM_MSG_CMGF_SENDING;
				Neo_SendAT(CMD_INIT, "at+cmgf=1\r",10, NULL, 0);
				break;
			case SM_MSG_CMGF_SEND_OK:
				gNeo_at_init_state_machine1=SM_MSG_CSCS_SENDING;// they used "at+cscs=\"HEX\"\r" because to send message in HEX format.
				Neo_SendAT(CMD_INIT, "at+cscs=\"HEX\"\r",14, NULL, 0); 
				break;
			case SM_MSG_CSCS_SEND_OK:
				gNeo_at_init_state_machine1=SM_MSG_CMGS_SENDING;
				Neoway_Print("message_count %d no_of_sms %d",message_count, no_of_sms);
				if(message_count < no_of_sms){
					DATA_SENDING_FLAG = 1;// for control LED blinking while data sending
					asciitohex(message);
					message[strlen(message)]='\x1a';
					Neo_SendAT(CMD_CMGS, MSG_Send_Cmd,strlen(MSG_Send_Cmd), message, strlen(message));
				}
				else{
					Neoway_Print("Message Send Count Limit Exceeded!!!");
					Neoway_StopTimer(&LED_Glow_Timer_Task3);
					Reset_RTC_Fun();
				}
				break;
			// case SM_MSG_CMGS_SEND_OK:
				// gNeo_at_init_state_machine1=SM_MSG_CSCS_GSM_SENDING; //  used "at+cscs=\"GSM\"\r" because to get receive message in TEXT format.
				// Neo_SendAT(CMD_INIT, "at+cscs=\"gsm\"\r",14, NULL, 0); 
				// break;
			default :
				break;
		}
		return ;
	}
}
/*****************************************************************************
 * Definition:   stop_timers
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void stop_timers() {
	Neoway_Print("All Timers stops working");
	Neoway_StopTimer(&neoway_at_stream);
	Neoway_StopTimer(&neoway_to_send_at);
	Neoway_StopTimer(&neoway_at_send_overtime);
	Neoway_StopTimer(&neoway_msg_send_timer_task2);
	Neoway_StopTimer(&LED_Glow_Timer_Task3);
}
/*****************************************************************************
 * Definition:   start_timer
 * Parameter:   void
 * Return value:   void
 *****************************************************************************/
void start_timer() {
	Neoway_StartTimer(&neoway_at_stream,21);  // 216 ticks equals to one seconds
	Neoway_StartTimer(&LED_Glow_Timer_Task3,21);  // 216 ticks equals to one seconds
}


#endif