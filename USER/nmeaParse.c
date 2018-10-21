/*-------------------------------------------------------

      GPS 艙芒脦枚脛拢驴茅 By wowbanui



 掳忙卤鸥脌煤脢路:

 Rev.2010--     鲁玫脢艗掳忙卤鸥

 Rev.201103     1. 脫脜禄炉艙谩鹿鹿脢媒鸥脻, 虏驴路脰脢媒鸥脻脰卤艙脫沤艩脌铆鲁脡脢媒脰碌,艗玫脡脵脛脷沤忙脮艗脫脙.

                2. 脢盲鲁枚虏驴路脰脰卤艙脫碌梅脫脙LCD脙眉脕卯,脪脝脰虏脨猫啪眉啪脛

 Rev.201107     1. 脨脼啪脛鸥颅脦鲁露脠碌脠脦陋脢媒脰碌脨脦脢艙

                2. 脨脼啪脛鲁玫脢艗禄炉脢媒鸥脻路艙路拧



---------------------------------------------------------*/

#include "nmeaParse.h"

#include <stdio.h>

#include <stdlib.h>





static u8   NMEA_Start = 0;                 // NMEA 脫茂鸥盲驴陋脢艗. 艗矛虏芒碌艙 $ 脢卤脰脙 1

static u8   NMEA_TypeParsed = 0;            // NMEA 脌脿脨脥艙芒脦枚脥锚卤脧

static u8   NMEA_MsgType = NMEA_NULL;       // NMEA 脫茂鸥盲脌脿脨脥

static char NMEA_MsgTypeBuff[] = "GPxxx,";  // NMEA 脫茂鸥盲脌脿脨脥脢露卤冒禄潞沤忙

static u8   NMEA_MsgTypeIndex = 0;          // 露脕脠隆 NMEA 脌脿脨脥脢露卤冒脳脰路没碌脛啪枚脢媒

static u8   NMEA_MsgBlock = 0;              // NMEA 脢媒鸥脻脳脰露脦潞脜 沤脫0驴陋脢艗

static u8   NMEA_MsgBlockDatIndex = 0;      // NMEA 脢媒鸥脻脙驴啪枚脳脰露脦脛脷脳脰路没脣梅脪媒 沤脫0驴陋脢艗



static u8   GPS_Parse_Status = 0;           // 碌卤脟掳艙芒脦枚脳沤脤卢.

static u8   SateInfoIndex = 0;              //


static struct_GPSRMC GPS_RMC_Data;
static struct_GPSGGA GPS_GGA_Data;
static struct_GPSGSA GPS_GSA_Data;
static struct_GPSGSV GPS_GSV_Data;
static nmeaGPVTG     GPS_VTG_Data;

static struct_parser_callback Parser_CallBack;

static void ParserGPGGA(char SBuf) {
    switch (SBuf)
    {
        case '*':   //语句结束
                 NMEA_Start=0;
		         Parser_CallBack.gpggaCallback(GPS_GGA_Data);
                 break;
        case ',':   //该字段结束
                 NMEA_MsgBlock++;
                 NMEA_MsgBlockDatIndex=0;
                 break;
        default:    //字段字符
                switch (NMEA_MsgBlock)  // 判断当前处于哪个字段
                {
			            /*
			            case 0:             // <1> UTC时间,hhmmss
			                break;
			            case 1:             // <2> 纬度 ddmm.mmmm
			                break;
			            case 2:             // <3> 纬度半球 N/S
			                break;
			            case 3:             // <4> 经度 dddmm.mmmm
			                break;
			            case 4:             // <5> 经度半球 E/W
			                break;
			            */
                    case 5:      // <6> GPS状态 0=未定位, 1=非差分定位, 2=差分定位, 6=正在估算
                           GPS_GGA_Data.PositionFix=SBuf;
                           break;
                    case 6:      // <7> 正在使用的卫星数量 00~12
                          switch (NMEA_MsgBlockDatIndex)
                          {
                              case 0:
                                     GPS_GGA_Data.SatUsed=(SBuf-'0')*10;
                                     break;
                               case 1:
                                     GPS_GGA_Data.SatUsed+=(SBuf-'0');
                                     break;
                          }
                          break;
                     /*
                                    case 7:             //<8> HDOP水平精度因子 0.5~99.9
                                            GPS_GGA_Data.HDOP[GPS_GGA_Data.BlockIndex]=SBuf;
                                            break;
                                   */
                     case 8:         //<9> 海拔高度 -9999.9~99999.9
                           GPS_GGA_Data.Altitude[NMEA_MsgBlockDatIndex]=SBuf;
                           break;
                  }
        NMEA_MsgBlockDatIndex++;     //字段字符索引++, 指向下一个字符
    }
}

