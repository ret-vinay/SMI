
#include "neoway_openplatform.h"


/**************************variables declaration*********************************/

extern S32 Write_file_handle,Read_file_handle,written_handle,commit_handle,read_handle,close_handle,delete_handle;

extern U32 writelenth,readLenth;

/**********************fixed variable Declaration****************************************/
extern U8 AMN[14], DMN[11], SPD[3];
/**************************************************************************************/

/***************************functions declaration********************************/

extern void Neoway_Read_File(char* file_name, char* readData, U16 size);
extern void Neoway_Write_To_File(char* file_name, char* writeData, U16 size);
extern void Neoway_delete_File(char* file_name);
extern void No_Data_Write_Fun(char* file_name, char* writeData, U16 size);
/***********************************************************/