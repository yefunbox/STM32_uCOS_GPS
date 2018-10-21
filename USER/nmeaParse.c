/*-------------------------------------------------------

      GPS œâÎöÄ£¿é By wowbanui



 °æ±ŸÀúÊ·:

 Rev.2010--     ³õÊŒ°æ±Ÿ

 Rev.201103     1. ÓÅ»¯œá¹¹ÊýŸÝ, ²¿·ÖÊýŸÝÖ±œÓŽŠÀí³ÉÊýÖµ,ŒõÉÙÄÚŽæÕŒÓÃ.

                2. Êä³ö²¿·ÖÖ±œÓµ÷ÓÃLCDÃüÁî,ÒÆÖ²ÐèžüžÄ

 Rev.201107     1. ÐÞžÄŸ­Î³¶ÈµÈÎªÊýÖµÐÎÊœ

                2. ÐÞžÄ³õÊŒ»¯ÊýŸÝ·œ·š



---------------------------------------------------------*/

#include "nmeaParse.h"

#include <stdio.h>

#include <stdlib.h>





static u8   NMEA_Start = 0;                 // NMEA ÓïŸä¿ªÊŒ. Œì²âµœ $ Ê±ÖÃ 1

static u8   NMEA_TypeParsed = 0;            // NMEA ÀàÐÍœâÎöÍê±Ï

static u8   NMEA_MsgType = NMEA_NULL;       // NMEA ÓïŸäÀàÐÍ

static char NMEA_MsgTypeBuff[] = "GPxxx,";  // NMEA ÓïŸäÀàÐÍÊ¶±ð»ºŽæ

static u8   NMEA_MsgTypeIndex = 0;          // ¶ÁÈ¡ NMEA ÀàÐÍÊ¶±ð×Ö·ûµÄžöÊý

static u8   NMEA_MsgBlock = 0;              // NMEA ÊýŸÝ×Ö¶ÎºÅ ŽÓ0¿ªÊŒ

static u8   NMEA_MsgBlockDatIndex = 0;      // NMEA ÊýŸÝÃ¿žö×Ö¶ÎÄÚ×Ö·ûË÷Òý ŽÓ0¿ªÊŒ