//$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh
static void ParserGPRMC(char SBuf)
{
    switch (SBuf)
    {
    case '*':
		printf("GPRMC_CallBack");
        NMEA_Start=0;
        GPS_Parse_Status=GPS_PARSE_OK;       //接收完毕, 可以处理
        Parser_CallBack.gprmcCallback(GPS_RMC_Data);
        break;
    case ',':
        NMEA_MsgBlock++;
        NMEA_MsgBlockDatIndex=0;
        break;
    default:
        switch (NMEA_MsgBlock)
        {
        case 0:         // <1> UTC时间 hhmmss.mmm
            switch (NMEA_MsgBlockDatIndex)
            {
            case 0: // hh
                GPS_RMC_Data.UTCDateTime[3]=(SBuf-'0')*10;
                break;
            case 1:
                GPS_RMC_Data.UTCDateTime[3]+=(SBuf-'0');
                GPS_RMC_Data.UTCDateTime[3]+=8;         // 源时区是 UTC, 转换成北京时区 +8, 下面要判断是否超过23小时
                break;
            case 2: // mm
                GPS_RMC_Data.UTCDateTime[4]=(SBuf-'0')*10;
                break;
            case 3:
                GPS_RMC_Data.UTCDateTime[4]+=(SBuf-'0');
                break;
            case 4: // ss
                GPS_RMC_Data.UTCDateTime[5]=(SBuf-'0')*10;
                break;
            case 5:
                GPS_RMC_Data.UTCDateTime[5]+=(SBuf-'0');
                break;
            }
            break;
        case 1:         // <2> 定位状态 A=有效定位, V=无效定位
            GPS_RMC_Data.Status=SBuf;
            break;
        case 2:         // <3> 纬度 ddmm.mmmm
            //GPS_RMC_Data.Latitude[NMEA_MsgBlockDatIndex]=SBuf;    //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 分别转换成数值型
            case 0:     // dd
                GPS_RMC_Data.LatitudeD=(SBuf-'0')*10;
                break;
            case 1:
                GPS_RMC_Data.LatitudeD+=(SBuf-'0');
                break;
            case 2:     // mm
                GPS_RMC_Data.LatitudeM=(SBuf-'0')*10;
                break;
            case 3:
                GPS_RMC_Data.LatitudeM+=(SBuf-'0');
                break;
            case 4:     // 小数点
                break;
            case 5:     // mmmm
                GPS_RMC_Data.LatitudeS=(SBuf-'0')*1000;
                break;
            case 6:
                GPS_RMC_Data.LatitudeS+=(SBuf-'0')*100;
                break;
            case 7:
                GPS_RMC_Data.LatitudeS+=(SBuf-'0')*10;
                break;
            case 8:
                GPS_RMC_Data.LatitudeS+=(SBuf-'0');
                break;
            }
            break;
        case 3:         //<4> 纬度半球 N/S
            GPS_RMC_Data.NS=SBuf;
            break;
        case 4:         //<5> 经度 dddmm.mmmm
            //GPS_RMC_Data.Longitude[NMEA_MsgBlockDatIndex]=SBuf;   //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 分别转换成数值型
            case 0:     // ddd
                GPS_RMC_Data.LongitudeD=(SBuf-'0')*100;
                break;
            case 1:
                GPS_RMC_Data.LongitudeD+=(SBuf-'0')*10;
                break;
            case 2:
                GPS_RMC_Data.LongitudeD+=(SBuf-'0');
                break;
            case 3:     // mm
                GPS_RMC_Data.LongitudeM=(SBuf-'0')*10;
                break;
            case 4:
                GPS_RMC_Data.LongitudeM+=(SBuf-'0');
                break;
            case 5:     // 小数点
                break;
            case 6:     // mmmm
                GPS_RMC_Data.LongitudeS=(SBuf-'0')*1000;
                break;
            case 7:
                GPS_RMC_Data.LongitudeS+=(SBuf-'0')*100;
                break;
            case 8:
                GPS_RMC_Data.LongitudeS+=(SBuf-'0')*10;
                break;
            case 9:
                GPS_RMC_Data.LongitudeS+=(SBuf-'0');
                break;
            }
            break;
        case 5:         // <6> 经度半球 E/W
            GPS_RMC_Data.EW=SBuf;
            break;
        case 6:         // <7> 地面速率 000.0~999.9 节
            //GPS_RMC_Data.sSpeed[NMEA_MsgBlockDatIndex]=SBuf;  //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型, 数值x10
            case 0:
                GPS_RMC_Data.Speed=(SBuf-'0')*1000;
                break;
            case 1:
                GPS_RMC_Data.Speed+=(SBuf-'0')*100;
                break;
            case 2:
                GPS_RMC_Data.Speed+=(SBuf-'0')*10;
                break;
            case 3:
                break;
            case 4:
                GPS_RMC_Data.Speed+=(SBuf-'0');
                break;
            }
            break;
        case 7:         // <8> 地面航向 000.0~359.9 度, 以真北为参考基准
            //GPS_RMC_Data.sCourse[NMEA_MsgBlockDatIndex]=SBuf; //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型, 数值x10
            case 0:
                GPS_RMC_Data.Course=(SBuf-'0')*1000;
                break;
            case 1:
                GPS_RMC_Data.Course+=(SBuf-'0')*100;
                break;
            case 2:
                GPS_RMC_Data.Course+=(SBuf-'0')*10;
                break;
            case 3: // 小数点, 忽略
                break;
            case 4:
                GPS_RMC_Data.Course+=(SBuf-'0');
                break;
            }
            break;
        case 8:         // <9> UTC日期 ddmmyy
            switch (NMEA_MsgBlockDatIndex)
            {
            case 0: // dd
                GPS_RMC_Data.UTCDateTime[2]=(SBuf-'0')*10;
                break;
            case 1:
                GPS_RMC_Data.UTCDateTime[2]+=(SBuf-'0');
                if (GPS_RMC_Data.UTCDateTime[3]>23)     // 如果小时超过23, 则表明日期要+1
                {
                    GPS_RMC_Data.UTCDateTime[3]-=24;    // Hour
                    GPS_RMC_Data.UTCDateTime[2]++;      // Day
                }
                break;
            case 2: // mm
                GPS_RMC_Data.UTCDateTime[1]=(SBuf-'0')*10;
                break;
            case 3:
                GPS_RMC_Data.UTCDateTime[1]+=(SBuf-'0');
                switch (GPS_RMC_Data.UTCDateTime[1])    // 根据大小月来判断日期是否溢出, 溢出则月份++
                {
                case 2:                             // 此处未考虑闰年29天的情况
                    if (GPS_RMC_Data.UTCDateTime[2]>28)
                    {
                        GPS_RMC_Data.UTCDateTime[2]-=28;
                        GPS_RMC_Data.UTCDateTime[1]++;
                    }
                    break;
                case 1:                             // 大月 31 天
                case 3:
                case 5:
                case 7:
                case 8:
                case 10:
                case 12:
                    if (GPS_RMC_Data.UTCDateTime[2]>31)
                    {
                        GPS_RMC_Data.UTCDateTime[2]-=31;
                        GPS_RMC_Data.UTCDateTime[1]++;
                    }
                    break;
                case 4:                             // 小月 30 天
                case 6:
                case 9:
                case 11:
                    if (GPS_RMC_Data.UTCDateTime[2]>30)
                    {
                        GPS_RMC_Data.UTCDateTime[2]-=30;
                        GPS_RMC_Data.UTCDateTime[1]++;
                    }
                    break;
                }
                break;
            case 4:
                GPS_RMC_Data.UTCDateTime[0]=(SBuf-'0')*10;
                break;
            case 5:
                GPS_RMC_Data.UTCDateTime[0]+=(SBuf-'0');
                if (GPS_RMC_Data.UTCDateTime[1]>12)     // 如果月份超出, 则年 ++
                {
                    GPS_RMC_Data.UTCDateTime[1]=1;
                    GPS_RMC_Data.UTCDateTime[0]++;
                }
                break;
            }
            break;
        }
        NMEA_MsgBlockDatIndex++;  // 该字段字符序号 +1
    }
}

