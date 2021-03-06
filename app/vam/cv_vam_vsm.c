/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vam_vsm.c
 @brief  : this file realize the function of vehicle status management
 @author : wangyifeng
 @history:
           2014-6-19    wangyifeng    Created file
           2014-7-30    wanglei       Modified: added evam msg process
           ...
******************************************************************************/
#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "vsm"
#include "cv_osal_dbg.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
//rt_timer_t timer_send_bsm;
//vam_envar_t *p_vam;

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

/* 
    计算本车状态消息自动发送时间，发送周期P=d*1000/r，其中：
    P表示发送周期，单位ms，最小精度10ms，强制取值范围100ms~3000ms
    d表示距离因子，单位m，默认取值为5
    r表示车速，单位为m/s
*/
static uint16_t _cal_peroid_from_speed(float speed, uint8_t bsm_boardcast_saftyfactor)
{
    uint16_t period = 100;
    if(0 == speed)
    {
        period = 3000;
    }
    else
    {
        /* parameter speed, unit: km/h */
        period = bsm_boardcast_saftyfactor*3600/speed;
    }
    period = period < 100 ? 100 : (period > 3000 ? 3000 : period/10*10);
    return period;
}

void vsm_start_bsm_broadcast(vam_envar_t *p_vam)
{
    uint16_t period;
    
    if(p_vam->working_param.bsm_boardcast_mode == BSM_BC_MODE_AUTO){
        //calcute peroid from speed
        period = _cal_peroid_from_speed(p_vam->local.speed, p_vam->working_param.bsm_boardcast_saftyfactor);
    }
    else if (p_vam->working_param.bsm_boardcast_mode == BSM_BC_MODE_FIXED){
        period = p_vam->working_param.bsm_boardcast_period;
    }
 
    p_vam->bsm_send_period_ticks = MS_TO_TICK(period);

    osal_timer_change(p_vam->timer_send_bsm, p_vam->bsm_send_period_ticks);
    osal_timer_start(p_vam->timer_send_bsm);
}

void vsm_stop_bsm_broadcast(vam_envar_t *p_vam)
{
    osal_timer_stop(p_vam->timer_send_bsm);
}

void vsm_pause_bsm_broadcast(vam_envar_t *p_vam)
{
    uint16_t period;
    rt_tick_t ticks;

    if (!(p_vam->flag & VAM_FLAG_TX_BSM_PAUSE))
    {
        p_vam->flag |= VAM_FLAG_TX_BSM_PAUSE;
        period = p_vam->working_param.bsm_pause_hold_time;
        ticks = SECOND_TO_TICK(period);      
        osal_timer_change(p_vam->timer_bsm_pause, ticks);
        osal_timer_start(p_vam->timer_bsm_pause);
    }
}

void timer_send_bsm_callback(void* parameter)
{
    vam_envar_t *p_vam = (vam_envar_t *)parameter;

    static uint8_t count = VAM_NO_ALERT_EVAM_TX_TIMES;
    if (p_vam->flag&VAM_FLAG_TX_BSM_ALERT && p_vam->local.alert_mask == 0){
        /* 发送VAM_NO_ALERT_EVAM_TX_TIMES次后停止发送EVAM消息数据 */
        if(0 == count--){
            p_vam->flag &= ~VAM_FLAG_TX_BSM_ALERT;
            /* change bsm timer time */
            vsm_update_bsm_bcast_timer(p_vam);
        }        
    }

    if ((p_vam->flag&VAM_FLAG_TX_BSM)&&(!(p_vam->flag&VAM_FLAG_TX_BSM_PAUSE))){
        #ifdef RSU_TEST
        vam_add_event_queue(p_vam, VAM_MSG_RCPTX, 0, RCP_MSG_ID_RSA, NULL);
        #else
        vam_add_event_queue(p_vam, VAM_MSG_RCPTX, 0, RCP_MSG_ID_BSM, NULL);
        #endif
    }
}