static u8   GPS_Parse_Status = 0;           // µ±Ç°œâÎö×ŽÌ¬.

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
        case '*':   //������
                 NMEA_Start=0;
		         Parser_CallBack.gpggaCallback(GPS_GGA_Data);
                 break;
        case ',':   //���ֶν���
                 NMEA_MsgBlock++;
                 NMEA_MsgBlockDatIndex=0;
                 break;
        default:    //�ֶ��ַ�
                switch (NMEA_MsgBlock)  // �жϵ�ǰ�����ĸ��ֶ�
                {
			            /*
			            case 0:             // <1> UTCʱ��,hhmmss
			                break;
			            case 1:             // <2> γ�� ddmm.mmmm
			                break;
			            case 2:             // <3> γ�Ȱ��� N/S
			                break;
			            case 3:             // <4> ���� dddmm.mmmm
			                break;
			            case 4:             // <5> ���Ȱ��� E/W
			                break;
			            */
                    case 5:      // <6> GPS״̬ 0=δ��λ, 1=�ǲ�ֶ�λ, 2=��ֶ�λ, 6=���ڹ���
                           GPS_GGA_Data.PositionFix=SBuf;
                           break;
                    case 6:      // <7> ����ʹ�õ��������� 00~12
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
                                    case 7:             //<8> HDOPˮƽ�������� 0.5~99.9
                                            GPS_GGA_Data.HDOP[GPS_GGA_Data.BlockIndex]=SBuf;
                                            break;
                                   */
                     case 8:         //<9> ���θ߶� -9999.9~99999.9
                           GPS_GGA_Data.Altitude[NMEA_MsgBlockDatIndex]=SBuf;
                           break;
                  }
        NMEA_MsgBlockDatIndex++;     //�ֶ��ַ�����++, ָ����һ���ַ�
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
        GPS_Parse_Status=GPS_PARSE_OK;       //�������, ���Դ���
        Parser_CallBack.gprmcCallback(GPS_RMC_Data);
        break;
    case ',':
        NMEA_MsgBlock++;
        NMEA_MsgBlockDatIndex=0;
        break;
    default:
        switch (NMEA_MsgBlock)
        {
        case 0:         // <1> UTCʱ�� hhmmss.mmm
            switch (NMEA_MsgBlockDatIndex)
            {
            case 0: // hh
                GPS_RMC_Data.UTCDateTime[3]=(SBuf-'0')*10;
                break;
            case 1:
                GPS_RMC_Data.UTCDateTime[3]+=(SBuf-'0');
                GPS_RMC_Data.UTCDateTime[3]+=8;         // Դʱ���� UTC, ת���ɱ���ʱ�� +8, ����Ҫ�ж��Ƿ񳬹�23Сʱ
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
        case 1:         // <2> ��λ״̬ A=��Ч��λ, V=��Ч��λ
            GPS_RMC_Data.Status=SBuf;
            break;
        case 2:         // <3> γ�� ddmm.mmmm
            //GPS_RMC_Data.Latitude[NMEA_MsgBlockDatIndex]=SBuf;    //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, �ֱ�ת������ֵ��
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
            case 4:     // С����
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
        case 3:         //<4> γ�Ȱ��� N/S
            GPS_RMC_Data.NS=SBuf;
            break;
        case 4:         //<5> ���� dddmm.mmmm
            //GPS_RMC_Data.Longitude[NMEA_MsgBlockDatIndex]=SBuf;   //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, �ֱ�ת������ֵ��
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
            case 5:     // С����
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
        case 5:         // <6> ���Ȱ��� E/W
            GPS_RMC_Data.EW=SBuf;
            break;
        case 6:         // <7> �������� 000.0~999.9 ��
            //GPS_RMC_Data.sSpeed[NMEA_MsgBlockDatIndex]=SBuf;  //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��, ��ֵx10
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
        case 7:         // <8> ���溽�� 000.0~359.9 ��, ���汱Ϊ�ο���׼
            //GPS_RMC_Data.sCourse[NMEA_MsgBlockDatIndex]=SBuf; //DEBUG
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��, ��ֵx10
            case 0:
                GPS_RMC_Data.Course=(SBuf-'0')*1000;
                break;
            case 1:
                GPS_RMC_Data.Course+=(SBuf-'0')*100;
                break;
            case 2:
                GPS_RMC_Data.Course+=(SBuf-'0')*10;
                break;
            case 3: // С����, ����
                break;
            case 4:
                GPS_RMC_Data.Course+=(SBuf-'0');
                break;
            }
            break;
        case 8:         // <9> UTC���� ddmmyy
            switch (NMEA_MsgBlockDatIndex)
            {
            case 0: // dd
                GPS_RMC_Data.UTCDateTime[2]=(SBuf-'0')*10;
                break;
            case 1:
                GPS_RMC_Data.UTCDateTime[2]+=(SBuf-'0');
                if (GPS_RMC_Data.UTCDateTime[3]>23)     // ���Сʱ����23, ���������Ҫ+1
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
                switch (GPS_RMC_Data.UTCDateTime[1])    // ���ݴ�С�����ж������Ƿ����, ������·�++
                {
                case 2:                             // �˴�δ��������29������
                    if (GPS_RMC_Data.UTCDateTime[2]>28)
                    {
                        GPS_RMC_Data.UTCDateTime[2]-=28;
                        GPS_RMC_Data.UTCDateTime[1]++;
                    }
                    break;
                case 1:                             // ���� 31 ��
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
                case 4:                             // С�� 30 ��
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
                if (GPS_RMC_Data.UTCDateTime[1]>12)     // ����·ݳ���, ���� ++
                {
                    GPS_RMC_Data.UTCDateTime[1]=1;
                    GPS_RMC_Data.UTCDateTime[0]++;
                }
                break;
            }
            break;
        }
        NMEA_MsgBlockDatIndex++;  // ���ֶ��ַ���� +1
    }
}

