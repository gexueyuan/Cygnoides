/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_sys_param.h
 @brief  : head file of vanet's param 
 @author : gexueyuan
 @history:
           2015-2-4    gexueyuan    Created file
           ...
******************************************************************************/

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
#ifndef __CV_SYS_PARAM_H_
#define __CV_SYS_PARAM_H_

#include "cv_cms_def.h"
typedef struct _flash_param{
    char *key;
    char *cus_value;
    char *hw_value;
    char *mt_value;    
    char *ct_value;
}flash_param, *flash_param_t;


typedef struct {

	cfg_param_t cus_param;
	cfg_param_t hw_param;
	cfg_param_t mt_param;
	cfg_param_t ct_param;

}vanet_param;


#define PARAM_FLAG_ADDR     ((uint32_t)0x80E0000)

#define PARAM_MODE_ADDR     ((uint32_t)0x80E0010)

#define PARAM_ADDR    		((uint32_t)0x80E0020)

#define PARAM_ERASE_SECTOR   FLASH_Sector_11

#define name_to_str(name)   (#name)


typedef struct _flash_env{
    char *key;
    char *value;
}flash_env, *flash_env_t;

static const flash_env default_param_data[] = {

    {"(0)ID","0000/0000/0000/0000"},
    {"(1)mode","0/1/2/3"},
        
    /*******************VAM*******************/    
    {"(1)vam.bsm_hops","1/1/1/1"},
    
    {"(2)vam.bsm_boardcast_mode","1/1/1/1"},/* 0 - disable, 1 - auto, 2 - fixed period */
    
    {"(3)vam.bsm_boardcast_saftyfactor","5/5/5/5"}, /* 1~10 */
    
    {"(4)vam.bsm_pause_mode","1/1/1/1"},/* 0 - disable, 1 - enable */
    
    {"(5)vam.bsm_pause_hold_time","5/5/5/5"}, /* unit:s */
    
    {"(6)vam.bsm_boardcast_period","100/100/100/100"},/* 100~3000, unit:ms, min accuracy :10ms */
    
    {"(7)vam.evam_hops","1/1/1/1"},
    
    {"(8)vam.evam_broadcast_type","2/2/2/2"},
    
    {"(9)vam.evam_broadcast_peroid","50/50/50/50"},

    /***********************VSA**************/
    
    {"(10)vsa.danger_detect_speed_threshold","30/30/30/30"},/* unit: km/h */
    
    {"(11)vsa.danger_alert_period","50/50/50/50"},/* 50~1000, unit:ms, min accuracy :10ms */
    
    {"(12)vsa.crd_saftyfactor","4/4/4/4"}, /* 1~10 */

    {"(13)vsa.crd_oppsite_speed","0/0/0/0"},/* unit: km/h */
    
    {"(14)vsa.crd_oppsite_rear_speed","10/10/10/10"},/* unit: km/h */
    
    {"(15)vsa.crd_rear_distance","20/20/20/20"},/*m:20m*/
    
    {"(16)vsa.ebd_mode","1/1/1/1"}, /* 0 - disable, 1 - enable */

    {"(17)vsa.ebd_acceleration_threshold","3/3/3/3"},/* unit:m/s2 */
    
    {"(18)vsa.ebd_alert_hold_time","5/5/5/5"},/* unit:s */

    /***********************gsnr******************/
    
    {"(19)gsnr.gsnr_cal_step","0/0/0/0"},
    
    {"(20)gsnr.gsnr_cal_thr","4/4/4/4"},

    {"(21)gsnr.gsnr_ebd_thr","-55/-55/-55/-55"},
    
    {"(22)gsnr.gsnr_ebd_cnt","2/2/2/2"},

    
    {"(23)gsnr.AcceV_x","0/0/0/0"},
    
    {"(24)gsnr.AcceV_y","0/0/0/0"},

    {"(25)gsnr.AcceV_z","0/0/0/0"},
    
    {"(26)gsnr.AcceAhead_x","0/0/0/0"},

    {"(27)gsnr.AcceAhead_y","0/0/0/0"},

    {"(28)gsnr.AcceAhead_z","0/0/0/0"},

    /**************wnet***************************/
    
    {"(27)wnet.channel","13/13/13/13"},

    {"(28)wnet.txrate","6/6/6/6"},

};
#endif
