/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vam_api.c
 @brief  : this file include the api interface of vehicle application middleware
 @author : wangyifeng
 @history:
           2014-6-15    wangyifeng    Created file
           2014-8-01    wanglei       Modified file
           ...
******************************************************************************/
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/



int32_t vam_start(void)
{
    rt_kprintf("%s: --->\n", __FUNCTION__);

    vam_add_event_queue(&p_cms_envar->vam, VAM_MSG_START, 0, 0, 0);
    
    return 0;
}
FINSH_FUNCTION_EXPORT(vam_start, vam module start);

int32_t vam_stop(void)
{
   rt_kprintf("%s: --->\n", __FUNCTION__);
   vam_add_event_queue(&p_cms_envar->vam, VAM_MSG_STOP, 0, 0, 0);
   return 0;
}
FINSH_FUNCTION_EXPORT(vam_stop, vam module stop);



int32_t vam_get_config(vam_config_t *config)
{
    return 0;
}


int32_t vam_set_config(vam_config_t *config)
{
    return 0;
}

int32_t vam_set_event_handler(uint32_t evt, vam_evt_handler callback)
{
    if ((evt >= VAM_EVT_MAX)||(!callback)){
        return -1;
    }

    p_vam_envar->evt_handler[evt] = callback;

    return 0;
}

int32_t vam_get_local_status(vam_stastatus_t *local)
{
    if(!local){
        return -1;
    }
    vam_stastatus_t *p_local = &(p_vam_envar->local);
    memcpy(local->pid, p_local->pid, RCP_TEMP_ID_LEN);
    local->timestamp = p_local->timestamp;
    local->dir = p_local->dir;

    memcpy(&local->acce, &p_local->acce, sizeof(vam_acce_t));
    //TBD 后续设计算法进行补偿
    local->speed = p_local->speed;
    memcpy(&local->pos, &p_local->pos, sizeof(vam_position_t));
    return 0;
}


/* 更新本车的状态信息（仅用于当VANET不支持内部解析本地GPS、加速度传感器等功能时） */
int32_t vam_set_local_status(vam_stastatus_t *local)
{
    if(!local){
        return -1;
    }

    memcpy(&(p_vam_envar->local), local, sizeof(vam_stastatus_t));
    return 0;
}

int32_t vam_get_peerlist(vam_stastatus_t **local, uint32_t maxitem, uint32_t *actual)
{
    return 0;
}


int32_t vam_get_peer_status(uint8_t *pid, vam_stastatus_t *local)
{
    vam_envar_t *p_vam = p_vam_envar;
    vam_sta_node_t *p_sta = NULL;

    if(!pid || !local){
        return -1;
    }
    
    rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);

	list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->neighbour_list, list){
        if (memcmp(p_sta->s.pid, pid, RCP_TEMP_ID_LEN)==0){
            memcpy(local, &p_sta->s, sizeof(vam_stastatus_t));
            break;
        }
	}
    rt_sem_release(p_vam->sem_sta);
    
    return 0;
}



int32_t vam_get_peer_relative_pos(uint8_t *pid,uint8_t vsa_print_en)
{
    vam_envar_t *p_vam = p_vam_envar;
    vam_sta_node_t *p_sta = NULL;
    vam_stastatus_t sta;

    rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);

	list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->neighbour_list, list){
        if (memcmp(p_sta->s.pid, pid, RCP_TEMP_ID_LEN)==0){
            memcpy(&sta, &p_sta->s, sizeof(vam_stastatus_t));
            break;
        }
	}
    rt_sem_release(p_vam->sem_sta);

    return (int32_t)vsm_get_relative_pos(&p_vam->local,&sta,vsa_print_en);
}



int32_t vam_get_peer_relative_dir(vam_stastatus_t local,vam_stastatus_t remote)
{
    int32_t delta, r;

    delta = (int32_t)vsm_get_relative_dir(&local,&remote);

    if ((delta <= 10)||(local.speed < 1.0f)||(remote.speed < 1.0f)){
        r = 1;
    }
    else{
        r = -1;
    }

    return r;
}


int32_t vam_get_peer_relative_speed(uint8_t *pid)
{
    if(!pid){
        return -1;
    }

    return 0;
}

int32_t vam_get_peer_absolute_speed(uint8_t *pid)
{
    if(!pid)
        return -1;
    vam_envar_t *p_vam = p_vam_envar;
    vam_sta_node_t *p_sta = NULL;
    vam_stastatus_t sta;

    rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);

	list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->neighbour_list, list){
        if (memcmp(p_sta->s.pid, pid, RCP_TEMP_ID_LEN)==0){
            memcpy(&sta, &p_sta->s, sizeof(vam_stastatus_t));
            break;
        }
	}
    rt_sem_release(p_vam->sem_sta);
    
    return sta.speed;
}


/* BEGIN: Added by wanglei, 2014/8/1 */
/*****************************************************************************
   获取目前所有邻车告警状态, 只有每个邻车都取消告警, 应用层才停止预警
   alert_mask:  bit0-Vehicle Break Down(vbd)
                bit1-Emergency Braking Danger(ebd)
*****************************************************************************/
int32_t vam_get_peer_alert_status(uint16_t *alert_mask)
{
    vam_envar_t *p_vam = p_vam_envar;
    vam_sta_node_t *p_sta = NULL;
    uint16_t mask = 0;
    rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);

	list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->neighbour_list, list){
        mask |= p_sta->s.alert_mask;
	}
    rt_sem_release(p_vam->sem_sta);
    if(p_sta == NULL){
        mask = 0;
    }
    *alert_mask = mask;
    return 0;
}

/*****************************************************************************
   alerttype:   0-Vehicle Break Down(vbd)
                1-Emergency Braking Danger(ebd)
*****************************************************************************/
int32_t vam_active_alert(uint32_t alerttype)
{
    vam_envar_t *p_vam = p_vam_envar;

    p_vam->local.alert_mask |= (1 << alerttype); 
    if(!(p_vam->flag & VAM_FLAG_TX_EVAM))
    {
        vsm_start_evam_broadcast(p_vam);
        p_vam->flag |= VAM_FLAG_TX_EVAM;
    }
        
    return 0;
}

/*****************************************************************************
   alerttype:   0-Vehicle Break Down(vbd)
                1-Emergency Braking Danger(ebd)
*****************************************************************************/
int32_t vam_cancel_alert(uint32_t alerttype)
{
    vam_envar_t *p_vam = p_vam_envar;
    p_vam->local.alert_mask &= ~(1 << alerttype); 
    return 0;
}


/* stop evam msg sending */
void vam_stop_alert()
{
    vam_envar_t *p_vam = p_vam_envar;
    p_vam->flag &= ~VAM_FLAG_TX_EVAM;
    rt_timer_stop(p_vam->timer_send_evam);
}

void vam_gsnr_ebd_detected(uint8_t status)
{
    vam_envar_t *p_vam = p_vam_envar;
    if(p_vam->evt_handler[VAM_EVT_GSNR_EBD_DETECT]){
        (p_vam->evt_handler[VAM_EVT_GSNR_EBD_DETECT])(&p_vam->local);
    }
}

/* 暂留, 调试EVAM用, 发送不同告警 */
void vam_alert(int mode, int type)
{
    if(mode == 0)
    {
        vam_stop_alert();
    }
    else if(mode == 1)
    {
        vam_active_alert(type);
    }
    else
    {
        vam_cancel_alert(type);
    }
}

/* shell cmd for debug */
#ifdef RT_USING_FINSH
#include <finsh.h>

//FINSH_FUNCTION_EXPORT(vam_alert, debug: vam alert send);
#endif


