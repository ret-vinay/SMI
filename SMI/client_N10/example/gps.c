#include "gps.h"
#include "math.h"
//#include "stdio.h"
//unsigned char data_to_server[300];

unsigned char data_to_server[400];
int flag=0,length_of_data=0;

/*************************String_string_compare_driver_function************************************************************************************/
char* StrStr(char *str, char *substr)
{
	  while (*str) 
	  {
		    char *Begin = str;
		    char *pattern = substr;
		    
		    // If first character of sub string match, check for whole string
		    while (*str && *pattern && *str == *pattern) 
			{
			      str++;
			      pattern++;
		    }
		    // If complete sub string match, return starting address 
		    if (!*pattern)
		    	  return Begin;
		    	  
		    str = Begin + 1;	// Increament main string 
	  }
	  return NULL;
}
/**************************************************************************************************************************************************/

/*************************string_to_float_function*************************************************************************************************/
float my_a2f(char *p) 
{
  // here i took another two   variables for counting the number of digits in mantissa
  int i, num = 0, num2 = 0, pnt_seen = 0, x = 0, y = 1; 
  float f1, f2, f3;
  for (i = 0; p[i]; i++)
    if (p[i] == '.') 
	{
      pnt_seen = i;
      break;
    }
//Neoway_Print("pnt_seen=%d",pnt_seen); 
 for (i = 0; p[i]; i++)
{
    if (i < pnt_seen) num = num * 10 + (p[i] - 48);
    else if (i == pnt_seen) continue;
    else 
	{
      num2 = num2 * 10 + (p[i] - 48);
      ++x;
    }
}
  //Neoway_Print("num=%d num2=%d x=%d",num,num2,x);
  // it takes 10 if it has 1 digit ,100 if it has 2 digits in mantissa
  for(i = 1; i <= x; i++) 
  y = y * 10;
  f2 = num2 / (float) y;
  f3 = num + f2;
 // Neoway_Print("f3=%f",f3);
  return f3;
}
/**************************************************************************************************************************************************/


