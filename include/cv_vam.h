/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vam.h
 @brief  : the definition of vehicle application middleware
 @author : wangyifeng
 @history:
           2014-6-17    wangyifeng    Created file
           ...
******************************************************************************/
#ifndef __CV_VAM_H__
#define __CV_VAM_H__

#include "list.h"
#include "nmea.h"
#include "cv_rcp.h"

/*****************************************************************************
 * definition of micro                                                       *
*****************************************************************************/
#define RCP_TEMP_ID_LEN 4
#define RCP_MACADR_LEN 8

#define RCP_MSG_ID_BSM   0x02
#define RCP_MSG_ID_EVAM  0x05


#define BSM_BC_MODE_DISABLE 0
#define BSM_BC_MODE_AUTO    1
#define BSM_BC_MODE_FIXED   2

#define VAM_FLAG_RX           (0x0001)
#define VAM_FLAG_TX_BSM       (0x0002)
#define VAM_FLAG_TX_BSM_PAUSE (0x0004)
#define VAM_FLAG_GPS_FIXED    (0x0008)
#define VAM_FLAG_TX_EVAM      (0x0010)
#define VAM_FLAG_XXX          (0x0020)


#define VAM_NEIGHBOUR_MAXNUM   (4)  //
#define VAM_NEIGHBOUR_MAXLIFE  (15)  //unit: second


/* BEGIN: Added by wanglei, 2014/8/1 */
#define VAM_REMOTE_ALERT_MAXLIFE  (5)  //unit: second
#define VAM_NO_ALERT_EVAM_TX_TIMES (5) //取消所有警告的evam消息发送次数

#define VAM_ALERT_MASK_VBD (0x1)
#define VAM_ALERT_MASK_EBD (0x2)
/* END:   Added by wanglei, 2014/8/1 */

enum VAM_EVT{
    VAM_EVT_LOCAL_UPDATE = 0,
    VAM_EVT_PEER_UPDATE,
    VAM_EVT_PEER_ALARM,
    VAM_EVT_GPS_STATUS,
    VAM_EVT_GSNR_EBD_DETECT, 
    VAM_EVT_MAX
};

/*****************************************************************************
 * definition of struct                                                      *
*****************************************************************************/

typedef struct _vam_position{
    float lat;
    float lon ;
    float elev;
    float accu;
}vam_position_t;

typedef float vam_dir_t ;
typedef float vam_speed_t ;

typedef struct _vam_acce{
    float lon;
    float lat;
    float vert;
    float yaw;
}vam_acce_t;


typedef struct _vam_stastatus{
    uint8_t pid[RCP_TEMP_ID_LEN];  //temporary ID
    uint16_t timestamp;
    vam_position_t  pos ;
    float  dir;
    float  speed;
    vam_acce_t  acce;
    uint16_t alert_mask;  //bit0-VBD, bit1-EBD;  1-active, 0-cancel; 同evam中alert_mask
}vam_stastatus_t;

typedef struct _vam_sta_node{
    /* !!!DON'T modify it!!! */
    list_head_t list;

    vam_stastatus_t s;

    /* private */
    uint16_t life;
    uint16_t alert_life;
    /* os related */
}vam_sta_node_t;


typedef struct _vam_config{

    /* 
        Basic Safty Message TX function:    
    */
    uint8_t bsm_hops; //BSM消息最大跳数；

    uint8_t bsm_boardcast_mode;  /* 0 - disable, 1 - auto, 2 - fixed period */
    
    uint8_t bsm_boardcast_saftyfactor;  /* 1~10 */
    uint8_t bsm_pause_mode;  /* 0 - disable, 1-tx evam enable, 2-rx evam pause bsm*/
    uint8_t bsm_pause_hold_time;  /* unit:s */
    uint16_t bsm_boardcast_period;  /* 100~3000, unit:ms, min accuracy :10ms */
    /* 
        Emergency Vehicle Alert Message TX function:    
    */
    uint8_t evam_hops; //EVAM消息最大跳数；
	
	uint8_t evam_broadcast_type;  /* 0 - disable, 1 - auto, 2 - fixed period */
	
	uint16_t evam_broadcast_peroid;  //EVAM消息广播周期 ms
    

}vam_config_t;

