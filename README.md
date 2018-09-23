# STM32_uCOS_GPS
MCU：stm32f103c8 
用了2个UART，波特率都是9600，其中uart1用于接pc，uart2接ublox的gps模块。
uar1口： A9->Tx A10->Rx
uar2口： A2->Tx A3->Rx
====================================================================
上电时配置ubx输出，stm32从uart2接到nmea协议，会经过简单处理，然后输出到uart1