static void ParserGPGSA(char SBuf)
{
    switch (SBuf)
    {
    case '*':               // 语句数据结束, 紧跟2位校验值
        NMEA_Start=0;
		Parser_CallBack.gpgsaCallback(GPS_GSA_Data);
        break;
    case ',':               // ,分隔符, 字段 +1, 字段内部字符编号归零
        NMEA_MsgBlock++;
        NMEA_MsgBlockDatIndex=0;
        //此处必须确保 GPS_GSA_Data.SatUsedList[] 初始化.
        break;
    default:
        switch (NMEA_MsgBlock)
        {
        case 0:         // <1>模式 M=手动, A=自动
            GPS_GSA_Data.Mode=SBuf;
            break;
        case 1:         // <2>定位型式 1=未定位, 2=二维定位, 3=三维定位
            GPS_GSA_Data.Mode2=SBuf;
            break;
        case 2:         // <3> PRN 01~32 使用中的卫星编号
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型
            case 0:
                GPS_GSA_Data.SatUsedList[NMEA_MsgBlock-2]=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSA_Data.SatUsedList[NMEA_MsgBlock-2]+=(SBuf-'0');
                break;
            }
            break;
        case 14:        // <4> PDOP 位置精度因子 0.5~99.9
            GPS_GSA_Data.PDOP[NMEA_MsgBlockDatIndex]=SBuf;
            break;
        case 15:        // <5> HDOP 水平精度因子 0.5~99.9
            GPS_GSA_Data.HDOP[NMEA_MsgBlockDatIndex]=SBuf;
            break;
        case 16:        // <6> VDOP 垂直精度因子 0.5~99.9
            GPS_GSA_Data.VDOP[NMEA_MsgBlockDatIndex]=SBuf;
            break;
        }
        NMEA_MsgBlockDatIndex++;  // 该字段字符序号 +1
    }
}

