/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_cms_def.h
 @brief  : This file include the cms global definition 
 @author : wangyifeng
 @history:
           2014-6-16    wangyifeng    Created file
           ...
******************************************************************************/
#ifndef __CV_CMS_DEF_H__
#define __CV_CMS_DEF_H__

#include "cv_vam.h"
#include "cv_vsa.h"
#include "gsensor.h"

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/

/**
    prority of all the tasks in system 
*/
#define RT_INIT_THREAD_PRIORITY		10   /* highest, to ensure all initial 
                                            will be complete before running*/
#define RT_USBEMU_THREAD_PRIORITY	13
#define RT_USBH_THREAD_PRIORITY		12 //26
#define RT_SYS_THREAD_PRIORITY		15
#define RT_VAM_THREAD_PRIORITY		23
#define RT_VSA_THREAD_PRIORITY		22

#define RT_GPS_THREAD_PRIORITY		21
#define RT_MEMS_THREAD_PRIORITY		22
#define RT_HI_THREAD_PRIORITY		25

#define RT_THREAD_STACK_SIZE   (1024*2)

#define RT_INIT_THREAD_STACK_SIZE   (1024*2)
#define RT_USBH_THREAD_STACK_SIZE   (1024*2)
#define RT_SYS_THREAD_STACK_SIZE   (1024*2)
#define RT_GPS_THREAD_STACK_SIZE   (1024*2)
#define RT_MEMS_THREAD_STACK_SIZE   (1024*2)
#define RT_VAM_THREAD_STACK_SIZE   (1024*2)
#define RT_VSA_THREAD_STACK_SIZE   (1024*2)
#define RT_HI_THREAD_STACK_SIZE   (1024*2)


/**
    size of all the queue in system 
*/
#define SYS_QUEUE_SIZE 16
#define VAM_QUEUE_SIZE 16
#define VSA_QUEUE_SIZE 16


enum SYSTEM_MSG_TYPE{
    SYS_MSG_BASE = 0x0000,
    SYS_MSG_INITED,
    SYS_MSG_KEY_PRESSED,
    SYS_MSG_KEY_RELEASED,
    SYS_MSG_START_ALERT,
    SYS_MSG_STOP_ALERT,
    SYS_MSG_ALARM_ACTIVE,
    SYS_MSG_ALARM_CANCEL,
    SYS_MSG_GPS_UPDATE,
    SYS_MSG_BSM_UPDATE,
    SYS_MSG_HI_IN_UPDATE,
    SYS_MSG_HI_OUT_UPDATE,
    SYS_MSG_XXX,

    VAM_MSG_BASE = 0x0200,
    VAM_MSG_START,
    VAM_MSG_STOP,
    VAM_MSG_RCPTX,
    VAM_MSG_RCPRX,
    VAM_MSG_NEIGH_TIMEOUT,
    VAM_MSG_GPSDATA,


    VSA_MSG_BASE = 0x0300,
    VSA_MSG_LOCAL_UPDATE,
    VSA_MSG_PEER_UPDATE,
    VSA_MSG_GPS_UPDATE,
    VSA_MSG_ALARM_UPDATE,
    VSA_MSG_KEY_UPDATE,
    VSA_GSNR_EBD_DETECT,
};

enum HI_OUT_TYPE{
    HI_OUT_NONE = 0,
    HI_OUT_SYS_INIT,
    HI_OUT_BSM_UPDATE,
    HI_OUT_BSM_NONE,
    HI_OUT_GPS_CAPTURED,
    HI_OUT_GPS_LOST,
    HI_OUT_CRD_ALERT,
    HI_OUT_CRD_CANCEL,
    HI_OUT_CRD_REAR_ALERT,
    HI_OUT_CRD_REAR_CANCEL,
    HI_OUT_VBD_ALERT,
    HI_OUT_VBD_CANCEL,
    HI_OUT_VBD_STATUS,
    HI_OUT_VBD_STOP,
    HI_OUT_EBD_ALERT,
    HI_OUT_EBD_CANCEL,
    HI_OUT_EBD_STATUS,
    HI_OUT_EBD_STOP,
    HI_OUT_CANCEL_ALERT,
};


/**
    misc definitions 
*/
#define MS_TO_TICK(n)     ((n)*RT_TICK_PER_SECOND/1000)
#define SECOND_TO_TICK(n) ((n)*RT_TICK_PER_SECOND)


/*****************************************************************************
 * declaration of structs                                                    *
*****************************************************************************/

/**
    structure of system global message 
*/
typedef struct _sys_msg{
    uint16_t id;
    uint16_t len;
    uint32_t argc; 
    void    *argv;
}sys_msg_t;

/**
    structure of system configure parameters 
*/
typedef struct _cfg_param{

	/*********************ID******************/	
    uint8_t pid[RCP_TEMP_ID_LEN];  // ID 

    /******************** VAM *********************/
    vam_config_t vam;

    /******************** VSA *********************/
    vsa_config_t vsa;

	/**************************************************/
	gsnr_param_t gsnr;
    /******************** DBG *********************/
    uint8_t print_xxx;  /* 0 - disable, 1 - enable */

                 
}cfg_param_t;


typedef struct _led_color{

uint8_t r;
uint8_t g;
uint8_t b;
}led_color_t;
/** 
    structure of system manager module's environment variable 
*/
typedef struct _sys_envar{
    /* working_param */
    vsa_config_t working_param;

    uint32_t status;

    uint32_t hi_timer_cnt;

    uint32_t led_priority;
    led_color_t led_color;
    uint16_t led_blink_duration;
    uint16_t led_blink_period;
    uint16_t led_blink_cnt;
	uint8_t voc_flag;

    /* os related */
    osal_task_t *task_sys_mng;
    osal_queue_t *queue_sys_mng;

    rt_thread_t task_sys_hi;
    rt_mq_t queue_sys_hi;

    rt_timer_t timer_hi;
    rt_timer_t timer_voc;
	//rt_timer_t timer_gps;
    
}sys_envar_t;


/**
    structure of system global environment variable 
*/
typedef struct _cms_global{
    vam_envar_t vam;
    vsa_envar_t vsa;

    sys_envar_t sys;
}cms_global_t;



/*****************************************************************************
 * declare of global functions and variables                                 *
*****************************************************************************/
extern cms_global_t cms_envar, *p_cms_envar;
extern cfg_param_t cms_param, *p_cms_param;

int sys_add_event_queue(sys_envar_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv);

extern double vsm_get_relative_pos_immediate(vam_stastatus_t *p_src, uint8_t *payload);
#endif /* __CV_CMS_DEF_H__ */

