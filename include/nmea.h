#ifndef _NMEA_H_
#define _NMEA_H_
#include <stdint.h>
#include <rtthread.h>	

#define NMEA_DEBUG(...)
//#define NMEA_DEBUG NOP_FuncTion
#ifndef __TRUE
 #define __TRUE         1
#endif
#ifndef __FALSE
 #define __FALSE        0
#endif

#define GPS_MAX_SIZE 3
#define GPS_WRITE_IN_WAKEUP 30

//GPS
#define CYC_GPS_MIN (20)
#define CYC_GPS_DEFALUT (30)

typedef struct _t_time
{
	uint16_t year;
    uint8_t mon;
    uint8_t day;
	uint8_t hour ;
	uint8_t min;
	uint8_t sec;

	uint32_t diffsec;
} t_time;

typedef struct{
    uint8_t isTrue;
    t_time updateTime;
    double speed;
    double latitude;
    double longitude;
    double  heading;
    t_time tt;
} t_nmea_rmc;

typedef struct{
    t_nmea_rmc gpsLastBuff;

    uint8_t crtIndex;
    uint8_t diffsec;
    t_nmea_rmc gpsBuff[GPS_MAX_SIZE];
} t_nmea_cfg;

typedef enum{
    GPS_PACK_UNKNOWN = 0,
    GPS_PACK_GPGGA,
    GPS_PACK_GPGSA,
    GPS_PACK_GPGSV,
    GPS_PACK_GPRMC,
    GPS_PACK_GPVTG
} e_nmea_type;
typedef enum {
	None		= 0,	//无
    Rush_Add	= 1,	//急加速 
    Rush_Stop	= 2,	//急减速
    Rush_Left	= 3,	//急左
	Rush_Right	= 4		//急右
} driving_rush_type;

typedef struct {
	driving_rush_type type;	//类型
	float value;				//值
	t_time time;				//时间点
	float latitude;
	float longitude;
	float speed;
} driving_rush_value_st;

typedef struct _driving_action_st
{
	float speed ;
	float vehicle_accel_value ;
	float diff_angle ;
	uint8_t is_locate ;
    uint8_t carRun;
}driving_action_st ;

extern driving_action_st G_Action;
extern uint8_t IsLocate;
void nmea_init(void);
int32_t nmea_get(t_nmea_rmc *recvBuff, int8_t flag);

void nmea_parse(uint8_t *buff, uint32_t len);

#endif
