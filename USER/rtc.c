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
*����ʱ��
*�������ʱ��ת��Ϊ����
*��1970��1��1��Ϊ��׼
*1970~2099��Ϊ�Ϸ����
����ֵ��0���ɹ�������������
*/
u8 RTC_Set(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec)
{
    u16 t;
    u32 secCount=0;
	
    printf("RTC_Set %02d:%02d:%02d",hour,min,sec);
    if(year<1970||year>2099)
        return 1;//3?�䨪
    for(t=1970;t<year;t++)    //��������ݵ��������
    {
        if(Is_Leap_Year(t))//����
            secCount+=31622400;//�����������
        else
            secCount+=31536000;    
    }
    mon-=1;//�ȼ���һ����������������������5��10�գ���ֻ��Ҫ��ǰ4���µ��������ټ���10�죬Ȼ�����������
    for(t=0;t<mon;t++)
    {
        secCount+=(u32)mon_table[t]*86400;//�·����������
        if(Is_Leap_Year(year)&&t==1)
            secCount+=86400;//���꣬2�·�����һ���������
    }
    
    secCount+=(u32)(day-1)*86400;//��ǰ�����ڵ���������ӣ���һ�컹û���꣬����-1��
    secCount+=(u32)hour*3600;//Сʱ������
    secCount+=(u32)min*60;//����������
    secCount+=sec;
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //һ��Ҫ�Ӵʾ䣬��Ȼ������
	PWR_BackupAccessCmd(ENABLE);  //һ��Ҫ�Ӵʾ䣬��Ȼ������
	RTC_SetCounter(secCount);   //����RTC��������ֵ
	RTC_WaitForLastTask();  //�ȴ����һ�ζ�RTC�Ĵ�����д�������
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
/*rtc�ж���������*/
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
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //һ��Ҫ�Ӵʾ䣬��Ȼ������
	PWR_BackupAccessCmd(ENABLE);  //һ��Ҫ�Ӵʾ䣬��Ȼ������
	RTC_SetCounter(tmp);   //����RTC��������ֵ
	RTC_WaitForLastTask();  //�ȴ����һ�ζ�RTC�Ĵ�����д�������

	//Time_Adjust(tmp);
}

void RTC_EXTI_INITIAL(FunctionalState interrupt_en_or_dis)
{
 NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure; 
//------------EXTI17 ���� -------------------   
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = interrupt_en_or_dis;
    EXTI_Init(&EXTI_InitStructure); 
//------------���� �ж�------------------- 
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;//����
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = interrupt_en_or_dis;
    NVIC_Init(&NVIC_InitStructure);     
//------------------------------------------- 
}
void RTC_SET_ALARM(u32 sec)
{
  //DEBUG_COM_STREAM("-����-",NULL);
  RTC_SetAlarm(RTC_GetCounter()+sec);
  //DEBUG_COM_STREAM("-����1-",NULL);
  RTC_WaitForLastTask();
  //DEBUG_COM_STREAM("-����2-",NULL);
  RTC_ITConfig(RTC_FLAG_ALR,ENABLE);
}
void RTC_AWU_SET(void)
{
  //����PWR��BKP��ʱ�ӣ�from APB1��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  //�������
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

//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Get(void)
{
    static u16 daycnt = 0;
    u32 timecount = 0;
    u32 temp = 0;
    u16 temp1 = 0;

    timecount = RTC->CNTH; //�õ��������е�ֵ(������)
    timecount <<= 16;
    timecount += RTC->CNTL;

    temp = timecount / 86400; //�õ�����(��������Ӧ��)
    if(daycnt != temp) //����һ����
    {
        daycnt = temp;
        temp1 = 1970; //��1970�꿪ʼ
        while(temp >= 365)
        {
            if(Is_Leap_Year(temp1))//������
            {
                if(temp >= 366)temp -= 366; //�����������
                else
                {
                    temp1++;
                    break;
                }
            }
            else temp -= 365; //ƽ��
            temp1++;
        }
        timer.w_year = temp1; //�õ����
        temp1 = 0;
        while(temp >= 28) //������һ����
        {
            if(Is_Leap_Year(timer.w_year) && temp1 == 1) //�����ǲ�������/2�·�
            {
                if(temp >= 29)temp -= 29; //�����������
                else break;
            }
            else
            {
                if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //ƽ��
                else break;
            }
            temp1++;
        }
        timer.w_month = temp1 + 1; //�õ��·�
        timer.w_date = temp + 1; //�õ�����
    }
    temp = timecount % 86400; //�õ�������
    timer.hour = temp / 3600; //Сʱ
    timer.min = (temp % 3600) / 60; //����
    timer.sec = (temp % 3600) % 60; //����
    return 0;
}

u8 Is_Leap_Year(u16 year)
{
    if(year % 4 == 0) //�����ܱ�4����
    {
        if(year % 100 == 0)
        {
            if(year % 400 == 0)return 1; //�����00��β,��Ҫ�ܱ�400����
            else return 0;
        }
        else return 1;
    }
    else return 0;
}


