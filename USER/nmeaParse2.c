#include "nmeaParse2.h"
#include <stdio.h>
#include <stdlib.h>

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号                             
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{               
    u8 *p=buf;
    while(cx)
    {        
        if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;   //返回差值，
}
//m^n函数
//返回值:m^n次方.
u32 NMEA_Pow(u8 m,u8 n)
{
    u32 result=1;    
    while(n--)result*=m;    
    return result;
}

//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(u8 *buf,u8*dx)
{
    u8 *p=buf;
    u32 ires=0,fres=0;
    u8 ilen=0,flen=0,i;
    u8 mask=0;
    int res;
    while(1) //得到整数和小数的长度
    {
        if(*p=='-'){mask|=0X02;p++;}//是负数
        if(*p==','||(*p=='*'))break;//遇到结束了
        if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
        else if(*p>'9'||(*p<'0'))   //有非法字符
        {   
            ilen=0;
            flen=0;
            break;
        }   
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++; //去掉负号
    for(i=0;i<ilen;i++) //得到整数部分数据
    {  
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;   //最多取5位小数
    *dx=flen;           //小数点位数
    for(i=0;i<flen;i++) //得到小数部分数据
    {  
        fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    } 
    res=ires*NMEA_Pow(10,flen)+fres;
    if(mask&0X02)res=-res;         
    return res;
}
//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p,*p1,dx;
    u8 len,i,j,slx=0;
    u8 posx;     
    p=buf;
    p1=(u8*)strstr((const char *)p,"$GPGSV");//strstr判断$GPGSV是否是p数组的子串，是则返回$GPGSV中首先出现的地址，
    len=p1[7]-'0';                              //得到GPGSV的条数，p1[7]表示，后面的第一个字符。
    posx=NMEA_Comma_Pos(p1,3);                  //得到可见卫星总数，既将‘，’后面的字符里第一个字符的差值的到。
    if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);//p1+posx 得到可见卫星总数的指针，
    for(i=0;i<len;i++)
    {    
        p1=(u8*)strstr((const char *)p,"$GPGSV");  
        for(j=0;j<4;j++)
        {     
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);   //得到卫星编号
            else break; 
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
            else break; 
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);    //得到卫星信噪比
            else break;
            slx++;     
        }   
        p=p1+1;//切换到下一个GPGSV信息
    }   
}
//分析GPRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;           
    u8 posx;     
    u32 temp;      
    float rs;  
    p1=(u8*)strstr((const char *)buf,"GPRMC");//"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
    posx=NMEA_Comma_Pos(p1,1);                              //得到UTC时间
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);     //得到UTC时间(hhmmss),去掉ms
        gpsx->utc.hour=temp/10000;
        gpsx->utc.min=(temp/100)%100;
        gpsx->utc.sec=temp%100;      
    }   
	posx=NMEA_Comma_Pos(p1,2);
	if(posx!=0XFF) {
		gpsx->state = p1[posx];      //定位状态，A = 有效定位，B = 无效定位
	}
    posx=NMEA_Comma_Pos(p1,3);                              //得到纬度
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);          
        gpsx->latitude=temp/NMEA_Pow(10,dx+2);  //得到°
        rs=temp%NMEA_Pow(10,dx+2);              //得到'        
        gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
    }
    posx=NMEA_Comma_Pos(p1,4);                              //南纬还是北纬 
    if(posx!=0XFF)gpsx->nshemi=*(p1+posx);                   
    posx=NMEA_Comma_Pos(p1,5);                              //得到经度
    if(posx!=0XFF)
    {                                                 
        temp=NMEA_Str2num(p1+posx,&dx);          
        gpsx->longitude=temp/NMEA_Pow(10,dx+2); //得到°
        rs=temp%NMEA_Pow(10,dx+2);              //得到'        
        gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
    }
    posx=NMEA_Comma_Pos(p1,6);                              //东经还是西经
    if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);       
    posx=NMEA_Comma_Pos(p1,9);                              //得到UTC日期
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);                     //得到UTC日期
        gpsx->utc.date=temp/10000;
        gpsx->utc.month=(temp/100)%100;
        gpsx->utc.year=2000+temp%100;        
    } 
}
//分析GPVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;           
    u8 posx;
    u32 temp;

    p1=(u8*)strstr((const char *)buf,"$GPVTG");
    posx=NMEA_Comma_Pos(p1,1);                              //得到地面速率
    if(posx!=0XFF) {
       gpsx->dir = (float)NMEA_Str2num(p1+posx,&dx);
	   gpsx->dir = gpsx->dir/NMEA_Pow(10,dx);
       //printf("gpsx.dir = %f,dx = %d\n",gpsx->dir,dx);
    } else {
       gpsx->dir = (double)0xff;
	}
    posx=NMEA_Comma_Pos(p1,7);                              //得到地面速率
    if(posx!=0XFF)
    {
        gpsx->speed=NMEA_Str2num(p1+posx,&dx);
        if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);             //确保扩大1000倍
    }
}

