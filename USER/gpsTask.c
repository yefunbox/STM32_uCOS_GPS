#include <includes.h>            
#include "nmeaParse.h"
#include "gmath.h"

static __align(8) OS_STK         App_TaskBlinkStk[APP_TASK_LED_STK_SIZE];

static void gpsTask (void);
char g_Mode = 0;
nmeaPOS lastPos;
nmeaPOS nowPos;
double dst;

void handerGpsData(const char* gpsBuffer,int size);


void  App_GPS_TaskCreate (void) {
    CPU_INT08U  os_err;
	os_err = os_err; /* prevent warning... */
	os_err = OSTaskCreate((void (*)(void *)) gpsTask,				
                          (void          * ) 0,							
                          (OS_STK        * )&App_TaskBlinkStk[APP_TASK_LED_STK_SIZE - 1],		
                          (INT8U           ) APP_TASK_BLINK_PRIO  );
}

extern OS_EVENT* g_MesgQ;
struct_gpsDataMQ  gpsDataMQ;
#if 1
void parseGpsData(const u8* data,u8 size) {
    int i = 0;
	u8 ret = 0;
	for(i = 0;i < size;i++) {
		ret = GPS_Parser(data[i]);
	}
}
void GPRMC_CallBack(struct_GPSRMC GPS_RMC_Data) {
	char buffer[80];
	//2018/09/18 00:00:00
	printf("UTC = %4d/%02d/%02d %2d:%02d:%02d  ",GPS_RMC_Data.UTCDateTime[0],GPS_RMC_Data.UTCDateTime[1],
											     GPS_RMC_Data.UTCDateTime[2],GPS_RMC_Data.UTCDateTime[3],
   											     GPS_RMC_Data.UTCDateTime[4],GPS_RMC_Data.UTCDateTime[5]);
    //return;
#if 1
    //Latitude	dd'mm'ss.sss"
    printf("Latitude=%02d %02d'%06.3f\"  ",GPS_RMC_Data.LatitudeD,GPS_RMC_Data.LatitudeM,(float)GPS_RMC_Data.LatitudeS*60/10000);
    //Longitude ddd'mm'ss.sss"
    printf("Longitude=%03d %02d'%06.3f\"  ",GPS_RMC_Data.LongitudeD,GPS_RMC_Data.LongitudeM,(float)GPS_RMC_Data.LongitudeS*60/10000);
//#else
    printf("Latitude=%02.6f  ",GPS_RMC_Data.LatitudeD+(float)(GPS_RMC_Data.LatitudeM+(float)GPS_RMC_Data.LatitudeS/10000)/60);
    printf("Longitude=%03.6f  ",GPS_RMC_Data.LongitudeD+(float)(GPS_RMC_Data.LongitudeM+(float)GPS_RMC_Data.LongitudeS/10000)/60);
#endif	
    printf("->%c  ",GPS_RMC_Data.NS);
    printf("%c <-	 ",GPS_RMC_Data.EW);
    printf("Speed=%.1fKM   ",((double)GPS_RMC_Data.Speed/10)*1.852);  //12*6=72pixel
    printf("Course=%5.1f \n",(float)GPS_RMC_Data.Course/10);
	//nmea_gen_GPRMC(buffer,GPS_RMC_Data);
	//printf("===>%s\n",buffer);
}
void GPGGA_CallBack(struct_GPSGGA GPS_GGA_Data) {
	printf("Altitude:%.1fmeter\n",atof(GPS_GGA_Data.Altitude));
}
void GPGSA_CallBack(struct_GPSGSA GPS_GSA_Data) {
	return;
   	printf("=========GPGSA_CallBack=====\n");
    printf("Mode1 = %c\n",GPS_GSA_Data.Mode);
    printf("Mode2 = %c\n",GPS_GSA_Data.Mode2);
    printf("Mode2 = %c\n",GPS_GSA_Data.Mode2);
    printf("Mode2 = %c\n",GPS_GSA_Data.Mode2);
    printf("Mode2 = %c\n",GPS_GSA_Data.Mode2);
	
}
void GPGSV_CallBack(struct_GPSGSV GPS_GSV_Data) {
	int i;
	return;

   	printf("=========GPGSV_CallBack=====\n");
    printf("SatInView = %d\n",GPS_GSV_Data.SatInView);
    for(i=0;i < 12;i++) {
       printf("SatID=%02d,Elevation=%02d,Azimuth=%03d,SNR=%02d\n",GPS_GSV_Data.SatInfo[i].SatID,GPS_GSV_Data.SatInfo[i].Elevation,
                                                                  GPS_GSV_Data.SatInfo[i].Azimuth,GPS_GSV_Data.SatInfo[i].SNR);
    }
}