static void ParserGPGSV(char SBuf)
{
    switch (SBuf)
    {
    case '*':               // 语句数据结束, 紧跟2位校验值
        NMEA_Start=0;
		SateInfoIndex=0;
		Parser_CallBack.gpgsvCallback(GPS_GSV_Data);
        break;
    case ',':               // ,分隔符, 字段 +1, 字段内部字符编号归零
        NMEA_MsgBlock++;
        NMEA_MsgBlockDatIndex=0;
        break;
    default:
        switch (NMEA_MsgBlock)
        {
            /*
            case 0:         // <1> GSV语句的总数
                break;
                */
        case 1:         // <2> 本句GSV的编号
            if (SBuf=='1') SateInfoIndex=0;
            //解析到第一句 GSV 语句 则判断卫星信息从新开始
            break;
        case 2:         // <3> 可见卫星的总数 00~12
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型
            case 0:
                GPS_GSV_Data.SatInView=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInView+=(SBuf-'0');
                break;
            }
            break;
        case 3:         // <4> 卫星编号 01~32
        case 7:
        case 11:
        case 15:
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex].SatID=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex].SatID+=(SBuf-'0');
                SateInfoIndex++;
                break;
            }
            break;
        case 4:         // <5>卫星仰角 00~90 度
        case 8:
        case 12:
        case 16:
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Elevation=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Elevation+=(SBuf-'0');
                break;
            }
            break;
        case 5:         // <6>卫星方位角 000~359 度
        case 9:
        case 13:
        case 17:
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Azimuth=(SBuf-'0')*100;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Azimuth+=(SBuf-'0')*10;
                break;
            case 2:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Azimuth+=(SBuf-'0');
                break;
            }
            break;
        case 6:         // <7>讯号噪声比 C/No 00~99
        case 10:
        case 14:
        case 18:
            switch (NMEA_MsgBlockDatIndex)
            {           // 前导0也会输出, 转换成数值型
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].SNR=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].SNR+=(SBuf-'0');
                break;
            }
            break;
        }
        NMEA_MsgBlockDatIndex++;  // 该字段字符序号 +1
    }
}
char dec[] ="326.22"; //326.22
static void ParserGPVTG(char SBuf) {
    switch (SBuf)
    {
        case '*':   //语句结束
            NMEA_Start=0;
			GPS_VTG_Data.dec = strtof(dec,NULL);
		    Parser_CallBack.gpvtgCallback(GPS_VTG_Data);
            break;
        case ',':   //该字段结束
            NMEA_MsgBlock++;
            NMEA_MsgBlockDatIndex=0;
            break;
        default:    //字段字符
            switch (NMEA_MsgBlock) { // 判断当前处于哪个字段
                case 0:              // <1> 运动角度，000 - 359，（前导位数不足则补0）
                     switch (NMEA_MsgBlockDatIndex) {
				        case 0:
					          break;
				        case 1:
							  break;
			        }
				    break;
            }
		    NMEA_MsgBlockDatIndex++;	 //字段字符索引++, 指向下一个字符
            break;
    }
}