//分析GPGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;           
    u8 posx;    
    p1=(u8*)strstr((const char *)buf,"$GPGGA");
    posx=NMEA_Comma_Pos(p1,6);                              //得到GPS状态
    if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);   
    posx=NMEA_Comma_Pos(p1,7);                              //得到用于定位的卫星数
    if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
    posx=NMEA_Comma_Pos(p1,8);                              //HDOP水平精度因子（0.5 - 99.9）
    if(posx!=0XFF) { 
        gpsx->hdop=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx); //去掉小数,保留整数
    } else {
		gpsx->hdop = 0xff;
	}
    posx=NMEA_Comma_Pos(p1,9);                              //得到海拔高度
    if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}
/*
**setup为0时，从头开始拷贝，否则从末尾开始拷贝
**/
void strcpy(char * dest,char * src,u8 setup) {// 实现src到dest的复制
     char i = 0,length;
     char* p;
     length = strlen(src);
     p = src+setup;

     if(setup == 0) {
         while(i < length) {  //把src字符串的内容复制到dest下
              dest[i] = src[i];
              i++;
         }
         dest[length] = '\0';
     } else {
         i =  length-1;
         dest[i+setup] = '\0';
         while(i >= 0) {       //把src字符串的内容复制到dest下,从末尾开始拷贝
              p[i] = src[i];
              //printf("src[%d] = %c\n",i,src[i]);
              i--;
         }
     }
}
/*before  xx,aaaaaa,xx  先计算出src.a的length
            ^      ^
            |      |
            p1     p2
**after   xx,bbb,xx     计算出dest.b的length
            ^   ^
            |   |
            p1  p2
*/
int NMEA_StrReplace(u8 *buf,u8 cx,char* replaceBuf) {
    u8 posx,i = 0;
    u8 *p1,*p2;
    char aLength = 0,bLength = 0;

    posx = NMEA_Comma_Pos(buf,cx);
    aLength = NMEA_Comma_Pos(buf,cx+1) - posx -1;
	if(replaceBuf == NULL) {
        bLength = 0;
	} else {
        bLength = strlen(replaceBuf);
	}
    //printf("posx = %d,aLength = %d,bLength = %d \n",posx,aLength,bLength);
    p1 = buf + posx;
    p2 = buf + posx + aLength;

    if(aLength == bLength) {
        
    } else if(aLength > bLength){
        strcpy(p1+bLength,p2,0);
    } else {
        strcpy(p1+bLength,p2,bLength - aLength);
    }
    //printf("buf = %s\n",buf);
    for(i = 0;i < bLength;i++) {
        p1[i] = replaceBuf[i];
    }

    return 0;
}
void checkCRC(u8 * buffer,u8* crc) {
    u8 i,result;
    for(result=buffer[1],i=2;buffer[i]!='*';i++) {
         result^=buffer[i];
    }
    sprintf(crc,"%x",result);
    //printf("checkCRC 0x%s\n",crc);
}

void NMEA_UpdateCRC(u8 *buf) {
    u8 i = 0,crc[2];
    while(1) {
        if(buf[i] == '*') {
            break;
        }
        i++;
    }
    checkCRC(buf,crc);
    buf[i+1] = crc[0];
    buf[i+2] = crc[1];
}


