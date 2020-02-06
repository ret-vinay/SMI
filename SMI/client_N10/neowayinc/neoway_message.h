#ifndef __NEOWAY_MESSAGE_H__
#define __NEOWAY_MESSAGE_H__

#include "neoway_openplatform.h"
// #include "neoway_analysis_sms.h"

#define NEO_MAX_SEND_AT_LEN  600   //The maximum length of the AT command character string
#define NEO_MAX_SEND_DATA_LEN  600  //The maximum length of the data sent by AT commands

typedef enum 
{
	AT_IDLE,//Idle status. No AT command is sent.
	AT_SENDING,//Sending
	AT_WAIT_DATA,//Waiting for data input
	AT_SENDING_DATA//Sending data
}NeowayAtSendStateEnum;//AT data sending

typedef enum 
{
	SM_IDLE,
	SM_AT_SENDING,
	SM_AT_SEND_OK,
	SM_CSCS_SENDING,
	SM_CSCS_SEND_OK,
	SM_CNMI_SENDING,
	SM_CNMI_SEND_OK,
	SM_CGSN_SENDING,
	SM_CGSN_SEND_OK,
	SM_CCID_SENDING,
	SM_CCID_SEND_OK,
	SM_CSQ_SENDING,
	SM_CSQ_SEND_OK,
	SM_CREG_SENDING,
	SM_CREG_SEND_OK,
	SM_XIIC_SENDING,
	SM_XIIC_SEND_OK,
	SM_GPS_SENDING,
	SM_GPS_SEND_OK,
	SM_IPR_SENDING,
	SM_IPR_SEND_OK,
	SM_INIT_OK,
	SM_MSG_IDLE,
	SM_MSG_CSQ_SENDING,
	SM_MSG_CSQ_SEND_OK,
	SM_GLTS_SENDING,
	SM_GLTS_SEND_OK,
	SM_MSG_CMGF_SENDING,
	SM_MSG_CMGF_SEND_OK,
	SM_MSG_CSCS_SENDING,
	SM_MSG_CSCS_SEND_OK,
	SM_MSG_CMGS_SENDING,
	SM_MSG_CMGS_SEND_OK,
	SM_MSG_CSCS_GSM_SENDING,
	SM_MSG_CSCS_GSM_SEND_OK,
	SM_MSG_INIT_OK	
}NeowayATInitStateMachineEnum;//Initialize AT state machine

typedef enum 
{
	SM_GPRS_IDLE,
	SM_GPRS_NETAPN_SENDING,
	SM_GPRS_NETAPN_SEND_OK,
	SM_GPRS_XIIC_SENDING,
	SM_GPRS_XIIC_SEND_OK,
	SM_GPRS_XIIC_Q_SENDING,
	SM_GPRS_XIIC_Q_SEND_OK,
	SM_GPRS_TCPCLOSE_PRE_SENDING,
	SM_GPRS_TCPCLOSE_PRE_SEND_OK,
	SM_GPRS_TCPSETUP_SENDING,
	SM_GPRS_TCPSETUP_WAIT,
	SM_GPRS_TCPSETUP_SEND_OK,
	SM_GPRS_TCPSEND_SENDING,//No need
	SM_GPRS_TCPSEND_SEND_OK,//No need
	SM_GPRS_TCPCLOSE_SENDING,
	SM_GPRS_TCPCLOSE_SEND_OK,
	SM_GPRS_INIT_OK
}NeowayGPRSStateMachineEnum;//Initialize AT state machine

typedef enum
{
	GSM_IDLE,
	GSM_START,
	GSM_ALERING,
	GSM_RING,
	GSM_NOSIM,
	GSM_CONNECT,
	GSM_SIGNAL_BAD,
	GSM_NORMAL,
	GSM_REP
}NeowayGSMLedStateMachineEnum;

typedef enum 
{
	CMD_NULL,
	CMD_CSQ,
	CMD_CREG,
	CMD_TCPSEND,
	CMD_CMGS,
	CMD_GPSPWR,
	CMD_PHONE,
	CMD_AT,
	CMD_INIT,
	CMD_TCP0,
	CMD_TCP0_SETUP,
	CMD_TCP1,
	CMD_TCP1_SETUP
}NeowayCmdEnum;//State machine

