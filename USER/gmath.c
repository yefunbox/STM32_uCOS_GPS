#include <math.h>
#include <float.h>
#include "gmath.h"
#include "nmeaParse.h"

//根据角度计算弧度
double rad(double d) {
    return d * PI/180.0;
}
//根据弧度计算角度
double Angle(double r) {
    return r * 180/PI;
}

//根据两点经纬度计算两点距离
double GetDistance(nmeaPOS from_pos,nmeaPOS to_pos) {
    const double earth_radius = 6378137;//地球半径(单位：m)
    
    double radLat1 = rad(from_pos.lat);//根据角度计算弧度
    double radLat2 = rad(to_pos.lat);
    double radLng1 = rad(from_pos.lon);
    double radLng2 = rad(to_pos.lon);
    
    double a = radLat1 - radLat2;
    double b = radLng1 - radLng2;
    
    double s = 2 * asin(sqrt(pow(sin(a/2),2) + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));//google maps里面实现的算法
    s = s * earth_radius;  
	if(s < 1000) {
   	    printf("Distance=%0.3fm  ", s);
	} else {
   	    printf("Distance=%0.3fkm  ", s/1000);
	}
    return s;  
}

//计算方位角
double GetBearing(double lat1, double lng1, double lat2, double lng2) {
    double d = 0;
    
    double radLat1 = rad(lat1);//根据角度计算弧度
    double radLat2 = rad(lat2);
    double radLng1 = rad(lng1);
    double radLng2 = rad(lng2);
    
    d = sin(radLat1)*sin(radLat2)+cos(radLat1)*cos(radLat2)*cos(radLng2-radLng1);
    d = sqrt(1-d*d);
    d = cos(radLat2)*sin(radLng2-radLng1)/d;
    d = Angle(asin(d));
    
    return d;//返回方位角
}
/**
 * Convert NDEG (NMEA degree) to fractional degree
 * 2233.756060 -> 22.562601 = 22��33'45.3636"
 */
void nmea_ndeg2degree(double val,double *retVal)
{
    double deg = ((int)(val / 100));
    val = deg + (val - deg * 100) / 60;
    *retVal = val;
}

double nmea_distance(
        const nmeaPOS *from_pos,    /**< From position in radians */
        const nmeaPOS *to_pos       /**< To position in radians */
        )
{
    double dist = ((double)NMEA_EARTHRADIUS_M) * acos(
        sin(to_pos->lat) * sin(from_pos->lat) +
        cos(to_pos->lat) * cos(from_pos->lat) * cos(to_pos->lon - from_pos->lon)
        );
    return dist;
}