void initNmeaParserCallBack() {
    initParserCallBack(GPRMC_CallBack,GPGGA_CallBack,
	                   GPGSA_CallBack,GPGSV_CallBack);
}
void testGPSDemo() {
     int i = 0,j = 0,lengh;
	 
     const char *buff[] = {
		"$GPRMC,023048.00,A,2233.75606,N,11355.73507,E,22.867,182.55,090918,,,A*5F",
		"$GPVTG,182.55,T,,M,22.867,N,42.350,K,A*3F",
		"$GPGGA,023048.00,2233.75606,N,11355.73507,E,1,06,1.32,59.0,M,-2.8,M,,*7B",
		"$GPGSA,A,3,06,17,19,03,09,28,,,,,,,2.25,1.32,1.83*05",
		"$GPGSV,3,1,09,03,28,045,19,06,52,296,26,09,14,127,13,17,52,014,17*72",
		"$GPGSV,3,2,09,19,40,335,18,23,12,098,,24,01,297,,28,62,162,27*77",
		"$GPGSV,3,3,09,41,46,237,*41",
		"$GPGLL,2233.75606,N,11355.73507,E,023048.00,A,A*63",
     };
     lengh = sizeof(buff)/sizeof(buff[0]);
     printf("testGPSDemo lengh = %d\n",lengh);

     for(i = 0;i < lengh;i++) {
		 parseGpsData(buff[i],(int)strlen(buff[i]));
		 //handerGpsData(buff[i],(int)strlen(buff[i]));
	 }

}

void testNmeaGenerate() {
    nmeaINFO info;
    char buff[100];

    //nmea_zero_INFO(&info);

    info.sig = 3;
    info.fix = 3;
    info.lat = 5000.0;
    info.lon = 3600.0;
    info.speed = 2.14 * 3.6;
    info.elv = 10.86;

    info.satinfo.inuse = 1;
    info.satinfo.inview = 1;

    nmea_generate(&buff[0], 100, &info,GPRMC);
	printf("testNmeaGenerate=%s",buff);
}
void debugMode(char mode) {
  switch(mode) {
  	case '1':
	  testGPSDemo();
	  break;
    case '2': //show RTC
	  break;
	case '3': //show ubx
	  break;
  }
}
#endif
extern char* g_gpsData_ptr;
void handerGpsData(const char* gpsBuffer,int size) {
	char buff[300];
	int pack_type;
	int i;
	nmeaGPRMC rmcPack;
	nmeaGPGSA gsaPack;
	nmeaGPGSV gsvpack;
	nmeaGPVTG vtgPack;
	int hAcc = 0;

	//parseGpsData(gpsBuffer,size);
	//return;
	memcpy(buff, gpsBuffer, size);
	pack_type = nmea_pack_type(&buff[1],size);
	switch(pack_type) {
		case GPRMC:
			 nmea_parse_GPRMC(buff,size,&rmcPack);
			 //printf("UTC=%d/%d/%d %d:%d:%d,  ",pack.utc.year+1900,pack.utc.mon,pack.utc.day,
			 //	                               pack.utc.hour,pack.utc.min,pack.utc.sec);
			 nmea_ndeg2degree(rmcPack.lat,&nowPos.lat);
			 nmea_ndeg2degree(rmcPack.lon,&nowPos.lon);
			 if(rmcPack.status == 'A') {
			     if((lastPos.lat == 0) && (lastPos.lon == 0)) {
				 	
					 printf("firt init lastPos lat=%f, lon=%f  \n",lastPos.lat,lastPos.lon);
				     lastPos.lat = nowPos.lat;
				     lastPos.lon = nowPos.lon;
			     } else {
					 //dst = GetDistance(lastPos,nowPos);
					 lastPos.lat = nowPos.lat;
				     lastPos.lon = nowPos.lon;
				 }
			 }
			 
			 printf("lastPos lat=%f, lon=%f  ",lastPos.lat,lastPos.lon);
			 printf("nowPos  lat=%f, lon=%f  ",nowPos.lat,nowPos.lon);
			 //hAcc = ubxhAcc();
             //printf("hAcc = %.1f\n",(double)hAcc/1000);
			 //printf("=>%c %c<== direction=%f \n",pack.ew,pack.ns,pack.direction);
			 //pack.declin_ew = 'E';
			 //printf("ew=%c,mode=%c \n",pack.declin_ew,pack.mode);
			 //memset(buff, 0, 150);
			 //nmea_gen_GPRMC(&buff[0],150, &pack);
			 //printf("buff = %s\n",buff);
			 //RTC_Set(2020,10,8,18,18,18);
			 break; 
		case GPGSA:
			 nmea_parse_GPGSA(buff,size,&gsaPack);
			 printf("fix_mode=%c,fix_type=%d,HDOP=%f,PDOP=%f,VDOP=%f \n",gsaPack.fix_mode,gsaPack.fix_type,
			 	                                                         gsaPack.HDOP,gsaPack.PDOP,gsaPack.VDOP);
			 break;
		case GPGSV:
			 nmea_parse_GPGSV(buff,size,&gsvpack);
			 printf("count=%d,index=%d,sat_count=%d\n",gsvpack.pack_count,gsvpack.pack_index,gsvpack.sat_count);
			 for(i = 0;i < gsvpack.sat_count;i++) {
                 printf("id=%02d,in_use=%d,elv=%02d,azimuth=%03d degrees,sig=%02ddb\n",gsvpack.sat_data[i].id,gsvpack.sat_data[i].in_use,
				 	                                               gsvpack.sat_data[i].elv,gsvpack.sat_data[i].azimuth,gsvpack.sat_data[i].sig);
			 }
			 break;
		case GPVTG:
			 nmea_parse_GPVTG(buff,size,vtgPack);
			 printf("dir=%f,dir_t=%c dec=%f,dec_m=c spn=%f,spn_n=%c spk=%f,spk_k=%c\n",
			 	                                      vtgPack.dir,vtgPack.dir_t,
			 	                                      vtgPack.dec,vtgPack.dec_m,
			 	                                      vtgPack.spn,vtgPack.spn_n,
			 	                                      vtgPack.spk,vtgPack.spk_k);
			 break;
		case GPGLL:
			 printf("GPGLL\n");
			 break;
	}

}
void enableUBX_NAV_PVT(){
	u8 i = 0;
	u8 enableCFG[] = {
		//0xB5,0x62,0x06,0x01,0x03,0x00,0x01,0x07,0x01,0x13,0x51,
		//0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x06,0x00,0x01,0x00,0x00,0x00,0x00,0x17,0xDA,
		0xB5,0x62,0x06,0x01,0x06,0x00,0x01,0x02,0x00,0x01,0x00,0x00,0x11,0x88,
	};
	for(i = 0;i < sizeof(enableCFG);i++) {
	    Uart2_PutChar(enableCFG[i]);
		//Uart1_PutChar(enableCFG[i]);
	}
}
void printHexToString(const char* buffer,u8 buffer_sz) {
	u8 i = 0;
    for(i = 0;i < buffer_sz;i++) {
		if(g_Mode == '3')  printf("%x ",buffer[i]);
		else               Uart1_PutChar(buffer[i]);
	}
	printf("\n");
}

