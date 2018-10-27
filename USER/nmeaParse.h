#ifndef __NMEAR_PARSE_H
#define __NMEAR_PARSE_H

typedef unsigned char u8;
typedef unsigned short u16;

typedef struct
{
    u8 UTCDateTime[6];  // YMDHMS
    char Status;        // A/V [ASCII]

    u8 LatitudeD;       // dd
    u8 LatitudeM;       // mm
    u16 LatitudeS;      // mmmm
    char NS;            // N/S [ASCII] 

    u8 LongitudeD;      // ddd
    u8 LongitudeM;      // mm
    u16 LongitudeS;     // mmmm
    char EW;            // E/W [ASCII]

    u16 Speed;          // é€Ÿç‡000.0~999.9èŠ‚  x10
    char sSpeed[5];     // DEBUG
    u16 Course;         // èˆªå‘000.0~359.9åº¦  x10
    char sCourse[5];    // DEBUG
}struct_GPSRMC;

typedef struct
{
//  u8 UTCTime[10];	    // hhmmss.mmm
//  u8 Latitude[9];	    // ddmm.mmmm
//  char NS;		    // N/S
//  u8 Longitude[10];	// dddmm.mmmm
//  char EW;		    // E/W
    char PositionFix;	// 0,1,2,6 [ASCII]
    u8 SatUsed;	        // 00~12
//  u8 HDOP[4];		    // 0.5~99.9
    char Altitude[8];	// -9999.9~99999.9 [ASCII]
}struct_GPSGGA;

typedef struct
{
    char Mode;		    // A/M [ASCII]
    char Mode2;	        // 0,1,2,3 [ASCII]
    u8 SatUsedList[12];	// ä½¿ç”¨ä¸­çš„å«æ˜Ÿå·
    char PDOP[5];       // ä½ç½®ç²¾åº¦ 0.5-99.9
    char HDOP[5];       // æ°´å¹³ç²¾åº¦ 0.5-99.9
    char VDOP[5];       // å‚ç›´ç²¾åº¦ 0.5-99.9
}struct_GPSGSA;

typedef struct
{
    u8 SatID;           // å«æ˜Ÿå·
    u8 Elevation;       // å«æ˜Ÿä»°è§’   00-90 åº¦
    u16 Azimuth;        // å«æ˜Ÿæ–¹ä½è§’ 000-359 åº¦
    u8 SNR;             // è®¯å·å™ªå£°æ¯” 00-99 dbHz
}struct_SatInfo;

typedef struct
{
    u8 SatInView;
    struct_SatInfo SatInfo[12];
}struct_GPSGSV;

/**
 * VTG packet information structure (Track made good and ground speed)
 */
typedef struct _nmeaGPVTG
{
    double  dir;        /**< True track made good (degrees) */
    char    dir_t;      /**< Fixed text 'T' indicates that track made good is relative to true north */
    double  dec;        /**< Magnetic track made good */
    char    dec_m;      /**< Fixed text 'M' */
    double  spn;        /**< Ground speed, knots */
    char    spn_n;      /**< Fixed text 'N' indicates that speed over ground is in knots */
    double  spk;        /**< Ground speed, kilometers per hour */
    char    spk_k;      /**< Fixed text 'K' indicates that speed over ground is in kilometers/hour */

} nmeaGPVTG;

typedef  void (*GPRMC_CALLBACK)(struct_GPSRMC);
typedef  void (*GPGGA_CALLBACK)(struct_GPSGGA);
typedef  void (*GPGSA_CALLBACK)(struct_GPSGSA);
typedef  void (*GPGSV_CALLBACK)(struct_GPSGSV);
typedef  void (*GPVTG_CALLBACK)(nmeaGPVTG);

typedef struct
{
    GPRMC_CALLBACK  gprmcCallback;
    GPGGA_CALLBACK  gpggaCallback;
    GPGSA_CALLBACK  gpgsaCallback;
	GPGSV_CALLBACK  gpgsvCallback;
	GPVTG_CALLBACK  gpvtgCallback;
}struct_parser_callback;

