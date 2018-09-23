#include "rtc.h"
#include "stm32f10x_rtc.h"


/*************************************************************************/
//u32 Month_Days_Accu_C[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
//u32 Month_Days_Accu_L[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};
//#define SecsPerDay   (3600*24)

//uc8 _Compile_Time[]=__DATE__ __TIME__;
//const char szEnglishMonth[MONTH_PER_YEAR][4]={ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#define SET_YEAR  			2017
#define SET_MONTH  			11
#define SET_DATA  			9
#define SET_HOURS  			16
#define SET_MINUTES  		25
#define SET_SECONDS  		37
#define SET_DEFAULT_TIM   ((2017*365+305)*3600*24)+(16*3600)+(25*60)+37

tm timer; 
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


/*************************************************************************/

/*************************************************************************/
void Time_Adjust(u32 tr_val)
{
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(tr_val);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}
/*
*设置时钟
*把输入的时钟转换为秒钟
*以1970年1月1日为基准
*1970~2099年为合法年份
返回值：0，成功；其它：错误
*/
u8 RTC_Set(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec)
{
    u16 t;
    u32 secCount=0;
	
    printf("RTC_Set %02d:%02d:%02d",hour,min,sec);
    if(year<1970||year>2099)
        return 1;//3?′í
    for(t=1970;t<year;t++)    //把所有年份的秒钟相加
    {
        if(Is_Leap_Year(t))//闰年
            secCount+=31622400;//闰年的秒钟数
        else
            secCount+=31536000;    
    }
    mon-=1;//先减掉一个月再算秒数（如现在是5月10日，则只需要算前4个月的天数，再加上10天，然后计算秒数）
    for(t=0;t<mon;t++)
    {
        secCount+=(u32)mon_table[t]*86400;//月份秒钟数相加
        if(Is_Leap_Year(year)&&t==1)
            secCount+=86400;//闰年，2月份增加一天的秒钟数
    }
    
    secCount+=(u32)(day-1)*86400;//把前面日期的秒钟数相加（这一天还没过完，所以-1）
    secCount+=(u32)hour*3600;//小时秒钟数
    secCount+=(u32)min*60;//分钟秒钟数
    secCount+=sec;
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //一定要加词句，不然会死机
	PWR_BackupAccessCmd(ENABLE);  //一定要加词句，不然会死机
	RTC_SetCounter(secCount);   //设置RTC计数器的值
	RTC_WaitForLastTask();  //等待最近一次对RTC寄存器的写操作完成
    return 0;
}

/*************************************************************************/
void RTC_Configuration(void)
{
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);
  /* Reset Backup Domain */
  BKP_DeInit();
  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}
  /* Select LSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);
  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* Enable the RTC Second ################################################*/
  RTC_ITConfig(RTC_IT_SEC, ENABLE);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}
/*************************************************************************/
/*rtc中断向量配置*/
void NVIC_Configuration_RTC(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void RTC_INIT(void)
{
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) {
		/* Backup data register value is not correct or not yet programmed (when
		   the first time the program is executed) */
		printf("RTC not yet configured....\n");
		/* RTC Configuration */
		RTC_Configuration();
	
		/* Adjust time by values entred by the user on the hyperterminal */
		//Time_Adjust(0x162D435A);
		RTC_Set(2019,9,16,8,8,8);
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	} else {
		/* Check if the Power On Reset flag is set */
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
			printf("Power On Reset occurred....\n");
		}
		/* Check if the Pin Reset flag is set */
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
		  	printf("External Reset occurred....\n");
		}
		
		printf("No need to configure RTC....\n");
		/* Wait for RTC registers synchronization */
		RTC_WaitForSynchro();
		/* Enable the RTC Second############################################ */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}
	NVIC_Configuration_RTC();
}
/*************************************************************************/
void GF_SetTime(u8 tr_fival,u8 tr_seval,u8 tr_thval,u8 tr_forval)
{
	u32 tmp;
	tmp = 0;
	printf("ReceiveAndroidTim_paramater %2x,%2x,%2x,%2x\n",tr_fival,tr_seval,tr_thval,tr_forval);

	tmp = (u32)(tr_fival<< 24);
	tmp += (u32)(tr_seval<< 16);
	tmp += (u16)(tr_thval<< 8);
	tmp += tr_seval;
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //一定要加词句，不然会死机
	PWR_BackupAccessCmd(ENABLE);  //一定要加词句，不然会死机
	RTC_SetCounter(tmp);   //设置RTC计数器的值
	RTC_WaitForLastTask();  //等待最近一次对RTC寄存器的写操作完成

	//Time_Adjust(tmp);
}

