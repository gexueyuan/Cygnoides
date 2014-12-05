/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vam_main.c
 @brief  : this file include main function of vehicle application middleware
           
 @author : wangyifeng
 @history:
           2014-6-19    wangyifeng    Created file
           ...
******************************************************************************/
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"

#include "nmea.h"

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
#define BSM_SEND_PERIOD_DEFAULT      SECOND_TO_TICK(1)
#define BSM_PAUSE_HOLDTIME_DEFAULT   SECOND_TO_TICK(5)
#define BSM_GPS_LIFE_DEFAULT         SECOND_TO_TICK(5)
#define NEIGHBOUR_LIFE_ACCUR         SECOND_TO_TICK(1)
#define EVAM_SEND_PERIOD_DEFAULT     MS_TO_TICK(50)


extern void timer_send_bsm_callback(void* parameter);
extern void timer_bsm_pause_callback(void* parameter);
extern void timer_gps_life_callback(void* parameter);
extern void timer_neigh_time_callback(void* parameter);
extern void timer_send_evam_callback(void* parameter);

vam_envar_t *p_vam_envar;

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/



void vam_main_proc(vam_envar_t *p_vam, sys_msg_t *p_msg)
{
    switch(p_msg->id){
        case VAM_MSG_START:

            rt_kprintf("%s: start\n", __FUNCTION__);

            p_vam->flag |= VAM_FLAG_RX;
            rt_timer_start(p_vam->timer_neighbour_life);
            
            if (p_vam->working_param.bsm_boardcast_mode != BSM_BC_MODE_DISABLE){
                p_vam->flag |= VAM_FLAG_TX_BSM;
                vsm_start_bsm_broadcast(p_vam);
            }
            
            break;

        case VAM_MSG_STOP:

            if (p_vam->flag && VAM_FLAG_TX_BSM){
                vsm_stop_bsm_broadcast(p_vam);
            }

            p_vam->flag &= ~(VAM_FLAG_RX|VAM_FLAG_TX_BSM);
            rt_timer_stop(p_vam->timer_neighbour_life);
            
            break;

        case VAM_MSG_RCPTX:
            if (p_msg->argc == RCP_MSG_ID_BSM){
                rcp_send_bsm(p_vam);
            }
            if (p_msg->argc == RCP_MSG_ID_EVAM){
                rcp_send_evam(p_vam);
            }

            break;

        case VAM_MSG_RCPRX:
            rcp_parse_msg(p_vam, (rcp_rxinfo_t *)p_msg->argv, \
                          (uint8_t *)p_msg->argc, p_msg->len);
            
            break;

        case VAM_MSG_NEIGH_TIMEOUT:
            vam_update_sta(p_vam);
            break;

        case VAM_MSG_GPSDATA:
            lip_gps_proc(p_vam, (uint8_t *)p_msg->argv, p_msg->len);

            break;
            
        default:
            break;
    }
}

void rt_vam_thread_entry(void *parameter)
{
    rt_err_t err;
    sys_msg_t msg, *p_msg = &msg;
    vam_envar_t *p_vam = (vam_envar_t *)parameter;

    rt_kprintf("%s: ---->\n", __FUNCTION__);

	while(1){
        err = rt_mq_recv(p_vam->queue_vam, p_msg, sizeof(sys_msg_t), RT_WAITING_FOREVER);
        if (err == RT_EOK){
            vam_main_proc(p_vam, p_msg);
            //rt_free(p_msg);
        }
        else{
            rt_kprintf("%s: rt_mq_recv error [%d]\n", __FUNCTION__, err);
        }
	}
}

rt_err_t vam_add_event_queue(vam_envar_t *p_vam, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv)
{
    rt_err_t err = RT_ENOMEM;
    sys_msg_t msg;

    msg.id = msg_id;
    msg.len = msg_len;
    msg.argc = msg_argc;
    msg.argv = msg_argv;
    err = rt_mq_send(p_vam->queue_vam, &msg, sizeof(sys_msg_t));

    if (err != RT_EOK){
        rt_kprintf("%s: failed=[%d], msg=%04x\n", __FUNCTION__, err, msg_id);
    }

    return err;
}

rt_err_t vam_add_event_queue_2(vam_envar_t *p_vam, void *p_msg)
{
    rt_err_t err;
    
    err = rt_mq_send(p_vam->queue_vam, p_msg, sizeof(sys_msg_t));
    if (err != RT_EOK){
        rt_kprintf("%s: failed=[%d], msg=%04x\n", __FUNCTION__, err, \
                  ((sys_msg_t *)p_msg)->id);
    }

    return err;
}

