#include <includes.h>

static  OS_STK         App_TaskStartStk[APP_TASK_START_STK_SIZE];

static  void  App_TaskCreate		(void);
static  void  App_TaskStart			(void		*p_arg);  
extern  void  App_GPS_TaskCreate  (void);
static  void  GPIO_Configuration    (void);

/*************************************************
函数: void RCC_Configuration(void)
功能: 复位和时钟控制 配置
参数: 无
返回: 无
**************************************************/
void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;                    //定义外部高速晶体启动状态枚举变量
  RCC_DeInit();                                    //复位RCC外部设备寄存器到默认值
  RCC_HSEConfig(RCC_HSE_ON);                       //打开外部高速晶振
  HSEStartUpStatus = RCC_WaitForHSEStartUp();      //等待外部高速时钟准备好
  if(HSEStartUpStatus == SUCCESS)                  //外部高速时钟已经准别好
  {
    //FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //开启FLASH预读缓冲功能，加速FLASH的读取。所有程序中必须的用法.位置：RCC初始化子函数里面，时钟起振之后
    //FLASH_SetLatency(FLASH_Latency_2);                    //flash操作的延时
      	
    RCC_HCLKConfig(RCC_SYSCLK_Div1);               //配置AHB(HCLK)时钟等于==SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);                //配置APB2(PCLK2)钟==AHB时钟
    RCC_PCLK1Config(RCC_HCLK_Div2);                //配置APB1(PCLK1)钟==AHB1/2时钟
         
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);  //配置PLL时钟 == 外部高速晶体时钟 * 9 = 72MHz
    RCC_PLLCmd(ENABLE);                                   //使能PLL时钟
   
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)    //等待PLL时钟就绪
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);            //配置系统时钟 = PLL时钟
    while(RCC_GetSYSCLKSource() != 0x08)                  //检查PLL时钟是否作为系统时钟
    {
    }
  }

}
/*******************************************************************************
* Function Name   : NVIC_Configuration
* Description        : Configures NVIC and Vector Table base location.
* Input                    : None
* Output                 : None
* Return                 : None
*******************************************************************************/
void NVIC_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;
  
   /* Set the Vector Table base location at 0x08000000 */
   NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
  
   /* Configure the NVIC Preemption Priority Bits */  
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
   /* Enable the USART1 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;     //通道设置为串口1中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						           //初始化
	
	 /* Enable the USART2 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;     //通道设置为串口2中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   ///中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						           //初始化
}

#define    QTBL_SIZE                    20   //消息队列数组大小

OS_EVENT*  g_MesgQ;                         //消息队列
void*      g_mesgqtbl[QTBL_SIZE];           //消息队列数组

/*
===============================================================================
 Function    : BuildMQ()
 功能：建立应用需的消息队列
===============================================================================
*/
void BuildMQ(void)
{

	g_MesgQ = OSQCreate(&g_mesgqtbl[0], QTBL_SIZE);  //建一个消息队列
}

INT32S main (void)
{
    CPU_INT08U  os_err;
	os_err = os_err; /* prevent warning... */

 	/* Note:  由于使用UCOS, 在OS运行之前运行,注意别使能任何中断. */
	CPU_IntDis();                    /* Disable all ints until we are ready to accept them.  */

    OSInit();                        /* Initialize "uC/OS-II, The Real-Time Kernel".         */
	BuildMQ();
	os_err = OSTaskCreateExt((void (*)(void *)) App_TaskStart,  /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_START_PRIO,
                             (INT16U          ) APP_TASK_START_PRIO,
                             (OS_STK        * )&App_TaskStartStk[0],
                             (INT32U          ) APP_TASK_START_STK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	OSStart();               /* Start multitasking (i.e. give control to uC/OS-II).  */

	return (0);
}


/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument(s) : p_arg       Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/	  
static  void  App_TaskStart (void *p_arg)
{   
    (void)p_arg;
	
    /***************  Init hardware ***************/
    OS_CPU_SysTickInit();                                    /* Initialize the SysTick.                              */
	RCC_Configuration();
	NVIC_Configuration();
	GPIO_Configuration();
    USART1_Configuration();
	//USART2_Configuration();
	RTC_INIT();
#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                            /* Determine CPU capacity.                              */
#endif

    App_TaskCreate();                                        /* Create application tasks.                            */
	//printf("build %s_%s\n", __DATE__, __TIME__);
    printf("=========Main Task Start=====\n");
	while(1){
      	OSTimeDlyHMSM(0, 1, 0, 0);							 /* Delay One minute */
    }	
}


/*
*********************************************************************************************************
*                                            App_TaskCreate()
*
* Description : Create the application tasks.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TaskCreate (void)
{
	
	App_GPS_TaskCreate();	

}

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configure GPIO Pin
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE); 						 

  //LED -> PB12		 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
