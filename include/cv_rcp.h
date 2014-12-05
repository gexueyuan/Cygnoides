/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_rcp.h
 @brief  : this file include the definitions of  the remote communicate 
           protocol of vehicle
 @author : wangyifeng
 @history:
           2014-6-22    wangyifeng    Created file
           ...
******************************************************************************/
#ifndef __CV_RCP_H__
#define __CV_RCP_H__


#define __COMPILE_PACK__  __packed
#define __COMPILE_INLINE__ 





/*****************************************************************************
 * definition of micro                                                       *
*****************************************************************************/




/*****************************************************************************
 * definition of structs                                                     *
*****************************************************************************/
typedef __COMPILE_PACK__ struct _rcp_position{
    int32_t lat;
    int32_t lon;
    int16_t elev;
    int32_t accu;
}rcp_position_t;

typedef __COMPILE_PACK__ struct _rcp_acceleration{
    uint16_t lon;
    uint16_t lat;
    uint16_t vert;
    uint8_t  yaw;
}rcp_acceleration_t;


typedef __COMPILE_PACK__ struct _rcp_motion{
    uint16_t speed;
    uint16_t heading; 
    rcp_acceleration_t  acce;                 
}rcp_motion_t;

typedef __COMPILE_PACK__ struct _rcp_msg_head{
    uint8_t   msg_id;
    uint8_t   msg_count;
    uint8_t   temporary_id[4];
    uint16_t  dsecond;
}rcp_msg_head_t;


typedef __COMPILE_PACK__ struct _rcp_msg_basic_safty{
    rcp_msg_head_t header;
    rcp_position_t position;
    rcp_motion_t motion;
}rcp_msg_basic_safty_t;

typedef __COMPILE_PACK__ struct _rcp_msg_emergency_vehicle_alert{
    rcp_msg_head_t header;
    rcp_position_t position;
    rcp_motion_t motion;
    uint16_t alert_mask;
}rcp_msg_emergency_vehicle_alert_t;






#endif /* __CV_RCP_H__ */