/*****************************************************************************
 @funcname: vam_init
 @brief   : vam module initial
 @param   : None
 @return  : 
*****************************************************************************/

void vam_init(void)
{
    int i;
    vam_envar_t *p_vam = &p_cms_envar->vam;

    p_vam_envar = p_vam;
    
    memset(p_vam, 0, sizeof(vam_envar_t));
    memcpy(&p_vam->working_param, &p_cms_param->vam, sizeof(vam_config_t));
    memcpy(p_vam->local.pid, p_cms_param->pid, RCP_TEMP_ID_LEN);

    INIT_LIST_HEAD(&p_vam->neighbour_list);
    INIT_LIST_HEAD(&p_vam->sta_free_list);
    for(i = 0;i< VAM_NEIGHBOUR_MAXNUM;i++){
        list_add_tail(&p_vam->remote[i].list, &p_vam->sta_free_list);
    }

    /* os object for vam */
    p_vam->queue_vam = rt_mq_create("q-vam", sizeof(sys_msg_t), VAM_QUEUE_SIZE, RT_IPC_FLAG_FIFO);
    RT_ASSERT(p_vam->queue_vam != RT_NULL);
    
	 p_vam->task_vam = rt_thread_create("t-vam",
                           rt_vam_thread_entry, p_vam,
                           RT_VAM_THREAD_STACK_SIZE, RT_VAM_THREAD_PRIORITY, 20);
    RT_ASSERT(p_vam->task_vam != RT_NULL)
    rt_thread_startup(p_vam->task_vam);

    p_vam->timer_send_bsm = osal_timer_create("tm-sb",timer_send_bsm_callback,p_vam,\
        BSM_SEND_PERIOD_DEFAULT,TRUE); 					
    RT_ASSERT(p_vam->timer_send_bsm != NULL);

    p_vam->timer_bsm_pause = rt_timer_create("tm-bp",timer_bsm_pause_callback,p_vam,\
        BSM_PAUSE_HOLDTIME_DEFAULT,RT_TIMER_FLAG_ONE_SHOT); 					
    RT_ASSERT(p_vam->timer_send_bsm != RT_NULL);

    p_vam->timer_send_evam = rt_timer_create("tm-se",timer_send_evam_callback, p_vam,\
        EVAM_SEND_PERIOD_DEFAULT,RT_TIMER_FLAG_PERIODIC); 					
    RT_ASSERT(p_vam->timer_send_evam != RT_NULL);

    p_vam->timer_gps_life = rt_timer_create("tm-gl",timer_gps_life_callback,p_vam,\
        BSM_GPS_LIFE_DEFAULT,RT_TIMER_FLAG_ONE_SHOT); 					
    RT_ASSERT(p_vam->timer_send_bsm != RT_NULL);

    p_vam->timer_neighbour_life = rt_timer_create("tm-nl",timer_neigh_time_callback,p_vam,\
        NEIGHBOUR_LIFE_ACCUR,RT_TIMER_FLAG_PERIODIC); 					
    RT_ASSERT(p_vam->timer_send_bsm != RT_NULL);

    p_vam->sem_sta = rt_sem_create("s-sta", 1, RT_IPC_FLAG_PRIO);
    RT_ASSERT(p_vam->sem_sta != RT_NULL);

	rt_kprintf("vam module initial\n");
}

/*****************************************************************************
 @funcname: vam_deinit
 @brief   : vam module unstall
 @param   : None
 @return  : 
*****************************************************************************/
void vam_deinit()
{
	rt_kprintf("vam module initial\n");
}


void dump_pos(vam_stastatus_t *p_sta)
{
    char str[64];

    rt_kprintf("------------sta---------\n");
    rt_kprintf("PID:%02x-%02x-%02x-%02x\n", p_sta->pid[0], p_sta->pid[1]\
                                          , p_sta->pid[2], p_sta->pid[3]);
    sprintf(str,"%f", p_sta->pos.lat);
    rt_kprintf("pos.lat:%s\n", str);
    sprintf(str,"%f", p_sta->pos.lon);
    rt_kprintf("pos.lon:%s\n", str);
    sprintf(str,"%f", p_sta->pos.elev);
    rt_kprintf("pos.elev:%s\n", str);
    sprintf(str,"%f", p_sta->pos.accu);
    rt_kprintf("pos.accu:%s\n", str);
    sprintf(str,"%f", p_sta->dir);
    rt_kprintf("pos.heading:%s\n", str);
    sprintf(str,"%f", p_sta->speed);
    rt_kprintf("pos.speed:%s\n", str);
    rt_kprintf("------------end---------\n");
}