static void ParserGPGSA(char SBuf)
{
    switch (SBuf)
    {
    case '*':               // ������ݽ���, ����2λУ��ֵ
        NMEA_Start=0;
		Parser_CallBack.gpgsaCallback(GPS_GSA_Data);
        break;
    case ',':               // ,�ָ���, �ֶ� +1, �ֶ��ڲ��ַ���Ź���
        NMEA_MsgBlock++;
        NMEA_MsgBlockDatIndex=0;
        //�˴�����ȷ�� GPS_GSA_Data.SatUsedList[] ��ʼ��.
        break;
    default:
        switch (NMEA_MsgBlock)
        {
        case 0:         // <1>ģʽ M=�ֶ�, A=�Զ�
            GPS_GSA_Data.Mode=SBuf;
            break;
        case 1:         // <2>��λ��ʽ 1=δ��λ, 2=��ά��λ, 3=��ά��λ
            GPS_GSA_Data.Mode2=SBuf;
            break;
        case 2:         // <3> PRN 01~32 ʹ���е����Ǳ��
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
            {           // ǰ��0Ҳ�����, ת������ֵ��
            case 0:
                GPS_GSA_Data.SatUsedList[NMEA_MsgBlock-2]=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSA_Data.SatUsedList[NMEA_MsgBlock-2]+=(SBuf-'0');
                break;
            }
            break;
        case 14:        // <4> PDOP λ�þ������� 0.5~99.9
            GPS_GSA_Data.PDOP[NMEA_MsgBlockDatIndex]=SBuf;
            break;
        case 15:        // <5> HDOP ˮƽ�������� 0.5~99.9
            GPS_GSA_Data.HDOP[NMEA_MsgBlockDatIndex]=SBuf;
            break;
        case 16:        // <6> VDOP ��ֱ�������� 0.5~99.9
            GPS_GSA_Data.VDOP[NMEA_MsgBlockDatIndex]=SBuf;
            break;
        }
        NMEA_MsgBlockDatIndex++;  // ���ֶ��ַ���� +1
    }
}

static void ParserGPGSV(char SBuf)
{
    switch (SBuf)
    {
    case '*':               // ������ݽ���, ����2λУ��ֵ
        NMEA_Start=0;
		SateInfoIndex=0;
		Parser_CallBack.gpgsvCallback(GPS_GSV_Data);
        break;
    case ',':               // ,�ָ���, �ֶ� +1, �ֶ��ڲ��ַ���Ź���
        NMEA_MsgBlock++;
        NMEA_MsgBlockDatIndex=0;
        break;
    default:
        switch (NMEA_MsgBlock)
        {
            /*
            case 0:         // <1> GSV��������
                break;
                */
        case 1:         // <2> ����GSV�ı��
            if (SBuf=='1') SateInfoIndex=0;
            //��������һ�� GSV ��� ���ж�������Ϣ���¿�ʼ
            break;
        case 2:         // <3> �ɼ����ǵ����� 00~12
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��
            case 0:
                GPS_GSV_Data.SatInView=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInView+=(SBuf-'0');
                break;
            }
            break;
        case 3:         // <4> ���Ǳ�� 01~32
        case 7:
        case 11:
        case 15:
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex].SatID=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex].SatID+=(SBuf-'0');
                SateInfoIndex++;
                break;
            }
            break;
        case 4:         // <5>�������� 00~90 ��
        case 8:
        case 12:
        case 16:
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Elevation=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].Elevation+=(SBuf-'0');
                break;
            }
            break;
        case 5:         // <6>���Ƿ�λ�� 000~359 ��
        case 9:
        case 13:
        case 17:
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��
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
        case 6:         // <7>Ѷ�������� C/No 00~99
        case 10:
        case 14:
        case 18:
            switch (NMEA_MsgBlockDatIndex)
            {           // ǰ��0Ҳ�����, ת������ֵ��
            case 0:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].SNR=(SBuf-'0')*10;
                break;
            case 1:
                GPS_GSV_Data.SatInfo[SateInfoIndex-1].SNR+=(SBuf-'0');
                break;
            }
            break;
        }
        NMEA_MsgBlockDatIndex++;  // ���ֶ��ַ���� +1
    }
}
char dec[] ="326.22"; //326.22
static void ParserGPVTG(char SBuf) {
    switch (SBuf)
    {
        case '*':   //������
            NMEA_Start=0;
			GPS_VTG_Data.dec = strtof(dec,NULL);
		    Parser_CallBack.gpvtgCallback(GPS_VTG_Data);
            break;
        case ',':   //���ֶν���
            NMEA_MsgBlock++;
            NMEA_MsgBlockDatIndex=0;
            break;
        default:    //�ֶ��ַ�
            switch (NMEA_MsgBlock) { // �жϵ�ǰ�����ĸ��ֶ�
                case 0:              // <1> �˶��Ƕȣ�000 - 359����ǰ��λ��������0��
                     switch (NMEA_MsgBlockDatIndex) {
				        case 0:
					          break;
				        case 1:
							  break;
			        }
				    break;
            }
		    NMEA_MsgBlockDatIndex++;	 //�ֶ��ַ�����++, ָ����һ���ַ�
            break;
    }
}