void handerUbxData(const char* ubxBuffer,int size) {
	int hAcc = 0;
	
	printHexToString(ubxBuffer,size);
	if(ubxBuffer[2]==0x01 && ubxBuffer[3]==0x02 ) {//0xB5,0x62,0x01,0x02
		hAcc = ubxhAcc(ubxBuffer,size);
		printf("hAcc = %.2f\n",(double)hAcc/1000);
	}


}

static void gpsTask (void) {
	struct_gpsDataMQ*    mesg;
	u8      err;
	nmeaPOS from_pos,to_pos;
	
	//char data[] = {"$GPRMC,085432.00,A,2234.72646,N,11354.44461,E,0.472,,050918,,,A*7B"};
   	printf("=========GPS Task Start=====\n");
#if 1
    from_pos.lat = 22.61664;  //深圳北(114.036953,22.61664)
	from_pos.lon = 114.036953;
	to_pos.lat = 22.543366;   //前海湾地铁站(113.904669,22.543366)
	to_pos.lon = 113.904669;
    dst = GetDistance(from_pos,to_pos);
#else
	from_pos.lat = 22.578779;
	from_pos.lon = 113.907402;
	to_pos.lat = 22.578791;
	to_pos.lon = 113.907394;
    dst = GetDistance(from_pos,to_pos);
#endif
	//initNmeaParserCallBack();
	//testGPSDemo();
	
	//nmea_parse_GPRMC(&data,sizeof(data),&pack);
	
	//printf("==hour=%d:%d:%d lat=%f lon=%f\n",pack.utc.hour,pack.utc.min,pack.utc.sec,
	//	                                     pack.lat,pack.lon);
	//testNmeaGenerate();
	enableUBX_NAV_PVT();

   	while(1){
		mesg = (struct_gpsDataMQ* )OSQPend(g_MesgQ, 0, &err);  //等待消息
		GPIO_ResetBits(GPIOB , GPIO_Pin_12);  //LED-ON
		//printf("MQ_type=%d,size=%d,err=%d\n",mesg->mq_type,mesg->size,err);
		switch(mesg->mq_type) {
			case 1:
				//mesg->pData[mesg->size] = '\0';
				g_gpsData_ptr[mesg->size] = '\0';
				printf("%s\n",g_gpsData_ptr);
				//printf("parseGpsData = %s,pAddr=0x%x,size = %d\n",g_gpsData_ptr,&g_gpsData_ptr,mesg->size);
				//parseGpsData(mesg->pData,mesg->size);
				//handerGpsData(g_gpsData_ptr,mesg->size);
				break;
			case 2:
				g_Mode = (char)*(mesg->pData);
				printf("MQ_type=%d,debugMode=%d,err=%d\n",mesg->mq_type,g_Mode,err);
				debugMode(g_Mode);
				break;
			case 3:
				handerUbxData(g_gpsData_ptr,mesg->size);
				break;
		}
		GPIO_SetBits(GPIOB , GPIO_Pin_12);    //LED-OFF

   }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/