#define NMEA_NULL   0x00        //GPSè¯­å¥ç±»å‹
#define NMEA_GPGGA  0x01
#define NMEA_GPGSA  0x02
#define NMEA_GPGSV  0x04
#define NMEA_GPRMC  0x08
#define NMEA_GPVTG  0x10

#define GPS_PARSE_FAILURE           0x00
#define GPS_PARSE_START             0x01
#define GPS_PARSE_CHECKSUM_ERROR    0x02
#define GPS_PARSE_OK                0x03

#define NMEA_MAXSAT         (37)
#define NMEA_SATINPACK      (4)
#define NMEA_NSATPACKS      (NMEA_MAXSAT / NMEA_SATINPACK)

u8 GPS_Parser(char SBuf);
void GPS_GetLogData(u8 * LogBuf);
void GPS_GetTime(u8* ucDataTime);
typedef struct
{
    u8 mq_type;
	u8 size;
    u8* pData;
}struct_gpsDataMQ;
typedef struct {
    u8* nmeaFramePtr;
	u8  nmeaFrameSize;
    u8* nmeaPtrRMC;
    u8* nmeaPtrVTG;
    u8* nmeaPtrGGA;	
    u8* nmeaPtrGSA;
    u8* nmeaPtrGSV1;
    u8* nmeaPtrGSV2;
    u8* nmeaPtrGSV3;
    u8* nmeaPtrGSV4;
    u8* nmeaPtrGLL;
    u8* ubxFramePtr;
}GpsFrame;

typedef struct _nmeaTIME
{
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [1,12] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     hsec;       /**< Hundredth part of second - [0,99] */

} nmeaTIME;

typedef struct _nmeaGPGGA
{
    nmeaTIME utc;       /**< UTC of position (just time) */
    double  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
    double  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    int     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
    int     satinuse;   /**< Number of satellites in use (not those in view) */
    double  HDOP;       /**< Horizontal dilution of precision */
    double  elv;        /**< Antenna altitude above/below mean sea level (geoid) */
    char    elv_units;  /**< [M]eters (Antenna height unit) */
    double  diff;       /**< Geoidal separation (Diff. between WGS-84 earth ellipsoid and mean sea level. '-' = geoid is below WGS-84 ellipsoid) */
    char    diff_units; /**< [M]eters (Units of geoidal separation) */
    double  dgps_age;   /**< Time in seconds since last DGPS update */
    int     dgps_sid;   /**< DGPS station ID number */

} nmeaGPGGA;

/**
 * Information about satellite
 * @see nmeaSATINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATELLITE
{
    int     id;         /**< Satellite PRN number */
    int     in_use;     /**< Used in position fix */
    int     elv;        /**< Elevation in degrees, 90 maximum */
    int     azimuth;    /**< Azimuth, degrees from true north, 000 to 359 */
    int     sig;        /**< Signal, 00-99 dB */

} nmeaSATELLITE;


/**
 * GSA packet information structure (Satellite status)
 */
typedef struct _nmeaGPGSA
{
    char    fix_mode;   /**< Mode (M = Manual, forced to operate in 2D or 3D; A = Automatic, 3D/2D) */
    int     fix_type;   /**< Type, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D) */
    int     sat_prn[NMEA_MAXSAT]; /**< PRNs of satellites used in position fix (null for unused fields) */
    double  PDOP;       /**< Dilution of precision */
    double  HDOP;       /**< Horizontal dilution of precision */
    double  VDOP;       /**< Vertical dilution of precision */

} nmeaGPGSA;

/**
 * GSV packet information structure (Satellites in view)
 */
typedef struct _nmeaGPGSV
{
    int     pack_count; /**< Total number of messages of this type in this cycle */
    int     pack_index; /**< Message number */
    int     sat_count;  /**< Total number of satellites in view */
    nmeaSATELLITE sat_data[NMEA_SATINPACK];

} nmeaGPGSV;


/**
 * RMC packet information structure (Recommended Minimum sentence C)
 */
