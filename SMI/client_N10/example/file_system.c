
#include "neoway_openplatform.h"
#include "file_system.h"


S32 Write_file_handle,Read_file_handle,written_handle,commit_handle,read_handle,close_handle,delete_handle;
U32 writelenth,readLenth;

/**********************fixed variable Declaration****************************************/
U8 AMN[14]="+918660366534", DMN[11]="8660366534", SPD[3]="99"; // default values
/**************************************************************************************/

void Neoway_Read_File(char* file_name, char* readData, U16 size);
void Neoway_Write_To_File(char* file_name, char* writeData, U16 size);
void Neoway_delete_File(char* file_name);
void No_Data_Write_Fun(char* file_name, char* writeData, U16 size);
/***********************************************************************************************************/

/*****************************************************************************
 * Definition:  Neoway_Write_To_File
 * Parameter:   char* file_name, char* writeData
 * Return value:   void
 *****************************************************************************/
void Neoway_Write_To_File(char* file_name, char* writeData, U16 size)
{
	// Neoway_Print("file_name is %s\r",file_name);
	Neoway_delete_File(file_name);
	Write_file_handle = (S32)Neoway_FSOpen((const U16*)file_name,NEOWAY_FS_READ_WRITE | NEOWAY_FS_CREATE);//Open the file in readable manner. If the file does not exist, creat a file.
	if(Write_file_handle < 0){
		Neoway_Print("%d,Error while Writting File",Write_file_handle);
	}
	else{		
		written_handle = Neoway_FSWrite(Write_file_handle, writeData, size, &writelenth);//Write a character A of 1024 bytes into the file
		if(written_handle == 0){
			Neoway_Print("%d,data written successfully",written_handle);
			// Neoway_Print("written data is: %s",writeData);
			// Neoway_Print("written data lenth is: %d",writelenth);
		}
		else{
			Neoway_Print("%d,data not written successfully",written_handle);
		}
		commit_handle = Neoway_FSCommit(Write_file_handle);//	Write the data in the buffer to the file.
		if(commit_handle == 0){
			// Neoway_Print("%d,file commit successfully",commit_handle);
		}
		else{
			Neoway_Print("%d,file not commit successfully",commit_handle);
		}
		close_handle = Neoway_FSClose((S32) Write_file_handle);
		if(close_handle == 0){
			// Neoway_Print("%d,file closed successfully",close_handle);
		}
		else{
			Neoway_Print("%d,file not closed successfully",close_handle);
		}
	}
}
/*****************************************************************************
 * Definition:  Neoway_Read_File
 * Parameter:   char* file_name, char* readData
 * Return value:   void
 *****************************************************************************/
void Neoway_Read_File(char* file_name, char* readData, U16 size)
{
	// Neoway_Print("file_name is %s\r",file_name);
	memset(readData,0,sizeof(readData));//Assign 0 to all elements of the array.
	Read_file_handle = (S32)Neoway_FSOpen((const U16*)file_name,NEOWAY_FS_READ_ONLY);//Open the file in readable manner. If the file does not exist, creat a file.		
	if(Read_file_handle < 0)
	{
		Neoway_Print("%d,Error while reading File",Read_file_handle);
		close_handle = Neoway_FSClose((S32) Read_file_handle);
		if(close_handle == 0){
			// Neoway_Print("%d,file closed successfully",close_handle);
		}
		else{
			Neoway_Print("%d,file not closed successfully",close_handle);
		}
		Read_file_handle = (S32)Neoway_FSOpen((const U16*)file_name,NEOWAY_FS_READ_ONLY);//Open the file in readable manner. If the file does not exist, creat a file.
		if(Read_file_handle < 0)
		{
			Neoway_Print("%d, Again Error while reading File",Read_file_handle);
			if(strcmp(file_name, "amn.txt")== 0){
				No_Data_Write_Fun(file_name, AMN, size);
			}
			else if(strcmp(file_name, "dmn.txt")== 0){
				No_Data_Write_Fun(file_name, DMN, size);
			}
			else if(strcmp(file_name, "spd.txt")== 0){
				No_Data_Write_Fun(file_name, SPD, size);
			}
			else{
			}
		}
	}		
	read_handle = Neoway_FSRead(Read_file_handle, readData, size, &readLenth);
	if(close_handle == 0){
		Neoway_Print("%d,file red successfully",close_handle);
		// Neoway_Print("readData is: %s",readData);
		// Neoway_Print("lenth is %d",readLenth);
	}
	else{
		Neoway_Print("%d,file not red successfully",close_handle);
	}
	close_handle = Neoway_FSClose((S32) Read_file_handle);
	if(close_handle == 0){
		// Neoway_Print("%d,file closed successfully",close_handle);
	}
	else{
		Neoway_Print("%d,file not closed successfully",close_handle);
	}
}
/*****************************************************************************
 * Definition:  Neoway_delete_File
 * Parameter:   char* file_name
 * Return value:   void
 *****************************************************************************/
void Neoway_delete_File(char* file_name)
{
	// Neoway_Print("Neoway_delete_File_Function\r");
	delete_handle = Neoway_FSDelete((const U16*)file_name);
	if(delete_handle == 0){
		// Neoway_Print("%d,file deleted successfully",delete_handle);
	}
	else{
		Neoway_Print("%d,file not deleted successfully",delete_handle);
	}
}

/*****************************************************************************
 * Definition:  No_Data_Write_Fun
 * Parameter:   char* file_name, char* writeData
 * Return value:   void
 *****************************************************************************/
void No_Data_Write_Fun(char* file_name, char* writeData, U16 size)
{
	Neoway_Print("No_Data_Write_Function\r");
	// Neoway_Print("file_name is %s\r",file_name);
	Neoway_delete_File(file_name);
	Write_file_handle = (S32)Neoway_FSOpen((const U16*)file_name,NEOWAY_FS_READ_WRITE | NEOWAY_FS_CREATE);//Open the file in readable manner. If the file does not exist, creat a file.
	if(Write_file_handle < 0){
		Neoway_Print("%d,Error while Writting File",Write_file_handle);
	}
	else
	{		
		written_handle = Neoway_FSWrite(Write_file_handle, writeData, size, &writelenth);//Write a character A of 1024 bytes into the file
		if(written_handle == 0){
			Neoway_Print("%d,data written successfully",written_handle);
			// Neoway_Print("written data is: %s",writeData);
			// Neoway_Print("written data lenth is: %d",writelenth);
		}
		else{
			Neoway_Print("%d,data not written successfully",written_handle);
		}
		commit_handle = Neoway_FSCommit(Write_file_handle);//	Write the data in the buffer to the file.
		if(commit_handle == 0){
			// Neoway_Print("%d,file commit successfully",commit_handle);
		}
		else{
			Neoway_Print("%d,file not commit successfully",commit_handle);
		}
		close_handle = Neoway_FSClose((S32) Write_file_handle);
		if(close_handle == 0){
			// Neoway_Print("%d,file closed successfully",close_handle);
		}
		else{
			Neoway_Print("%d,file not closed successfully",close_handle);
		}
	}
}