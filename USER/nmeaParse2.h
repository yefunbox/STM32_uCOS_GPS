#ifndef __NMEAR_PARSE2_H
#define __NMEAR_PARSE2_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

//GPS NMEA-0183Э����Ҫ�����ṹ�嶨�� 
//������Ϣ
__packed typedef struct  
{                                           
    u8 num;     //���Ǳ��
    u8 eledeg;  //��������
    u16 azideg; //���Ƿ�λ��
    u8 sn;      //�����          
}nmea_slmsg;  
//UTCʱ����Ϣ
__packed typedef struct  
{                                           
    u16 year;   //���
    u8 month;   //�·�
    u8 date;    //����
    u8 hour;    //Сʱ
    u8 min;     //����
    u8 sec;     //����
}nmea_utc_time;        
//NMEA 0183 Э����������ݴ�Žṹ��
__packed typedef struct  
{                                           
    u8 svnum;                   //�ɼ�������
    nmea_slmsg slmsg[12];       //���12������
    nmea_utc_time utc;          //UTCʱ��
    u32 latitude;               //γ�� ������100000��,ʵ��Ҫ����100000
    u8 state;
    u8 nshemi;                  //��γ/��γ,N:��γ;S:��γ                 
    u32 longitude;              //���� ������100000��,ʵ��Ҫ����100000
    u8 ewhemi;                  //����/����,E:����;W:����
    u8 gpssta;                  //GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.                  
    u8 posslnum;                //���ڶ�λ��������,0~12.
    u8 possl[12];               //���ڶ�λ�����Ǳ��
    u8 fixmode;                 //��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
    u16 pdop;                   //λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
    u16 hdop;                   //ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
    u16 vdop;                   //��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0 

    int altitude;               //���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m     
    u16 speed;                  //��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ  
    double  dir;                /**< True track made good (degrees) */
    char    dir_t;              /**< Fixed text 'T' indicates that track made good is relative to true north */
}nmea_msg; 

#endif