typedef struct _nmeaGPRMC
{
    nmeaTIME utc;       /**< UTC of position */
    char    status;     /**< Status (A = active or V = void) */
    double  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
    double  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    double  speed;      /**< Speed over the ground in knots */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */
    char    declin_ew;  /**< [E]ast or [W]est */
    char    mode;       /**< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator) */

} nmeaGPRMC;

/**
 * NMEA packets type which parsed and generated by library
 */
enum nmeaPACKTYPE
{
    GPNON   = 0x0000,   /**< Unknown packet type. */
    GPGGA   = 0x0001,   /**< GGA - Essential fix data which provide 3D location and accuracy data. */
    GPGSA   = 0x0002,   /**< GSA - GPS receiver operating mode, SVs used for navigation, and DOP values. */
    GPGSV   = 0x0004,   /**< GSV - Number of SVs in view, PRN numbers, elevation, azimuth & SNR values. */
    GPRMC   = 0x0008,   /**< RMC - Recommended Minimum Specific GPS/TRANSIT Data. */
    GPVTG   = 0x0010,   /**< VTG - Actual track made good and speed over ground. */
	GPGLL   = 0x2000,   //Geographic Position£¨GLL£©¶¨Î»µØÀíĞÅÏ¢

    GPTXT   = 0x0020,   /**< TXT - ²úÆ·ĞÅÏ¢¡¢ÌìÏß×´Ì¬¡¢ÈòÃëµÈ */

    BDGSV   = 0x0040,   /**< GSV - ¿É¼ûÎÀĞÇ */
    BDGSA   = 0x0080,   /**< GGA - ¾«¶ÈÒò×ÓºÍÓĞĞ§ÎÀĞÇ */

    GNZDA   = 0x0100,   /**< ZDA - Ê±¼äºÍÈÕÆÚ */
    GNGLL   = 0x0200,   /**< GLL - µØÀíÎ»ÖÃ ¾­Î³¶È */
    GNVTG   = 0x0400,    /**< VTG - ¶ÔµØËÙ¶ÈºÍº½Ïò */
    GNRMC   = 0x0800,   /**< RMC - ÍÆ¼öµÄ×îÉÙ×¨ÓÃº½ÏòÊı¾İ */
    GNGGA   = 0x1000,   /**< GGA - ½ÓÊÕ»ú¶¨Î»Êı¾İ£¬ÏêÏ¸*/
};

/**
 * Information about all satellites in view
 * @see nmeaINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATINFO
{
    int     inuse;      /**< Number of satellites in use (not those in view) */
    int     inview;     /**< Total number of satellites in view */
    nmeaSATELLITE sat[NMEA_MAXSAT]; /**< Satellites information */

} nmeaSATINFO;

typedef struct _nmeaINFO
{
    int     smask;      /**< Mask specifying types of packages from which data have been obtained */

    nmeaTIME utc;       /**< UTC of position */

    int     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
    int     fix;        /**< Operating mode, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D) */

    double  PDOP;       /**< Position Dilution Of Precision */
    double  HDOP;       /**< Horizontal Dilution Of Precision */
    double  VDOP;       /**< Vertical Dilution Of Precision */
   
    double  lat;        /**< Latitude in NDEG - +/-[degree][min].[sec/60] */
    double  lon;        /**< Longitude in NDEG - +/-[degree][min].[sec/60] */
    double  elv;        /**< Antenna altitude above/below mean sea level (geoid) in meters */
    double  sog;        /**< ÊıÖµ ¶ÔµØËÙ¶È£¬µ¥Î»Îª½Ú */
    double  speed;      /**< Speed over the ground in kilometers/hour */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */
    char    mode;       /**< ×Ö·û ¶¨Î»Ä£Ê½±êÖ¾ (A = ×ÔÖ÷Ä£Ê½, D = ²î·ÖÄ£Ê½, E = ¹ÀËãÄ£Ê½, N = Êı¾İÎŞĞ§) */
    nmeaSATINFO satinfo; /**< Satellites information */
    nmeaSATINFO BDsatinfo; /**±±¶·ÎÀĞÇĞÅÏ¢*/
		
	int txt_level;
	char *txt;
} nmeaINFO;


#endif