u8 GPS_Parser(char SBuf) {
    u8 i;

    GPS_Parse_Status=0;

    if (NMEA_Start)

    {               // œâÎöµœÒÔ$¿ªÊŒµÄ NMEA ÓïŸä, œøÈëNMEA œâÎöÁ÷³Ì:

        if (NMEA_TypeParsed)

        {           // NMEA ÓïŸäÀàÐÍœâÎöÍê±Ï, žùŸÝÀàÐÍµ÷ÓÃœâÎöº¯Êý

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
            default:    //ÎÞ·šÊ¶±ðµÄžñÊœ, žŽÎ»

                NMEA_Start=0;

                NMEA_TypeParsed=0;

                NMEA_MsgType=NMEA_NULL;

                NMEA_MsgTypeIndex=1;

            }

        }

        else

        {           // NMEA ÓïŸäÀàÐÍÎŽœâÎö, žùŸÝÀàÐÍµ÷ÓÃœâÎöº¯Êý

            switch (SBuf)

            {

            case ',':       // NMEA ÓïŸäÀàÐÍ×Ö¶ÎœáÊø,¿ªÊŒÅÐ¶Ï

                // GPS Êä³öË³Ðò - 0

                if (NMEA_MsgTypeBuff[3]=='G'&&NMEA_MsgTypeBuff[4]=='A')

                {

                    //³õÊŒ»¯º£°ÎÊýŸÝ



                    //³õÊŒ»¯¶šÎ»ÐÅÏ¢ÊýŸÝ

                    //GPS_GGA_Data.PositionFix=0x00;

                    GPS_GGA_Data.SatUsed=0x00;

                    NMEA_MsgType=NMEA_GPGGA;

                }



                // GPS Êä³öË³Ðò - 1

                if (NMEA_MsgTypeBuff[3]=='S'&&NMEA_MsgTypeBuff[4]=='A')

                {

                    //³õÊŒ»¯Ê¹ÓÃÖÐµÄÎÀÐÇÁÐ±í

                    for (i=0;i<12;i++)

                    {

                        GPS_GSA_Data.SatUsedList[i]=0x00;

                    }

                    //³õÊŒ»¯Ÿ«¶ÈÊýŸÝ

                    for (i=0;i<5;i++)

                    {

                        GPS_GSA_Data.HDOP[i]=0x00;

                        GPS_GSA_Data.VDOP[i]=0x00;

                        GPS_GSA_Data.PDOP[i]=0x00;

                    }

                    //GPS_GSA_Data.Mode=0x00;

                    //GPS_GSA_Data.Mode2=0x00;



                    //ÓÉÓÚGSVÓïŸäÓÐÈýŸä, ÏÂÁÐ³õÊŒ»¯²»ÄÜ·ÅÓÚ GSVÖÐ,

                    //·ñÔò»áµŒÖÂÇ°ÁœŸäœâÎöµœµÄÊýŸÝ±»³õÊŒ»¯µô

                    //³õÊŒ»¯ÎÀÐÇÐÅºÅ·œÎ»œÇÊýŸÝ

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



                // GPS Êä³öË³Ðò - 2,3,4

                if (NMEA_MsgTypeBuff[4]=='V')

                {

                    NMEA_MsgType=NMEA_GPGSV;

                }

                if(NMEA_MsgTypeBuff[3] == 'T' && NMEA_MsgTypeBuff[4] == 'G') {
                    NMEA_MsgType = NMEA_GPVTG;
				}

                // GPS Êä³öË³Ðò - 5

                if (NMEA_MsgTypeBuff[4]=='C')

                {

                    //GPS_RMC_Data.Status='-';

                    //³õÊŒ»¯Ÿ­Î³¶ÈÊýŸÝºÍËÙ¶È,ºœÏò

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

                //ŽËŽŠÈç¹û¶Œ²»³ÉÁ¢, ŒŽÓïŸä²»±»Ê¶±ð, ÔòNMEA_MsgTypeÎªNULL»òÆäËû,

                //Ôò×ªÎªžùŸÝÀàÐÍœâÎöÊ±»áÌø×ªµœÎÞ·šÊ¶±ðµÄžñÊœ, ¶øºóžŽÎ»

                NMEA_TypeParsed=1;

                NMEA_MsgTypeIndex=1;

                NMEA_MsgBlock=0;

                NMEA_MsgBlockDatIndex=0;

                break;

            case '*':

                NMEA_Start=0;

                //GPSÄ£¿éÉÏµçÊ±Êä³ö

                //$PSRF Model Name : J3S31_DGCB1_496 *45

                //$PSRF *321.3*30

                //$PSRF*17

                //$PSRF*17

                //$PSRF Product by J communications Co., Ltd *4C

                //$PSRF Revision by Young Wook *69

                //$PSRF www.jcomco.com *06

                //µŒÖÂÅÐ¶ÏÊ§Ð§,

                break;

            default:        //ŽŠÓÚµÚÒ»žö×Ö¶ÎÖÐ, ŒÌÐøœÓÊÕ

                NMEA_MsgTypeBuff[NMEA_MsgTypeIndex]=SBuf;

                NMEA_MsgTypeIndex++;

                if (NMEA_MsgTypeIndex>5) NMEA_Start=0;

                // NMEA ÀàÐÍ³¬¹ý 5 žö×Ö·û, (Êý×éÔœœç, µŒÖÂËÀ»ú)

                // ÔòÅÐ¶Ï²»ÊÇÕý³£µÄ NMEA ÓïŸä, ÔòÂÔ¹ýŽËŸä.

            }

        }

    }

    else

    {               //ÎŽœâÎöµœ$, Ñ­»·œÓÊÕ²¢ÅÐ¶Ï Ö±µœ $

        if (SBuf=='$')

        {           //œÓÊÕµœ$, ÏÂÒ»žö×Ö·ûŒŽÎªÀàÐÍÅÐ¶Ï×Ö·û, ÏÈœøÐÐÏà¹Ø±äÁ¿³õÊŒ»¯

            NMEA_Start = 1;         //ÏÂŽÎµ÷ÓÃÔòœøÈëNMEA œâÎöÁ÷³Ì:

            NMEA_MsgTypeIndex = 0;  //ŽÓÍ·Žæ·ÅGPSÀàÐÍ×Ö·ûµœ±äÁ¿

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


