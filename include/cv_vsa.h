/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vsa.h
 @brief  : this file include the definition of vsa module
 @author : wangyifeng
 @history:
           2014-6-20    wangyifeng    Created file
           ...
******************************************************************************/
#ifndef __CV_VSA_H__
#define __CV_VSA_H__

/*****************************************************************************
 * definition of micro                                                       *
*****************************************************************************/
enum VSA_APP_ID{
    VSA_ID_NONE = 0,
    VSA_ID_CRD,
    VSA_ID_CRD_REAR,
    VSA_ID_VBD,    
    VSA_ID_EBD,
    VSM_ID_END
};


typedef struct _adpcm{

    uint32_t addr;
    uint32_t size;
    uint8_t  channel;
    uint8_t  cmd;
    
}adpcm_t;

enum VOC_CMD{
    VOC_PLAY = 0,
    VOC_PAUSE,
    VOC_STOP
};

/* target classification locations */

enum VSA_TARGET_LOCATION{
    POSITION_ERROR = 0,
    AHEAD_LEFT,        
    AHEAD,
    AHEAD_RIGHT,   
    RIGHT,
    BEHIND_RIGHT,
    BEHIND,
    BEHIND_LEFT,   
    LEFT
};

typedef struct _vsa_info{

    uint8_t pid[RCP_TEMP_ID_LEN];
    
    uint32_t vsa_location;

    float local_speed;

    float remote_speed;

    float relative_speed;

    float lat_offset;

    float lon_offset;

    uint32_t linear_distance;

    uint32_t safe_distance;

    float dir;

    int8_t flag_dir;

    uint8_t jump_count;

    uint8_t vsa_id;

}vsa_info_t;



typedef struct _vsa_position_node{

    list_head_t list;

    vsa_info_t vsa_position;
}vsa_position_node_t;


/*****************************************************************************
 * definition of struct                                                      *
*****************************************************************************/

typedef struct _vsa_config{
    /*
        General
    */
    

    uint8_t danger_detect_speed_threshold;  /* unit: km/h */
    uint16_t danger_alert_period;  /* 50~1000, unit:ms, min accuracy :10ms */
    /*
        Close Range Danger function:
    */
    uint8_t crd_saftyfactor;  /* 1~10 */
    uint8_t crd_oppsite_speed;/*<=255:30km/h*/
    uint8_t crd_oppsite_rear_speed;/*<=255:30km/h*/
    uint8_t crd_rear_distance;/*<=255:10m*/
    /*
        Emergency Braking Danger function:
    */
    uint8_t ebd_mode;  /* 0 - disable, 1 - enable */
    uint8_t ebd_acceleration_threshold; /* unit:m/s2 */
    uint8_t ebd_alert_hold_time;  /* unit:s */
	
}vsa_config_t;


typedef struct _vsa_envar{
    /* working_param */
    vsa_config_t working_param;

    uint32_t gps_status;

    uint32_t alert_mask;
    uint32_t alert_pend;

    vam_stastatus_t local;
    vam_stastatus_t remote;

    adpcm_t adpcm_data;

	/*List head*/	
    list_head_t crd_list;
    list_head_t position_list;
    //list_head_t ebd_list;
    //list_head_t vbd_list;
    vsa_position_node_t position_node[VAM_NEIGHBOUR_MAXNUM];
    

    /* os related */
    osal_task_t  *task_vsa;
    osal_queue_t *queue_vsa;

    osal_queue_t *queue_voc;

    osal_timer_t *timer_ebd_send;

    osal_timer_t *timer_position_prepro;
                 
}vsa_envar_t;

typedef struct _vsa_crd_node{
    /* !!!DON'T modify it!!! */
    list_head_t list;

    uint8_t pid[RCP_TEMP_ID_LEN];  //temporary ID

    /* ccw_id = 1(VSA_ID_CRD) is cfcw,ccw_id = 2(VSA_ID_CRD_REAR) is crcw.*/
    uint8_t ccw_id;

    /* private */
    uint16_t life;
    

}vsa_crd_node_t;



typedef int (*vsa_app_handler)(vsa_envar_t *p_vsa, void *p_msg);


/*****************************************************************************
 * declaration of global variables and functions                             *
*****************************************************************************/
void vsa_start(void);

rt_err_t vsa_add_event_queue(vsa_envar_t *p_vsa, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv);


#endif /* __CV_VSA_H__ */