void extracting_gps_values(char* loc_buf )
{
	
char comma=',',valid,lat_dir,long_dir;
unsigned char lat_buf[12],long_buf[12],fix[10];
char time_buf[10],degree[5],min[10],PDOP[5],HDOP[5],VDOP[5],satellite_buf[2]; 
unsigned char speed_buf[10],date_buf[6];
int j=0,count=0,a=0,b=0,c=0,m;
int d=0,e=0,g=0,h=0,k=0,z=0;
static int i=0;
float deg_flt,conv_deg_flt,min_flt;
float conv_min_flt,my_lat,my_long;

char *rmc,*gga,*gsa,*gsv,*minn;
unsigned char final_speed1[15]="SPEED:",final_pdop1[10]="PDOP:",final_hdop1[10]="HDOP:",final_vdop1[10]="VDOP:",final_satellite1[15]="SATELLITES:",final_fix1[7]="FIX:";
unsigned char GGA[80],RMC[80],GSV[80],GSA[80],my_buf[20],string_lat[20],string_long[20];
unsigned char final_lat[20]="LAT:",final_long[20]="LONG:",final_time[20]="TIME:",final_date[20]="DATE:",final_speed[20]="SPEED:";
unsigned char final_fix[7]="FIX:",final_pdop[10]="PDOP:",final_hdop[10]="HDOP:",final_vdop[10]="VDOP:",final_satellite[14]="SATELLITES:";


		memset(time_buf,0,sizeof(time_buf));
		memset(lat_buf,0,sizeof(lat_buf));
		memset(long_buf,0,sizeof(long_buf));
		memset(speed_buf,0,sizeof(speed_buf));
		memset(date_buf,0,sizeof(date_buf));
		memset(RMC,0,sizeof(RMC));
		memset(GGA,0,sizeof(GGA));
		memset(GSA,0,sizeof(GSA));
		memset(GSV,0,sizeof(GSV));
		memset(data_to_server,0,sizeof(data_to_server));

//strcpy(loc_buf,Rcv_str);	
/***********************************RMC CONVERTION************************************************************************************************/	
	rmc=StrStr(loc_buf,"$GNRMC");
	if(rmc[0]==0x24)
	{
		for(j=0;rmc[j]!= '*'; j++)//j<80;j++)
		{
			if(rmc[j]==0x0D)
				break;
			else
				RMC[j]=rmc[j];
		}
		RMC[j]='\0';
		Neoway_Print("\n");
		Neoway_Print("RMC=%s",RMC);	
		Neoway_Print("\n");
	}
	
	
	for(i=0;RMC[i]!='\0';i++)
	{
		if(comma==RMC[i])
		{
			count++;
			//Neoway_Print("now comma position=%d %c",i,RMC[i]);
		
			//b=i;
			switch(count)
			{
				case 1:for(a=i+1;RMC[a]!=',';a++)						
						{
							time_buf[z]=RMC[a];
							z++;
						}  
						time_buf[z]='\0';
						//Neoway_Print("time_buf=%s",time_buf);
						z=0;
						break;				
			
				case 2:	valid=RMC[i+1];
						//Neoway_Print("valid_data=%c",valid);
						//Neoway_Print("now value at position A=%d",i+1);
						break;
						
				case 3:	for(b=i+1;RMC[b]!=',';b++)						
						{
							lat_buf[z]=RMC[b];
							z++;
						}  
						lat_buf[z]='\0';
						//Neoway_Print("lat_buf=%s",lat_buf);
						z=0;
						break;
				
				case 4: lat_dir=RMC[i+1];
						//Neoway_Print("lat_dir=%c",lat_dir);
						break;
						
				case 5: for(c=i+1;RMC[c]!=',';c++)
						{
							long_buf[z]=RMC[c];
							z++;	
						} 
						long_buf[z] ='\0';
						//Neoway_Print("long_buf=%s",long_buf);
						z=0;
						break;
					
				case 6:long_dir=RMC[i+1];
						//Neoway_Print("long_dir=%c",long_dir);
						break;
						
				case 7:	for(d=i+1;RMC[d]!=',';d++)
						{
							speed_buf[z]=RMC[d];
							z++;	
						} 
						speed_buf[z] ='\0';
						//Neoway_Print("speed_buf=%s",speed_buf);
						z=0;
						break;
				case 9: for(e=i+1;RMC[e]!=',';e++)
						{
							date_buf[z]=RMC[e];
							z++;	
						} 
						date_buf[z] ='\0';
						//Neoway_Print("date_buf=%s",date_buf);
						z=0;
						break;

			}
		}			
			   
	}
	count=0; a=0;b=0;c=0;d=0;e=0;z=0;
/**************************GGA********************************************************************************************************************/	
	gga=StrStr(loc_buf,"$GNGGA");
	if(gga[0]==0x24)
	{
		for(j=0;j<80;j++)
		{
			if(gga[j]==0x0D)
				break;
			else
				GGA[j]=gga[j];
		}
		Neoway_Print("\n");
		Neoway_Print("GGA=%s",GGA);	
		Neoway_Print("\n");
	}
/**************************************************************************************************************************************************/	
/**********************GSA*************************************************************************************************************************/
gsa=StrStr(loc_buf,"$GNGSA");
	if(gsa[0]==0x24)
	{
		for(j=0;j<80;j++)
		{
			if(gsa[j]==0x0D)
				break;
			else
				GSA[j]=gsa[j];
		}
		Neoway_Print("\n");
		Neoway_Print("GSA=%s",GSA);	
		Neoway_Print("\n");
	}
	
	for(i=0;GSA[i]!='\0';i++)
	{
		if(comma==GSA[i])
		{
			count++;
			//Neoway_Print("now comma position=%d %c",i,RMC[i]);
		
			switch(count)
			{
				
				case 2:	fix[0]=GSA[i+1];
						fix[1]='\0';
						//Neoway_Print("fix=%s",fix);
						break;
						
				case 15:z=0;
						for(g=i+1;GSA[g]!=',';g++)						
						{
							PDOP[z]=GSA[g];
							z++;
						}
						PDOP[z]='\0';
						//Neoway_Print("PDOP=%s",PDOP);
						break;
						
				case 16:z=0;
						for(h=i+1;GSA[h]!=',';h++)						
						{
							HDOP[z]=GSA[h];
							z++;
						}
						HDOP[z]='\0';
						//Neoway_Print("HDOP=%s",HDOP);
						break;
						
				case 17:z=0;
						for(k=i+1;GSA[k]!=',';k++)						
						{
							VDOP[z]=GSA[k];
							z++;
						} 
						VDOP[z]='\0';
						//Neoway_Print("VDOP=%s",VDOP);
						z=0;
						break;
				
			}
		}			
			   
	}
/**************************************************************************************************************************************************/

/***********************GSV************************************************************************************************************************/
gsv=StrStr(loc_buf,"$GPGSV");
	if(gsv[0]==0x24)
	{
		for(j=0;j<80;j++)
		{
			if(gsv[j]==0x0D)
				break;
			else
				GSV[j]=gsv[j];
		}
		Neoway_Print("\n");
		Neoway_Print("GSV=%s",GSV);	
		Neoway_Print("\n");
	}
	
	count=0;
	for(i=0;GSV[i]!='\0';i++)
	{
		if(comma==GSV[i])
		{
			count++;
			//Neoway_Print("now comma position=%d %c",i,GSV[i]);
			a=0;
			if(count==3)
			{
				
					for(a=i+1;GSV[a]!=',';a++)						
					{
						satellite_buf[z]=GSV[a];
						z++;
					}
					strcat(satellite_buf,"\0");
					//Neoway_Print("satellite_buf=%s",satellite_buf);
					z=0;
			
			}
		}
	}
/**************************************************************************************************************************************************/
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(((valid=='A')&&(lat_dir=='N')&&(long_dir=='E'))||((valid=='A')&&(lat_dir=='N')&&(long_dir=='W'))||((valid=='A')&&(lat_dir=='S')&&(long_dir=='E'))||((valid=='A')&&(lat_dir=='S')&&(long_dir=='W')))
	{	
		//Neoway_Print("lat_buf=%s",lat_buf);
		//Neoway_Print("long_buf=%s",long_buf);
/***************************lat_convertion*********************************************************************************************************/
		degree[0]=lat_buf[0];
		degree[1]=lat_buf[1];
		degree[2]='.';
		degree[3]='\0';
		deg_flt=my_a2f(degree);
		//deg_flt=atoi(degree);
		//Neoway_Print("deg_flt=%d",deg_flt);
		memcpy(min,&lat_buf[2],(sizeof(lat_buf)-2));
		min[9]='\0';
		//Neoway_Print("min=%s",min);
		min_flt=my_a2f(min);
		conv_min_flt=min_flt/60;
		my_lat=deg_flt+conv_min_flt;
/**************************************************************************************************************************************************/

/***************************long_convertion********************************************************************************************************/
		degree[0]=long_buf[0];
		degree[1]=long_buf[1];
		degree[2]=long_buf[2];
		degree[3]='.';
		degree[4]='\0';
		//Neoway_Print("degree=%s",degree);
		deg_flt=my_a2f(degree);
		//Neoway_Print("deg_flt=%f",deg_flt);
		memcpy(min,&long_buf[3],(sizeof(long_buf)-3));
		min[9]='\0';
		//Neoway_Print("min=%s",min);
		min_flt=my_a2f(min);
		conv_min_flt=min_flt/60;
		//Neoway_Print("conv_min_flt=%f",conv_min_flt);
		my_long=deg_flt+conv_min_flt;
/**************************************************************************************************************************************************/
/*****************FINAL_DATA_DISPLAY***************************************************************************************************************/
		//Neoway_Print("my_lat=%f",my_lat);
		//Neoway_Print("my_long=%f",my_long);
		//Neoway_Print("time_buf=%s",time_buf);
		strcat(final_time,time_buf);
		Neoway_Print("final_time=%s",final_time);
		strcat(final_speed,speed_buf);
		strcat(final_speed1,speed_buf);
		Neoway_Print("final_speed=%s",final_speed);
		strcat(final_date,date_buf);
		Neoway_Print("final_date=%s",final_date);
		sprintf(string_lat,"%f", my_lat);		//float to string
		sprintf(string_long,"%f", my_long);	//float to string 
		//Neoway_Print("string_lat=%s",string_lat);
		strcat(final_lat,string_lat);
		Neoway_Print("final_lat=%s",final_lat);
		//Neoway_Print("string_long=%s",string_long);
		strcat(final_long,string_long);
		Neoway_Print("final_long=%s",final_long);
		strcat(final_fix,fix);
		strcat(final_fix1,fix);
		Neoway_Print("final_fix=%s",final_fix);
		strcat(final_pdop,PDOP);
		strcat(final_pdop1,PDOP);
		Neoway_Print("final_pdop=%s",final_pdop);
		strcat(final_hdop,HDOP);
		strcat(final_hdop1,HDOP);
		Neoway_Print("final_hdop=%s",final_hdop);
		strcat(final_vdop,VDOP);
		strcat(final_vdop1,VDOP);
		Neoway_Print("final_vdop=%s",final_vdop);
		strcat(final_satellite,satellite_buf);
		//strcat(final_satellite,"\0");
		//strcat(final_satellite1,satellite_buf);
		Neoway_Print("final_satellite=%s",final_satellite);
/**************************************************************************************************************************************************/
		
		
/********************************DATA_TO_SERVER****************************************************************************************************/
		strcat(data_to_server,url);
		strcat(data_to_server,final_lat);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_long);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_time);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_date);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_speed1);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_pdop1);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_vdop1);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_hdop1);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_satellite);
		strcat(data_to_server,"&");
		strcat(data_to_server,final_fix1);
		strcat(data_to_server,"&");
		strcat(data_to_server,acc);
		strcat(data_to_server,"&");
		strcat(data_to_server,"\r");
		length_of_data=strlen(data_to_server);
		Neoway_Print("data_to_serverlength=%d",length_of_data); 
		Neoway_Print("data_to_server=%s",data_to_server);
			
/**************************************************************************************************************************************************/
		
/************************************MEMORY_CLEAR**************************************************************************************************/
		/* memset(time_buf,0,sizeof(time_buf));
		memset(lat_buf,0,sizeof(lat_buf));
		memset(long_buf,0,sizeof(long_buf));
		memset(speed_buf,0,sizeof(speed_buf));
		memset(date_buf,0,sizeof(date_buf));
		memset(RMC,0,sizeof(RMC));
		memset(GGA,0,sizeof(GGA));
		memset(GSA,0,sizeof(GSA));
		memset(GSV,0,sizeof(GSV));
		memset(data_to_server,0,sizeof(data_to_server)); */
/**************************************************************************************************************************************************/
	} 

}