/* update timeout time of the bsm broadcast timer*/
void vsm_update_bsm_bcast_timer(vam_envar_t *p_vam)
{
    uint16_t period;
    uint32_t timeout;
    if(p_vam->working_param.bsm_boardcast_mode == BSM_BC_MODE_AUTO)
    {
        period = _cal_peroid_from_speed(p_vam->local.speed, p_vam->working_param.bsm_boardcast_saftyfactor);
    }
    else
    {
        if(p_vam->flag & VAM_FLAG_TX_BSM_ALERT)
        {
            period = p_vam->working_param.evam_broadcast_peroid;
        }
        else
        {
            period = p_vam->working_param.bsm_boardcast_period;
        }
    }
    timeout = MS_TO_TICK(period);
    if(p_vam->bsm_send_period_ticks != timeout)
    {
        osal_timer_change(p_vam->timer_send_bsm, timeout);
        p_vam->bsm_send_period_ticks = timeout;
    }
}

void timer_bsm_pause_callback(void* parameter)
{
    vam_envar_t *p_vam = (vam_envar_t *)parameter;

    if (p_vam->flag & VAM_FLAG_TX_BSM){
        p_vam->flag &= ~VAM_FLAG_TX_BSM_PAUSE;
    }
}

void timer_gps_life_callback(void* parameter)
{
    vam_envar_t *p_vam = (vam_envar_t *)parameter;

    if (p_vam->flag & VAM_FLAG_GPS_FIXED){
        p_vam->flag &= ~VAM_FLAG_GPS_FIXED;
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "gps is lost!\n");
        if (p_vam->evt_handler[VAM_EVT_GPS_STATUS]){
            (p_vam->evt_handler[VAM_EVT_GPS_STATUS])((void *)0);
        }
    }
}


void vsm_start_evam_broadcast(vam_envar_t *p_vam)
{
    uint16_t period;
    rt_tick_t ticks;
    if(p_vam->working_param.evam_broadcast_type == BSM_BC_MODE_AUTO){
        period = _cal_peroid_from_speed(p_vam->local.speed, p_vam->working_param.bsm_boardcast_saftyfactor);
    }
    else if (p_vam->working_param.evam_broadcast_type == BSM_BC_MODE_FIXED){
        period = p_vam->working_param.evam_broadcast_peroid;
    }

    ticks = MS_TO_TICK(period);
    osal_timer_change(p_vam->timer_send_evam, ticks);
    osal_timer_start(p_vam->timer_send_evam);
}

void timer_send_evam_callback(void* parameter)
{
    static uint16_t mask = 0;
    vam_envar_t *p_vam = (vam_envar_t *)parameter;
    static uint8_t count = VAM_NO_ALERT_EVAM_TX_TIMES;
    if (p_vam->flag&VAM_FLAG_TX_EVAM)
    {
        /* broadcast evam, then pause bsm broadcast */
        if((1 == p_vam->working_param.bsm_pause_mode) && (p_vam->local.alert_mask != mask))
        {
            vsm_pause_bsm_broadcast(p_vam);
            mask = p_vam->local.alert_mask;
        }

        vam_add_event_queue(p_vam, VAM_MSG_RCPTX, 0, RCP_MSG_ID_EVAM, NULL);
        /* 所有alter已取消 */
        if(p_vam->local.alert_mask == 0)
        {
            /* 发送VAM_NO_ALERT_EVAM_TX_TIMES次后停止发送EVAM消息数据 */
            if(0 == count--)
            {
                p_vam->flag &= ~VAM_FLAG_TX_EVAM;
                osal_timer_stop(p_vam->timer_send_evam);

                /* don't wait timer_bsm_pause timeout. restart to send bsm */
                if(p_vam->flag & VAM_FLAG_TX_BSM_PAUSE)
                {
                    p_vam->flag &= ~VAM_FLAG_TX_BSM_PAUSE;
                    osal_timer_stop(p_vam->timer_bsm_pause);
                }
            }
        }
        else
        {
            count = VAM_NO_ALERT_EVAM_TX_TIMES;
        }
    }
}


