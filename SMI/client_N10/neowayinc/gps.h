#ifndef __GPS_H__
#define __GPS_H__

#include "neoway_openplatform.h"


extern unsigned char acc[10],gyro[10];
extern U8 url[39];
extern U8 port[22];
extern U8 setup[14];
extern U8 action[17];

extern char* StrStr(char *str, char *substr);
extern float my_a2f(char *p);
extern void extracting_gps_values(char* Rcv_str );


#endif