u8 GPS_Parser(char SBuf) {
    u8 i;

    GPS_Parse_Status=0;

    if (NMEA_Start)

    {               // 艙芒脦枚碌艙脪脭$驴陋脢艗碌脛 NMEA 脫茂鸥盲, 艙酶脠毛NMEA 艙芒脦枚脕梅鲁脤:

        if (NMEA_TypeParsed)

        {           // NMEA 脫茂鸥盲脌脿脨脥艙芒脦枚脥锚卤脧, 啪霉鸥脻脌脿脨脥碌梅脫脙艙芒脦枚潞炉脢媒

            switch (NMEA_MsgType)

            {

            case NMEA_GPGGA:

                ParserGPGGA(SBuf);

                break;

            case NMEA_GPGSA:

                ParserGPGSA(SBuf);

                break;

            case NMEA_GPGSV:

                ParserGPGSV(SBuf);

                break;

            case NMEA_GPRMC:

                ParserGPRMC(SBuf);

                break;
            case NMEA_GPVTG:
				ParserGPVTG(SBuf);
				break;
            default:    //脦脼路拧脢露卤冒碌脛啪帽脢艙, 啪沤脦禄

                NMEA_Start=0;

                NMEA_TypeParsed=0;

                NMEA_MsgType=NMEA_NULL;

                NMEA_MsgTypeIndex=1;

            }

        }

        else

        {           // NMEA 脫茂鸥盲脌脿脨脥脦沤艙芒脦枚, 啪霉鸥脻脌脿脨脥碌梅脫脙艙芒脦枚潞炉脢媒

            switch (SBuf)

            {

            case ',':       // NMEA 脫茂鸥盲脌脿脨脥脳脰露脦艙谩脢酶,驴陋脢艗脜脨露脧

                // GPS 脢盲鲁枚脣鲁脨貌 - 0

                if (NMEA_MsgTypeBuff[3]=='G'&&NMEA_MsgTypeBuff[4]=='A')

                {

                    //鲁玫脢艗禄炉潞拢掳脦脢媒鸥脻



                    //鲁玫脢艗禄炉露拧脦禄脨脜脧垄脢媒鸥脻

                    //GPS_GGA_Data.PositionFix=0x00;

                    GPS_GGA_Data.SatUsed=0x00;

                    NMEA_MsgType=NMEA_GPGGA;

                }



                // GPS 脢盲鲁枚脣鲁脨貌 - 1

                if (NMEA_MsgTypeBuff[3]=='S'&&NMEA_MsgTypeBuff[4]=='A')

                {

                    //鲁玫脢艗禄炉脢鹿脫脙脰脨碌脛脦脌脨脟脕脨卤铆

                    for (i=0;i<12;i++)

                    {

                        GPS_GSA_Data.SatUsedList[i]=0x00;

                    }

                    //鲁玫脢艗禄炉鸥芦露脠脢媒鸥脻

                    for (i=0;i<5;i++)

                    {

                        GPS_GSA_Data.HDOP[i]=0x00;

                        GPS_GSA_Data.VDOP[i]=0x00;

                        GPS_GSA_Data.PDOP[i]=0x00;

                    }

                    //GPS_GSA_Data.Mode=0x00;

                    //GPS_GSA_Data.Mode2=0x00;



                    //脫脡脫脷GSV脫茂鸥盲脫脨脠媒鸥盲, 脧脗脕脨鲁玫脢艗禄炉虏禄脛脺路脜脫脷 GSV脰脨,

                    //路帽脭貌禄谩碌艗脰脗脟掳脕艙鸥盲艙芒脦枚碌艙碌脛脢媒鸥脻卤禄鲁玫脢艗禄炉碌么

                    //鲁玫脢艗禄炉脦脌脨脟脨脜潞脜路艙脦禄艙脟脢媒鸥脻

                    for (i=0;i<12;i++)

                    {

                        GPS_GSV_Data.SatInfo[i].SatID=0x00;

                        GPS_GSV_Data.SatInfo[i].SNR=0x00;

                        GPS_GSV_Data.SatInfo[i].Elevation=0x00;

                        GPS_GSV_Data.SatInfo[i].Azimuth=0x0000;

                    }

                    //GPS_GSV_Data.SatInView=0x00;

                    NMEA_MsgType=NMEA_GPGSA;

                }



                // GPS 脢盲鲁枚脣鲁脨貌 - 2,3,4

                if (NMEA_MsgTypeBuff[4]=='V')

                {

                    NMEA_MsgType=NMEA_GPGSV;

                }

                if(NMEA_MsgTypeBuff[3] == 'T' && NMEA_MsgTypeBuff[4] == 'G') {
                    NMEA_MsgType = NMEA_GPVTG;
				}

                // GPS 脢盲鲁枚脣鲁脨貌 - 5

                if (NMEA_MsgTypeBuff[4]=='C')

                {

                    //GPS_RMC_Data.Status='-';

                    //鲁玫脢艗禄炉鸥颅脦鲁露脠脢媒鸥脻潞脥脣脵露脠,潞艙脧貌

                    GPS_RMC_Data.LatitudeD=0x00;

                    GPS_RMC_Data.LatitudeM=0x00;

                    GPS_RMC_Data.LatitudeS=0x0000;

                    GPS_RMC_Data.NS='-';



                    GPS_RMC_Data.LongitudeD=0x00;

                    GPS_RMC_Data.LongitudeM=0x00;

                    GPS_RMC_Data.LongitudeS=0x0000;

                    GPS_RMC_Data.EW='-';



                    GPS_RMC_Data.Speed=0x0000;

                    GPS_RMC_Data.Course=0x0000;



                    NMEA_MsgType=NMEA_GPRMC;

                }

                //沤脣沤艩脠莽鹿没露艗虏禄鲁脡脕垄, 艗沤脫茂鸥盲虏禄卤禄脢露卤冒, 脭貌NMEA_MsgType脦陋NULL禄貌脝盲脣没,

                //脭貌脳陋脦陋啪霉鸥脻脌脿脨脥艙芒脦枚脢卤禄谩脤酶脳陋碌艙脦脼路拧脢露卤冒碌脛啪帽脢艙, 露酶潞贸啪沤脦禄

                NMEA_TypeParsed=1;

                NMEA_MsgTypeIndex=1;

                NMEA_MsgBlock=0;

                NMEA_MsgBlockDatIndex=0;

                break;

            case '*':

                NMEA_Start=0;

                //GPS脛拢驴茅脡脧碌莽脢卤脢盲鲁枚

                //$PSRF Model Name : J3S31_DGCB1_496 *45

                //$PSRF *321.3*30

                //$PSRF*17

                //$PSRF*17

                //$PSRF Product by J communications Co., Ltd *4C

                //$PSRF Revision by Young Wook *69

                //$PSRF www.jcomco.com *06

                //碌艗脰脗脜脨露脧脢搂脨搂,

                break;

            default:        //沤艩脫脷碌脷脪禄啪枚脳脰露脦脰脨, 艗脤脨酶艙脫脢脮

                NMEA_MsgTypeBuff[NMEA_MsgTypeIndex]=SBuf;

                NMEA_MsgTypeIndex++;

                if (NMEA_MsgTypeIndex>5) NMEA_Start=0;

                // NMEA 脌脿脨脥鲁卢鹿媒 5 啪枚脳脰路没, (脢媒脳茅脭艙艙莽, 碌艗脰脗脣脌禄煤)

                // 脭貌脜脨露脧虏禄脢脟脮媒鲁拢碌脛 NMEA 脫茂鸥盲, 脭貌脗脭鹿媒沤脣鸥盲.

            }

        }

    }

    else

    {               //脦沤艙芒脦枚碌艙$, 脩颅禄路艙脫脢脮虏垄脜脨露脧 脰卤碌艙 $

        if (SBuf=='$')

        {           //艙脫脢脮碌艙$, 脧脗脪禄啪枚脳脰路没艗沤脦陋脌脿脨脥脜脨露脧脳脰路没, 脧脠艙酶脨脨脧脿鹿脴卤盲脕驴鲁玫脢艗禄炉

            NMEA_Start = 1;         //脧脗沤脦碌梅脫脙脭貌艙酶脠毛NMEA 艙芒脦枚脕梅鲁脤:

            NMEA_MsgTypeIndex = 0;  //沤脫脥路沤忙路脜GPS脌脿脨脥脳脰路没碌艙卤盲脕驴

            NMEA_TypeParsed = 0;

            NMEA_MsgType = NMEA_NULL;

            NMEA_MsgBlock = 0;

            NMEA_MsgBlockDatIndex = 0;

        }

    }



    return GPS_Parse_Status;

}
void initParserCallBack(GPRMC_CALLBACK gprmcCallback,GPGGA_CALLBACK gpggaCallback,
	                       GPGSA_CALLBACK gpgsaCallback,GPGSV_CALLBACK gpgsvCallback,
	                       GPVTG_CALLBACK gpVTGCallback){
	Parser_CallBack.gprmcCallback = gprmcCallback;
	Parser_CallBack.gpggaCallback = gpggaCallback;
	Parser_CallBack.gpgsaCallback = gpgsaCallback;
	Parser_CallBack.gpgsvCallback = gpgsvCallback;
	Parser_CallBack.gpvtgCallback = gpVTGCallback;
}
int _nmea_parse_time(const char *buff, int buff_sz, nmeaTIME *res)
{
    int success = 0;

    switch(buff_sz)
    {
    case sizeof("hhmmss") - 1:
        success = (3 == nmea_scanf(buff, buff_sz,
            "%2d%2d%2d", &(res->hour), &(res->min), &(res->sec)
            ));
        break;
    case sizeof("hhmmss.s") - 1:
    case sizeof("hhmmss.ss") - 1:
    case sizeof("hhmmss.sss") - 1:
        success = (4 == nmea_scanf(buff, buff_sz,
            "%2d%2d%2d.%d", &(res->hour), &(res->min), &(res->sec), &(res->hsec)
            ));
        break;
    default:
        printf("Parse of time error (format error)!");
        success = 0;
        break;
    }

    return (success?0:-1);
}