vam_sta_node_t *vam_find_sta(vam_envar_t *p_vam, uint8_t *temporary_id)
{
    vam_sta_node_t *p_sta = NULL, *pos;

	list_for_each_entry(pos, vam_sta_node_t, &p_vam->neighbour_list, list){
        if (memcmp(pos->s.pid, temporary_id, RCP_TEMP_ID_LEN) == 0){
            p_sta = pos;
            break;
        }
	}

    /* not found, allocate new one */
    if (p_sta == NULL){

        osal_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);
    	if (!list_empty(&p_vam->sta_free_list)) {
    		p_sta = list_first_entry(&p_vam->sta_free_list, vam_sta_node_t, list);
    		list_move(&p_sta->list, &p_vam->neighbour_list);
    	}
        osal_sem_release(p_vam->sem_sta);

        if (p_sta){
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "one neighbour join\n");
            memcpy(p_sta->s.pid, temporary_id, RCP_TEMP_ID_LEN);        
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: no free sta.\n", __FUNCTION__);
        }
    }

    return p_sta;
}


void vam_update_sta(vam_envar_t *p_vam)
{
    vam_sta_node_t *p_sta = NULL;
    list_head_t *pos;
    list_head_t *head = &p_vam->neighbour_list;
    vam_stastatus_t sta;
    static uint8_t gotNeighbour = 0;
    uint8_t isEmpty = 1;

    if (osal_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER) != RT_EOK){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "semaphor return failed\n");
        return;
    }

    for (pos = head->next; pos != (head); ){
        isEmpty = 0;
        gotNeighbour = 1;
        /* must prefatch the next pointer */
        p_sta = (vam_sta_node_t *)pos;
        pos = pos->next;

        if(p_sta->life)
            p_sta->life--;
        if(p_sta->alert_life)
            p_sta->alert_life--;
        
        if ((p_sta->alert_life == 0) && (p_sta->s.alert_mask) ){
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "one neighbour's alert is timeout to canceled.\n");  
            p_sta->s.alert_mask = 0;
            memcpy(&sta, &p_sta->s, sizeof(vam_stastatus_t));
            /* one neighbours's alert msg timeout */
            if(p_vam->evt_handler[VAM_EVT_PEER_ALARM]){
                (p_vam->evt_handler[VAM_EVT_PEER_ALARM])(&sta);
            }  
        }

        if (p_sta->life == 0 && p_sta->alert_life == 0){
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "one neighbour is kick out\n");
            list_move_tail(&p_sta->list, &p_vam->sta_free_list);
        }
    }
    osal_sem_release(p_vam->sem_sta);

    /* neighbour list turn to empty */
    if(gotNeighbour && isEmpty)
    {
        if(p_vam->evt_handler[VAM_EVT_PEER_UPDATE]){
            (p_vam->evt_handler[VAM_EVT_PEER_UPDATE])(NULL);
        }  
        gotNeighbour = 0;
    }
    
}

void timer_neigh_time_callback(void* parameter)
{
    vam_envar_t *p_vam = (vam_envar_t *)parameter;

    vam_add_event_queue(p_vam, VAM_MSG_NEIGH_TIMEOUT, 0, 0, NULL);
}


void vam_list_sta(void)
{
    vam_envar_t *p_vam = &p_cms_envar->vam;
    vam_sta_node_t *p_sta = NULL;
    int i = 0;

    if (osal_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER) != RT_EOK){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: semaphor return failed\n", __FUNCTION__);
        return;
    }

	list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->sta_free_list, list){
        i++;
	}

    osal_printf("free sta node:%d\n", i);
    osal_printf("neighbor node:%d\n", VAM_NEIGHBOUR_MAXNUM - i);

	list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->neighbour_list, list){
        osal_printf("STA:[%02x-%02x-%02x-%02x], life:%d, alert_life:%d alert_mask:%d\n",\
            p_sta->s.pid[0],p_sta->s.pid[1],p_sta->s.pid[2],p_sta->s.pid[3],\
            p_sta->life, p_sta->alert_life, p_sta->s.alert_mask);
	}
    osal_sem_release(p_vam->sem_sta);
}
FINSH_FUNCTION_EXPORT(vam_list_sta, list all neighbour sta);


