#ifndef _RTC_H
#define _RTC_H
#include "stm32f10x.h"

typedef struct 
{
	u8 hour;
	u8 min;
	u8 sec;			
	//公历日月年周
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;		 
}tm;	
				 
extern tm timer; 

/**************************************************************/

extern void RTC_Configuration(void);
extern void Time_Adjust(u32 tr_val);
extern void RTC_INIT(void);
extern void GF_SendTim(void);
extern void RTC_AWU_SET(void);
extern void RTC_SET_ALARM(u32 sec);
extern void RTC_ClearWakeUp(void);	
extern void GF_SetTime(u8 tr_fival,u8 tr_seval,u8 tr_thval,u8 tr_forval);
u8 Is_Leap_Year(u16 year);
u8 RTC_Get(void);

/**************************************************************/
#endif //_RTC_H