/**
 * \brief Parse GGA packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPGGA(const char *buff, int buff_sz, nmeaGPGGA *pack)
{
    char time_buff[16];

    memset(pack, 0, sizeof(nmeaGPGGA));

    if(14 != nmea_scanf(buff, buff_sz,
        "$GPGGA,%s,%f,%C,%f,%C,%d,%d,%f,%f,%C,%f,%C,%f,%d*",
        &(time_buff[0]),
        &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->sig), &(pack->satinuse), &(pack->HDOP), &(pack->elv), &(pack->elv_units),
        &(pack->diff), &(pack->diff_units), &(pack->dgps_age), &(pack->dgps_sid)))
    {
        printf("GPGGA parse error!");
        return 0;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int)strlen(&time_buff[0]), &(pack->utc)))
    {
        printf("GPGGA time parse error!");
        return 0;
    }

    return 1;
}

/**
 * \brief Parse RMC packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPRMC(const char *buff, int buff_sz, nmeaGPRMC *pack)
{
    int nsen;
    char time_buff[16];

    memset(pack, 0, sizeof(nmeaGPRMC));

    nsen = nmea_scanf(buff, buff_sz,
        "$GPRMC,%s,%c,%f,%c,%f,%c,%f,%f,%2d%2d%2d,%f,%c,%c*",
        &(time_buff[0]),
        &(pack->status), &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->speed), &(pack->direction),
        &(pack->utc.day), &(pack->utc.mon), &(pack->utc.year),
        &(pack->declination), &(pack->declin_ew), &(pack->mode));

    if(nsen != 13 && nsen != 14)
    {
        printf("GPRMC parse error!\n");
        return 0;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int)strlen(&time_buff[0]), &(pack->utc)))
    {
        printf("GPRMC time parse error!\n");
        return 0;
    }

    if(pack->utc.year < 90)
        pack->utc.year += 100;
    pack->utc.mon -= 1;

    return 1;
}

/**
 * \brief Parse GSA packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPGSA(const char *buff, int buff_sz, nmeaGPGSA *pack)
{
    memset(pack, 0, sizeof(nmeaGPGSA));

    if(17 != nmea_scanf(buff, buff_sz,
        "$GPGSA,%C,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f*",
        &(pack->fix_mode), &(pack->fix_type),
        &(pack->sat_prn[0]), &(pack->sat_prn[1]), &(pack->sat_prn[2]), &(pack->sat_prn[3]), &(pack->sat_prn[4]), &(pack->sat_prn[5]),
        &(pack->sat_prn[6]), &(pack->sat_prn[7]), &(pack->sat_prn[8]), &(pack->sat_prn[9]), &(pack->sat_prn[10]), &(pack->sat_prn[11]),
        &(pack->PDOP), &(pack->HDOP), &(pack->VDOP)))
    {
        printf("GPGSA parse error!");
        return 0;
    }

    return 1;
}

/**
 * \brief Parse GSV packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPGSV(const char *buff, int buff_sz, nmeaGPGSV *pack)
{
    int nsen, nsat;

    memset(pack, 0, sizeof(nmeaGPGSV));

    nsen = nmea_scanf(buff, buff_sz,
        "$GPGSV,%d,%d,%d,"
        "%d,%d,%d,%d,"
        "%d,%d,%d,%d,"
        "%d,%d,%d,%d,"
        "%d,%d,%d,%d*",
        &(pack->pack_count), &(pack->pack_index), &(pack->sat_count),
        &(pack->sat_data[0].id), &(pack->sat_data[0].elv), &(pack->sat_data[0].azimuth), &(pack->sat_data[0].sig),
        &(pack->sat_data[1].id), &(pack->sat_data[1].elv), &(pack->sat_data[1].azimuth), &(pack->sat_data[1].sig),
        &(pack->sat_data[2].id), &(pack->sat_data[2].elv), &(pack->sat_data[2].azimuth), &(pack->sat_data[2].sig),
        &(pack->sat_data[3].id), &(pack->sat_data[3].elv), &(pack->sat_data[3].azimuth), &(pack->sat_data[3].sig));

    nsat = (pack->pack_index - 1) * NMEA_SATINPACK;
    nsat = (nsat + NMEA_SATINPACK > pack->sat_count)?pack->sat_count - nsat:NMEA_SATINPACK;
    nsat = nsat * 4 + 3 /* first three sentence`s */;

    if(nsen < nsat || nsen > (NMEA_SATINPACK * 4 + 3))
    {
        printf("GPGSV parse error!");
        return 0;
    }

    return 1;
}