typedef struct _rcp_rxinfo {
	uint8_t src[RCP_MACADR_LEN]; //源地址
	uint8_t hops;  //实际传输转发跳数
	uint8_t prority;  //发送优先级
	uint8_t channel; //发送信道
	uint8_t datarate;  //发送速率
	uint8_t rssi;  //接收信号强度
}rcp_rxinfo_t;


typedef struct _rcp_txinfo {
	uint8_t dest[RCP_MACADR_LEN]; //目的地址
	uint8_t hops; //最大转发跳数
	uint8_t prority;  //发送优先级
	uint8_t channel; //发送信道
	uint8_t datarate; // 发送速率
	uint8_t txpower; // 发射功率
}rcp_txinfo_t;




typedef void (*vam_evt_handler)(void *);



typedef struct _vam_envar{

    /* working_param */
    vam_config_t working_param;

    int flag;

    uint16_t bsm_send_period_ticks;

    rcp_msg_basic_safty_t bsm;
    uint8_t tx_msg_cnt;

    rcp_msg_emergency_vehicle_alert_t evam;
    uint8_t tx_evam_msg_cnt;

    vam_stastatus_t local;
    vam_sta_node_t remote[VAM_NEIGHBOUR_MAXNUM];

    list_head_t sta_free_list;
    list_head_t neighbour_list;

    vam_evt_handler evt_handler[VAM_EVT_MAX];

    /* os related */
    rt_thread_t task_vam;
    rt_mq_t queue_vam;

    osal_timer_t *timer_send_bsm;
    rt_timer_t timer_send_evam;
    rt_timer_t timer_bsm_pause;
    rt_timer_t timer_gps_life;
    rt_timer_t timer_neighbour_life;

    rt_sem_t sem_sta;

}vam_envar_t;


/*****************************************************************************
 * declaration of global variables and functions                             *
*****************************************************************************/

extern vam_envar_t *p_vam_envar;

void vsm_start_bsm_broadcast(vam_envar_t *p_vam);
void vsm_stop_bsm_broadcast(vam_envar_t *p_vam);
void vsm_update_bsm_bcast_timer(vam_envar_t *p_vam);
void vsm_pause_bsm_broadcast(vam_envar_t *p_vam);

rt_err_t vam_add_event_queue(vam_envar_t *p_vam, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv);
rt_err_t vam_add_event_queue_2(vam_envar_t *p_vam, void *p_msg);

int32_t rcp_send_bsm(vam_envar_t *p_vam);

vam_sta_node_t *vam_find_sta(vam_envar_t *p_vam, uint8_t *temporary_id);
void vam_update_sta(vam_envar_t *p_vam);

int rcp_parse_msg(vam_envar_t *p_vam,
                  rcp_rxinfo_t *rxinfo, 
                  uint8_t *databuf, 
                  uint32_t datalen);

void lip_gps_proc(vam_envar_t *p_vam, uint8_t *databuf, uint32_t len);
void lip_update_local(t_nmea_rmc *p_rmc, float *p_accu);
void lip_update_local_acc(float x, float y, float z);

double vsm_get_distance(vam_position_t *p_src, vam_position_t *p_dest);
double vsm_get_relative_pos(vam_stastatus_t *p_src, vam_stastatus_t *p_dest,uint8_t vsa_print_en);
double vsm_get_relative_dir(vam_stastatus_t *p_src, vam_stastatus_t *p_dest);
int8_t vsm_get_rear_dir(vam_stastatus_t *p_dest);


int32_t vam_start(void);
int32_t vam_set_event_handler(uint32_t evt, vam_evt_handler callback);
int32_t vam_get_peer_relative_pos(uint8_t *pid,uint8_t vsa_print_en);
int32_t vam_get_peer_relative_dir(uint8_t *pid);
int32_t vam_get_peer_alert_status(uint16_t *alert_mask);
int32_t vam_active_alert(uint32_t alerttype);
int32_t vam_cancel_alert(uint32_t alerttype);

int32_t rcp_send_evam(vam_envar_t *p_vam);
int vam_rcp_recv(rcp_rxinfo_t *rxinfo, uint8_t *databuf, uint32_t datalen);
void vsm_start_evam_broadcast(vam_envar_t *p_vam);
void vam_gsnr_ebd_detected(uint8_t status);


#endif /* __CV_VAM_H__ */

