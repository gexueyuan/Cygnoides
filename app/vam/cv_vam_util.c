/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vam_util.c
 @brief  : this file include some common function
 @author : wangyifeng
 @history:
           2014-6-24    wangyifeng    Created file
           ...
******************************************************************************/
#include "cv_osal.h"

#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "vam"
#include "cv_osal_dbg.h"
OSAL_DEBUG_ENTRY_DEFINE(vam)

#include <math.h>

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"
#include "nmea.h"



/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/

#define EARTH_RADIUS  6371.004
#define PI 3.1415926
#define RAD(d) ((d)*PI/180.0)


#define _COMPILE_INLINE__


_COMPILE_INLINE__ uint16_t cv_ntohs(uint16_t s16);

__COMPILE_INLINE__ uint32_t cv_ntohl(uint32_t l32);

__COMPILE_INLINE__ float cv_ntohf(float f32);


__COMPILE_INLINE__ int32_t encode_longtitude(float x);


__COMPILE_INLINE__ float decode_longtitude(uint32_t x);

#define encode_latitude(x) encode_longtitude(x) 
#define decode_latitude(x) decode_longtitude(x) 

#define encode_accuracy(x) encode_longtitude(x) 
#define decode_accuracy(x) decode_longtitude(x) 

__COMPILE_INLINE__ uint16_t encode_elevation(float x);

__COMPILE_INLINE__ float decode_elevation(uint16_t x);

__COMPILE_INLINE__ uint16_t encode_speed(float x);


__COMPILE_INLINE__ float decode_speed(uint16_t x);

__COMPILE_INLINE__ uint16_t encode_heading(float x);

__COMPILE_INLINE__ float decode_heading(uint16_t x);

__COMPILE_INLINE__ uint16_t encode_acce_lon(float x);

__COMPILE_INLINE__ float decode_acce_lon(uint16_t x);
__COMPILE_INLINE__ uint16_t encode_acce_lat(float x);

__COMPILE_INLINE__ float decode_acce_lat(uint16_t x);

__COMPILE_INLINE__ uint16_t encode_acce_vert(float x);

__COMPILE_INLINE__ float decode_acce_vert(uint16_t x);

__COMPILE_INLINE__ uint8_t encode_acce_yaw(float x);

__COMPILE_INLINE__ float decode_acce_yaw(uint8_t x);







/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

#if 0
static double getDistanceVer1(double lat1, double lng1, double lat2, double lng2)

{

   double radLat1 = rad(lat1);

   double radLat2 = rad(lat2);

   double radLng1 = rad(lng1);

   double radLng2 = rad(lng2);

   double s = acos(sin(radLat1)*sin(radLat2)+cos(radLat1)*cos(radLat2)*cos(radLng1-radLng2));

   s = s * EARTH_RADIUS;

   return s;

}
#endif

double getDistanceVer2(double lat1, double lng1, double lat2, double lng2)

{

   double radLat1 = RAD(lat1);

   double radLat2 = RAD(lat2);

   double a = radLat1 - radLat2;

   double b = RAD(lng1) - RAD(lng2);

   double s = 2 * asin(sqrt(pow(sin(a/2),2) + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));

   s = s * EARTH_RADIUS;

   return s;

}

double vsm_get_distance(vam_position_t *p_src, vam_position_t *p_dest)
{
    double d = 1000.0;

    d *= getDistanceVer2(p_src->lat, p_src->lon, p_dest->lat, p_dest->lon);

    #if 0
    {
        char str[64];
        sprintf(str,"%f", d);
        rt_kprintf("distance:%s\n", str);
    }
    #endif

    return d;
}


const char *_directfromangle(int angle)
{
    static const char *dir[] = {
        "北",
        "东北",
        "东",
        "东南",
        "南",
        "西南",
        "西",
        "西北",
    };

    int i;

    if (angle <= 10){
        i = 0;
    }
    else if(angle < 80){
        i = 1;
    }
    else if(angle <= 100){
        i = 2;
    }
    else if(angle < 170){
        i = 3;
    }
    else if(angle <= 190){
        i = 4;
    }
    else if(angle < 260){
        i = 5;
    }
    else if(angle <= 280){
        i = 6;
    }
    else if(angle < 350){
        i = 7;
    }
    else{
        i = 0;
    }

    return dir[i];
}