/**
 * \brief Parse VTG packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPVTG(const char *buff, int buff_sz, nmeaGPVTG *pack)
{
    memset(pack, 0, sizeof(nmeaGPVTG));

    if(8 != nmea_scanf(buff, buff_sz,
        "$GPVTG,%f,%C,%f,%C,%f,%C,%f,%C*",
        &(pack->dir), &(pack->dir_t),
        &(pack->dec), &(pack->dec_m),
        &(pack->spn), &(pack->spn_n),
        &(pack->spk), &(pack->spk_k)))
    {
        printf("GPVTG parse error!");
        return 0;
    }

    if( pack->dir_t != 'T' ||
        pack->dec_m != 'M' ||
        pack->spn_n != 'N' ||
        pack->spk_k != 'K')
    {
        printf("GPVTG parse error (format error)!");
        return 0;
    }

    return 1;
}


/**
 * \brief Define packet type by header (nmeaPACKTYPE).
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @return The defined packet type
 * @see nmeaPACKTYPE
 */
int nmea_pack_type(const char *buff, int buff_sz)
{
    static const char *pheads[] = {
        "GPGGA",
        "GPGSA",
        "GPGSV",
        "GPRMC",
        "GPVTG",
        
        "GPTXT",

        "BDGSV",
        "BDGSA",

        "GNZDA",
        "GNGLL",
        "GNVTG",
        "GNRMC",
        "GNGGA",
        
        "GPGLL",
    };

    if(buff_sz < 5)
        return GPNON;
    else if(0 == memcmp(buff, pheads[0], 5))
        return GPGGA;
    else if(0 == memcmp(buff, pheads[1], 5))
        return GPGSA;
    else if(0 == memcmp(buff, pheads[2], 5))
        return GPGSV;
    else if(0 == memcmp(buff, pheads[3], 5))
        return GPRMC;
    else if(0 == memcmp(buff, pheads[4], 5))
        return GPVTG;

    else if(0 == memcmp(buff, pheads[5], 5))
        return GPTXT;
    else if(0 == memcmp(buff, pheads[6], 5))
        return BDGSV;
    else if(0 == memcmp(buff, pheads[7], 5))
        return BDGSA;
    else if(0 == memcmp(buff, pheads[8], 5))
        return GNZDA;
    else if(0 == memcmp(buff, pheads[9], 5))
        return GNGLL;
    else if(0 == memcmp(buff, pheads[10], 5))
        return GNVTG;
    else if(0 == memcmp(buff, pheads[11], 5))
        return GNRMC;
    else if(0 == memcmp(buff, pheads[12], 5))
        return GNGGA;
    else if(0 == memcmp(buff, pheads[13], 5))
        return GPGLL;


    return GPNON;
}
/*   Header             ID          Length      Payload    Checksum
 *  0xB5 0x62    0x01 0x02       28                         CK_A CK_B
*/
int ubxhAcc(const char *buff, int buff_sz) {
   int hAcc = 0;  //unit mm
   /*
     u8 buf[36] = {
         0xB5,0x62,0x01,0x02,0x1C,0x00,0xB8,0x14,0x60,0x0F,0x35,0x44,0xE7,0x43,0x76,0x5E,
         0x80,0x0D,0xF1,0xE8,0x00,0x00,0x3C,0xF4,0x00,0x00,0x91,0x73,0x00,0x00,0xF1,0xE6,
         0x03,0x00,0x45,0xB2};
     */
   hAcc = buff[26]|buff[27]<<8|buff[28]<<16|buff[29]<<24;
   
   //printf("hAcc = %.1f\n",(double)hAcc/1000);
   return hAcc;
}