typedef struct 
{
	S8 send_str[NEO_MAX_SEND_AT_LEN];
	U32 str_length;
	S8 send_data[NEO_MAX_SEND_DATA_LEN];
	U32 data_length;
	NeowayCmdEnum cmd;//Commands to be sent
	NeowayAtSendStateEnum at_send_state;
}Neo_Send_At_Info;

typedef struct 
{
	Neo_Send_At_Info * head_position;
	Neo_Send_At_Info * end_position;
	int				  total_item;
}NeowaySendAtInfoState;

typedef struct
{
	S8 buffer[2048+2];
	U16 last_lenth;//Length of character strings that has not been processed last time. If OD OA is not found at the end of data processed last time, the remaining character string will be processed next time.
	U16 start_position;//The start position of the response character string (excluding 0D 0A), 0 by default
	U16 column;//The sequence of the character after 0d0a, 0 by default
	U8 tag_num;//End symbol, 1 for 0D, otherwise 0
}Neo_AtResponseDistill;//Extract OK  ERROR from the response


//////////////////Variable quotation//////////////////////
/**************************Timer ID's*****************************************/
extern NeowayTimerId neoway_msg_send_timer_task2;
extern NeowayTimerId LED_Glow_Timer_Task3;
extern NeowayTimerId neoway_at_stream;
extern NeowayTimerId neoway_to_send_at;//Send AT commands in the queue.
extern NeowayTimerId neoway_at_send_overtime;
/******************************************************************************/
extern Neo_Send_At_Info gNeo_send_at_info_link[20];	//Send SMS. Making call through TCP data is in the saving queue.
extern NeowaySendAtInfoState gNeo_send_at_info_state;
extern Neo_Send_At_Info gNeo_at_info;
extern NeowayATInitStateMachineEnum gNeo_at_init_state_machine;	//When SM_INIT_OK is printed, AT initialization is complete. Other commands can be sent now.
extern NeowayATInitStateMachineEnum gNeo_at_init_state_machine1;
extern NeowayGSMLedStateMachineEnum gNeo_gsm_state_machine;

extern NeowayGPRSStateMachineEnum gNeo_gprs0_state_machine;//Channel 0
extern NeowayGPRSStateMachineEnum gNeo_gprs0_dest_state_machine;
extern NeowayGPRSStateMachineEnum gNeo_gprs1_state_machine;//Channel 1
extern NeowayGPRSStateMachineEnum gNeo_gprs1_dest_state_machine;

extern unsigned char gNeo_register_state;
extern unsigned char gNeo_signal_strong;
extern char gNeo_imei_str[16];
extern char gNeo_local_ip[16];

extern S8 send_at_command[NEO_MAX_SEND_AT_LEN];
extern S8 send_at_data[NEO_MAX_SEND_DATA_LEN];

extern U8 message[160], message_data[160], receive_msg_buf[1024];
extern S32 message_count;
extern U8 MSG_Send_Cmd[30];

extern U16 GSM_CONNECTED,DATA_SENDING_FLAG;

extern unsigned char gNeo_MSG_send_count;//Count message sent to prevent the queue from overflow.
extern Neo_AtResponseDistill gNeo_at_response_parser;

extern BOOL	gNeo_isWatchDog ;
extern BOOL	gNeo_isTcplinkOk ;

extern S8 sz_pdu_str[512];
extern BOOL gNeo_sms_state;	//Receive the SMS.


/**************************Function declaration***********************************/
extern void Neoway_MessageTimersInit(void);
extern void Neo_SearchAtResponse(U8 *Buffaddr, U16 Length, NeowayModuleTypeEnum mod_id);
extern void Neo_AT_ReceiveManage(U8 *buff_ptr, U16 len, NeowayModuleTypeEnum mod_id);
extern void Neo_AtInit(void);
extern NeowayBoolEnum Neo_SendAT(NeowayCmdEnum cmd,S8* send_str,U32 str_length,S8* send_data,U32 data_length);
extern void Neo_ATSendCmd(void);
extern void Neo_ATSendData(void);
extern void Neo_ATSendError(void);
extern void Neo_ATSendOvertime(void);
extern void Neo_ATSendOK(void);
extern void Neo_SendNextAT(void);
extern void stop_timers(void);
extern void start_timer(void);
/*********************************************************************************/

#endif  // End-of __NEOWAY_MESSAGE_H__