double vsm_get_relative_pos_immediate(vam_stastatus_t *p_src, uint8_t *payload)
{
    double lat1, lng1, lat2, lng2, lat3, lng3;
    double distance_1_2, distance_2_3;
    double angle, delta;

		rcp_msg_basic_safty_t *p_bsm;
    vam_stastatus_t p_dest;
	
		p_bsm = (rcp_msg_basic_safty_t *)payload;

		p_dest.timestamp = p_bsm->header.dsecond;

		p_dest.pos.lon = decode_longtitude(p_bsm->position.lon);
		p_dest.pos.lat = decode_latitude(p_bsm->position.lat);
		p_dest.pos.elev = decode_elevation(p_bsm->position.elev);
		p_dest.pos.accu = decode_accuracy(p_bsm->position.accu);

		p_dest.dir = decode_heading(p_bsm->motion.heading);
		p_dest.speed = decode_speed(p_bsm->motion.speed);
		p_dest.acce.lon = decode_acce_lon(p_bsm->motion.acce.lon);
		p_dest.acce.lat = decode_acce_lat(p_bsm->motion.acce.lat);
		p_dest.acce.vert = decode_acce_vert(p_bsm->motion.acce.vert);
		p_dest.acce.yaw = decode_acce_yaw(p_bsm->motion.acce.yaw);	
	
	
    /* reference point */
    lat1 = p_src->pos.lat;
    lng1 = p_src->pos.lon;

    /* destination point */
    lat2 = p_dest.pos.lat;
    lng2 = p_dest.pos.lon;

    /* temp point */
    lat3 = lat1;
    lng3 = lng2;

    distance_1_2 = getDistanceVer2(lat1, lng1, lat2, lng2);
    distance_2_3 = getDistanceVer2(lat2, lng2, lat3, lng3);
    angle = acos(distance_2_3/distance_1_2)*180/PI;

    /* calculate the relative angle against north, clockwise  */
    if (lat2 >= lat1){
    /* north */
        if (lng2 >= lng1){
        /* easts */
            //equal
        }
        else{
            angle = 360-angle;
        }
    }
    else{
    /* south */
        if (lng2 >= lng1){
        /* easts */
            angle = 180-angle;
        }
        else{
            angle = 180+angle;
        }
    }

    /* calculate the angle detra between local front and remote position  */
    if (angle > p_src->dir){
        delta = angle - p_src->dir;
    }
    else{
        delta = p_src->dir - angle;
    }

    if (delta > 180){
        delta = 360 - delta;
    }
		
	//zigbeeDelta = delta;
    //zigbeeDistance = distance_1_2 *= 1000; /* convert from Km to m */

    return (delta <= 45)? distance_1_2:(-distance_1_2);
}

uint8_t print_flag = 0xff;
double vsm_get_relative_pos(vam_stastatus_t *p_src, vam_stastatus_t *p_dest,uint8_t vsa_print_en)
{
    double lat1, lng1, lat2, lng2, lat3, lng3;
    double distance_1_2, distance_2_3;
    double angle, delta;

    /* reference point */
    lat1 = p_src->pos.lat;
    lng1 = p_src->pos.lon;

    /* destination point */
    lat2 = p_dest->pos.lat;
    lng2 = p_dest->pos.lon;

    /* temp point */
    lat3 = lat1;
    lng3 = lng2;

    distance_1_2 = getDistanceVer2(lat1, lng1, lat2, lng2);
    distance_2_3 = getDistanceVer2(lat2, lng2, lat3, lng3);
    angle = acos(distance_2_3/distance_1_2)*180/PI;

    /* calculate the relative angle against north, clockwise  */
    if (lat2 >= lat1){
    /* north */
        if (lng2 >= lng1){
        /* easts */
            //equal
        }
        else{
            angle = 360-angle;
        }
    }
    else{
    /* south */
        if (lng2 >= lng1){
        /* easts */
            angle = 180-angle;
        }
        else{
            angle = 180+angle;
        }
    }

    /* calculate the angle detra between local front and remote position  */
    if (angle > p_src->dir){
        delta = angle - p_src->dir;
    }
    else{
        delta = p_src->dir - angle;
    }

    if (delta > 180){
        delta = 360 - delta;
    }

    distance_1_2 *= 1000; /* convert from Km to m */
/*	
	rt_kprintf("My head:%s(%d), Your pos:%s(%d); Our delta:%d, distance:%d\n", \
				   _directfromangle((int)p_src->dir), (int)p_src->dir,\
				   _directfromangle((int)angle), (int)angle,\
				  (int)delta, (int)distance_1_2);

if((print_flag)&&(vsa_print_en))	
    rt_kprintf("tick =  %lu, My head:%s(%d),ID:%d%d%d%d, Your pos:%s(%d),ID:%d%d%d%d, Our delta:%d, distance:%d\n",rt_tick_get(), \
               _directfromangle((int)p_src->dir), (int)p_src->dir,p_src->pid[0],p_src->pid[1],p_src->pid[2],p_src->pid[3],\
               _directfromangle((int)p_dest->dir), (int)p_dest->dir,p_dest->pid[0],p_dest->pid[1],p_dest->pid[2],p_dest->pid[3],\
              (int)delta, (int)distance_1_2);
*/
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_LOUD,"My head:%s(%d),ID:%d%d%d%d, Your pos:%s(%d),ID:%d%d%d%d, Our delta:%d, distance:%d\n",\
               _directfromangle((int)p_src->dir), (int)p_src->dir,p_src->pid[0],p_src->pid[1],p_src->pid[2],p_src->pid[3],\
               _directfromangle((int)p_dest->dir), (int)p_dest->dir,p_dest->pid[0],p_dest->pid[1],p_dest->pid[2],p_dest->pid[3],\
              (int)delta, (int)distance_1_2);
              
//#endif
    return (delta <= 45)? distance_1_2:(-distance_1_2);
}

void set_print(void)
{
	print_flag = ~print_flag;
}
FINSH_FUNCTION_EXPORT(set_print,close or open  information of printing);
double vsm_get_relative_dir(vam_stastatus_t *p_src, vam_stastatus_t *p_dest)
{
    double delta;

    /* calculate the angle detra between local front and remote position  */
    if (p_dest->dir > p_src->dir){
        delta = p_dest->dir - p_src->dir;
    }
    else{
        delta = p_src->dir - p_dest->dir;
    }

    if (delta > 180){
        delta = 360 - delta;
    }

    return delta;
}

int8_t vsm_get_rear_dir(vam_stastatus_t *p_dest)
{
	if((p_dest->dir>270)||(p_dest->dir<90))
		return -1;
	else return 1;
	
}

