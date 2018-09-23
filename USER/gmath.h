#ifndef __NMEA_GMATH_H__
#define __NMEA_GMATH_H__
#include "nmeaParse.h"

#define PI                           3.1415926
#define NMEA_PI                     (3.141592653589793)             /**< PI value */
#define NMEA_PI180                  (NMEA_PI / 180)                 /**< PI division by 180 */
#define NMEA_EARTHRADIUS_KM         (6378)                          /**< Earth's mean radius in km */
#define NMEA_EARTHRADIUS_M          (NMEA_EARTHRADIUS_KM * 1000)    /**< Earth's mean radius in m */
#define NMEA_EARTH_SEMIMAJORAXIS_M  (6378137.0)                     /**< Earth's semi-major axis in m according WGS84 */
#define NMEA_EARTH_SEMIMAJORAXIS_KM (NMEA_EARTHMAJORAXIS_KM / 1000) /**< Earth's semi-major axis in km according WGS 84 */
#define NMEA_EARTH_FLATTENING       (1 / 298.257223563)             /**< Earth's flattening according WGS 84 */
#define NMEA_DOP_FACTOR             (5)                             /**< Factor for translating DOP to meters */

/**
 * Position data in fractional degrees or radians
 */
typedef struct _nmeaPOS
{
    double lat;         /**< Latitude */
    double lon;         /**< Longitude */
} nmeaPOS;

#endif /* __NMEA_GMATH_H__ */