void RTC_EXTI_INITIAL(FunctionalState interrupt_en_or_dis)
{
 NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure; 
//------------EXTI17 配置 -------------------   
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = interrupt_en_or_dis;
    EXTI_Init(&EXTI_InitStructure); 
//------------设置 中断------------------- 
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;//防拆
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = interrupt_en_or_dis;
    NVIC_Init(&NVIC_InitStructure);     
//------------------------------------------- 
}
void RTC_SET_ALARM(u32 sec)
{
  //DEBUG_COM_STREAM("-闹钟-",NULL);
  RTC_SetAlarm(RTC_GetCounter()+sec);
  //DEBUG_COM_STREAM("-闹钟1-",NULL);
  RTC_WaitForLastTask();
  //DEBUG_COM_STREAM("-闹钟2-",NULL);
  RTC_ITConfig(RTC_FLAG_ALR,ENABLE);
}
void RTC_AWU_SET(void)
{
  //启用PWR和BKP的时钟（from APB1）
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  //后备域解锁
  PWR_BackupAccessCmd(ENABLE);
  RTC_ITConfig(RTC_IT_SEC, DISABLE);
  RTC_SET_ALARM(3);
  //PWR_BackupAccessCmd(DISABLE);
  RTC_EXTI_INITIAL(ENABLE);
}
void RTCAlarm_IRQHandler(void)
{
   EXTI_ClearITPendingBit(EXTI_Line17);
  //SYS.wake_id|=1<<17;
}
extern char g_Mode;
void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET) { 
    RTC_ClearITPendingBit(RTC_IT_SEC);
    RTC_WaitForLastTask();
    //TIME_COUNT=RTC_GetCounter();
    //RTC_WaitForLastTask();
    RTC_Get();
	if(g_Mode=='2')
    printf("%4d/%02d/%02d %02d:%02d:%02d\n",timer.w_year,timer.w_month,timer.w_date,
		                                                   timer.hour,timer.min,timer.sec);
  }
  if (RTC_GetITStatus(RTC_FLAG_ALR) != RESET) {
     RTC_ClearITPendingBit(RTC_FLAG_ALR);
     RTC_WaitForLastTask();
     //SYS.wake_id|=1<<1;
  }
}

//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void)
{
    static u16 daycnt = 0;
    u32 timecount = 0;
    u32 temp = 0;
    u16 temp1 = 0;

    timecount = RTC->CNTH; //得到计数器中的值(秒钟数)
    timecount <<= 16;
    timecount += RTC->CNTL;

    temp = timecount / 86400; //得到天数(秒钟数对应的)
    if(daycnt != temp) //超过一天了
    {
        daycnt = temp;
        temp1 = 1970; //从1970年开始
        while(temp >= 365)
        {
            if(Is_Leap_Year(temp1))//是闰年
            {
                if(temp >= 366)temp -= 366; //闰年的秒钟数
                else
                {
                    temp1++;
                    break;
                }
            }
            else temp -= 365; //平年
            temp1++;
        }
        timer.w_year = temp1; //得到年份
        temp1 = 0;
        while(temp >= 28) //超过了一个月
        {
            if(Is_Leap_Year(timer.w_year) && temp1 == 1) //当年是不是闰年/2月份
            {
                if(temp >= 29)temp -= 29; //闰年的秒钟数
                else break;
            }
            else
            {
                if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //平年
                else break;
            }
            temp1++;
        }
        timer.w_month = temp1 + 1; //得到月份
        timer.w_date = temp + 1; //得到日期
    }
    temp = timecount % 86400; //得到秒钟数
    timer.hour = temp / 3600; //小时
    timer.min = (temp % 3600) / 60; //分钟
    timer.sec = (temp % 3600) % 60; //秒钟
    return 0;
}

u8 Is_Leap_Year(u16 year)
{
    if(year % 4 == 0) //必须能被4整除
    {
        if(year % 100 == 0)
        {
            if(year % 400 == 0)return 1; //如果以00结尾,还要能被400整除
            else return 0;
        }
        else return 1;
    }
    else return 0;
}


