/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vsa_main.c
 @brief  : this file realize the function of vehicle safty application
 @author : wangyifeng
 @history:
           2014-6-19    wangyifeng    Created file
           ...
******************************************************************************/
#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_TRACE
#define MODULE_NAME "vsa"
#include "cv_osal_dbg.h"
//OSAL_DEBUG_ENTRY_DEFINE(vsa)


#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"
#include "cv_vsa.h"
#include "key.h"
#include "math.h"

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
#define VSA_TIMER_PERIOD         SECOND_TO_TICK(1)
#define VSA_EBD_SEND_PERIOD      SECOND_TO_TICK(5)
#define VSA_POS_PERIOD           MS_TO_TICK(10)
#define DIRECTION_DIVIDE         22.5f
#define PI 3.1415926


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/


/*****************************************************************************
 @funcname: vsa_position_classify
 @brief   : describe target's postion
 @param   : None
 @return  : 
*****************************************************************************/
uint32_t  vsa_position_classify(const vam_stastatus_t *local, const vam_stastatus_t *remote,double distance_1_2)
{
//    uint32_t error = POSITION_ERROR;

    double lat1, lng1, lat2, lng2, lat3, lng3;
    double distance_2_3;
    double angle, delta;

    /* reference point */
    lat1 = local->pos.lat;
    lng1 = local->pos.lon;

    /* destination point */
    lat2 = remote->pos.lat;
    lng2 = remote->pos.lon;

    /* temp point */
    lat3 = lat1;
    lng3 = lng2;

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
    if (angle > local->dir){
        delta = angle - local->dir;
    }
    else{
        delta = local->dir - angle;
    }

    if((delta >360.0f)||(delta <0.0f)) return POSITION_ERROR;
/****************
    if (delta > 180){
        delta = 360 - delta;
    }
**********************/

/*divide posiotion to 8 pieces*/


    if((delta > 15*DIRECTION_DIVIDE)||(delta <= DIRECTION_DIVIDE))
       return AHEAD;
    else if((delta > DIRECTION_DIVIDE)&&(delta <= 3*DIRECTION_DIVIDE))
       return AHEAD_RIGHT;
    else if((delta > 3*DIRECTION_DIVIDE)&&(delta <= 5*DIRECTION_DIVIDE))
       return RIGHT;
    else if((delta > 5*DIRECTION_DIVIDE)&&(delta <= 7*DIRECTION_DIVIDE))
       return BEHIND_RIGHT;
    else if((delta > 7*DIRECTION_DIVIDE)&&(delta <= 9*DIRECTION_DIVIDE))
       return BEHIND;
    else if((delta > 9*DIRECTION_DIVIDE)&&(delta <= 11*DIRECTION_DIVIDE))
       return BEHIND_LEFT;
    else if((delta > 11*DIRECTION_DIVIDE)&&(delta <= 13*DIRECTION_DIVIDE))
       return LEFT;
    else if((delta > 13*DIRECTION_DIVIDE)&&(delta <= 15*DIRECTION_DIVIDE))
       return AHEAD_LEFT;    
    else return POSITION_ERROR;
}



/*****************************************************************************
 @funcname: timer_preprocess_pos_callback
 @brief   : preprocess for vsa module,information from neighbourlist
 @param   : None
 @return  : 
*****************************************************************************/
void  timer_preprocess_pos_callback( void *neighbour_info )
{
    
    vam_envar_t *p_vam = p_vam_envar;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    vam_sta_node_t *p_sta = NULL;
    vsa_position_node_t *p_pnt = NULL;
    double temp_dis;
    int8_t i = 0;

    if(!list_empty(&p_vam->neighbour_list)){
        
        vam_get_local_status(&p_vam->local);

        rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);

        list_for_each_entry(p_sta, vam_sta_node_t, &p_vam->neighbour_list, list){

            if(i > (VAM_NEIGHBOUR_MAXNUM - 1))
                return;
            else
                p_pnt = &p_vsa->position_node[i++];
      
            memcpy(p_pnt->vsa_position.pid,p_sta->s.pid,RCP_TEMP_ID_LEN);
      
            temp_dis = getDistanceVer2((double)p_vam->local.pos.lat,(double)p_vam->local.pos.lon,
                        (double)p_sta->s.pos.lat,(double)p_sta->s.pos.lon);
          
            p_pnt->vsa_position.vsa_location = vsa_position_classify(&p_vam->local,&p_sta->s,temp_dis);
      
            p_pnt->vsa_position.relative_speed = p_vam->local.speed - p_sta->s.speed;
            
            p_pnt->vsa_position.lat_offset = fabs(p_vam->local.pos.lat - p_sta->s.pos.lat);
      
            p_pnt->vsa_position.lon_offset = fabs(p_vam->local.pos.lat - p_sta->s.pos.lat);
      
            p_pnt->vsa_position.linear_distance = (uint32_t)temp_dis;

            p_pnt->vsa_position.safe_distance = (int32_t)((p_vam->local.speed*2.0f - p_sta->s.speed)*p_vsa->working_param.crd_saftyfactor*1000.0f/3600.0f);
                
            p_pnt->vsa_position.dir = p_sta->s.dir;
      
            p_pnt->vsa_position.flag_dir = vam_get_peer_relative_dir(&p_vam->local,&p_sta->s);
      
            list_add(&p_pnt->list,&p_vsa->position_list);

        }
        rt_sem_release(p_vam->sem_sta); 

        if (p_vam->evt_handler[VAM_EVT_PEER_UPDATE]){
                (p_vam->evt_handler[VAM_EVT_PEER_UPDATE])(&p_sta->s);
            }
        }
    else {
           if(p_vam->evt_handler[VAM_EVT_PEER_UPDATE]){
               (p_vam->evt_handler[VAM_EVT_PEER_UPDATE])(NULL);
            }
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "no neighour exist!");
    }
}

/*****************************************************************************
 @funcname: vsa_find_pn
 @brief   : find position node in position link list
 @param   : None
 @return  : 
*****************************************************************************/
vsa_position_node_t *vsa_find_pn(vsa_envar_t *p_vsa, uint8_t *temporary_id)
{
    vsa_position_node_t *p_pn = NULL, *pos;

	list_for_each_entry(pos, vsa_position_node_t, &p_vsa->position_list, list){
        if (memcmp(pos->vsa_position.pid, temporary_id, RCP_TEMP_ID_LEN) == 0){
            p_pn = pos;
            break;
        }
	}
    /* not found */
    if (p_pn == NULL){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "vsa position node need updating!");
                        
    }
    return p_pn;
}


void vsa_local_status_update(void* parameter)
{
    vam_stastatus_t *p_sta = (vam_stastatus_t *)parameter;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;

    memcpy(&p_vsa->local, p_sta, sizeof(vam_stastatus_t));
    vsa_add_event_queue(p_vsa, VSA_MSG_LOCAL_UPDATE, 0,0,NULL);
}

void vsa_peer_status_update(void *parameter)
{
    vam_stastatus_t *p_sta = (vam_stastatus_t *)parameter;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;

	if(p_sta)
		{
    		vsa_add_event_queue(p_vsa, VSA_MSG_PEER_UPDATE, 0,0,NULL);
			if(!(p_cms_envar->sys.led_priority&(1<<SYS_MSG_BSM_UPDATE)))
			sys_add_event_queue(&p_cms_envar->sys,SYS_MSG_BSM_UPDATE, 0, HI_OUT_BSM_UPDATE, NULL);
		}
	else
			sys_add_event_queue(&p_cms_envar->sys,SYS_MSG_BSM_UPDATE, 0, HI_OUT_BSM_NONE, NULL);
}

void vsa_gps_status_update(void *parameter)
{
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    vsa_add_event_queue(p_vsa, VSA_MSG_GPS_UPDATE, 0,(uint32_t)parameter,NULL);
}


/* BEGIN: Added by wanglei, 2014/8/1 */
/* modified by gexueyuan,2014/08/05 */
void vsa_peer_alarm_update(void *parameter)
{

	vam_stastatus_t *p_sta = (vam_stastatus_t *)parameter;
	vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    memcpy(&p_vsa->remote, p_sta, sizeof(vam_stastatus_t));
    vsa_add_event_queue(p_vsa, VSA_MSG_ALARM_UPDATE, 0,0,NULL);   
}
/* END:   Added by wanglei, 2014/8/1 */

void vsa_gsnr_ebd_update(void *parameter)
{
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    vsa_add_event_queue(p_vsa, VSA_GSNR_EBD_DETECT, 0,0,NULL);
}

void vsa_start(void)
{
    vam_set_event_handler(VAM_EVT_LOCAL_UPDATE, vsa_local_status_update);
    vam_set_event_handler(VAM_EVT_PEER_UPDATE, vsa_peer_status_update);
    vam_set_event_handler(VAM_EVT_PEER_ALARM, vsa_peer_alarm_update);
    vam_set_event_handler(VAM_EVT_GPS_STATUS, vsa_gps_status_update);
    vam_set_event_handler(VAM_EVT_GSNR_EBD_DETECT, vsa_gsnr_ebd_update);
}

/*****************************************************************************
 @funcname: crd_judge
 @brief   : check and judge the close range danger of vehicles
 @param   : vsa_envar_t *p_vsa  
 @return  : 
            0 - no alert
            1 - alert
*****************************************************************************/
static int ccw_judge(vsa_position_node_t *p_node)
{
    int32_t dis_actual, dis_alert;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    /* put the beginning only in order to output debug infomations */
    dis_actual = p_node->vsa_position.linear_distance;
    dis_alert = p_node->vsa_position.safe_distance;//(int32_t)((p_vsa->local.speed*2.0f - p_vsa->remote.speed)*p_vsa->working_param.crd_saftyfactor*1000.0f/3600.0f);
    /* end */

    if (p_node->vsa_position.local_speed <= p_vsa->working_param.danger_detect_speed_threshold){
		
        return 0;
    }

    if (p_node->vsa_position.remote_speed <= p_vsa->working_param.danger_detect_speed_threshold){
		
        return 0;
    }   
    
    if (p_node->vsa_position.flag_dir < 0){
		
        return 0;
    }

    if((p_node->vsa_position.vsa_location >= AHEAD_LEFT)&&
        (p_node->vsa_position.vsa_location <= AHEAD_RIGHT)){
        
    if (p_node->vsa_position.local_speed <= (p_node->vsa_position.remote_speed +\
                                            p_vsa->working_param.crd_oppsite_speed)){
		
        return 0;
    }



    /* remote is behind of local */
    if (dis_actual <= 0){
		
        return 0;
    }

    if (dis_actual > dis_alert){
        return 0;
    }
	
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Close range danger alert(safty:%d, actual:%d)!!!",\
                        dis_alert, dis_actual,p_vsa->remote.pid[0],p_vsa->remote.pid[1],p_vsa->remote.pid[2],p_vsa->remote.pid[3]);
    
    return VSA_ID_CRD;
    }
    else if((p_node->vsa_position.vsa_location >= BEHIND_LEFT)&&
        (p_node->vsa_position.vsa_location <= BEHIND_RIGHT)){
            

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Close range danger alert(safty:%d, actual:%d)!!!",\
                        dis_alert, dis_actual,p_vsa->remote.pid[0],p_vsa->remote.pid[1],p_vsa->remote.pid[2],p_vsa->remote.pid[3]);
    return VSA_ID_CRD_REAR;
    }    
//	rt_kprintf("Close range danger alert(safty:%d, actual:%d)!!! Id:%d%d%d%d\n", dis_alert, dis_actual,p_vsa->remote.pid[0],p_vsa->remote.pid[1],p_vsa->remote.pid[2],p_vsa->remote.pid[3]);

    return 0;
}

static int rear_end_judge(vsa_envar_t *p_vsa)
{

    int32_t dis_actual, dis_alert;

    /* put the beginning only in order to output debug infomations */
    dis_actual = vam_get_peer_relative_pos(p_vsa->remote.pid,0);
    dis_alert = p_vsa->working_param.crd_rear_distance;//(int32_t)((p_vsa->remote.speed*2.0f - p_vsa->local.speed)*p_vsa->working_param.crd_saftyfactor*1000.0f/3600.0f);
    /* end */

    if (p_vsa->remote.speed < p_vsa->working_param.danger_detect_speed_threshold){
		
        return 0;
    }

	if (p_vsa->local.speed < p_vsa->working_param.danger_detect_speed_threshold)
		return 0;
	
    if ((p_vsa->local.speed + p_vsa->working_param.crd_oppsite_rear_speed) >= p_vsa->remote.speed){
		
        return 0;
    }

    /*local  is behind of remote */
    if (dis_actual >= 0){
		
        return 0;
    }

    if ((-dis_actual) > dis_alert){
        return 0;
    }

	//if(vsm_get_rear_dir(&p_vsa->remote) * dis_actual < 0)
	//	return 0;
	//rt_kprintf("Close range danger alert(safty:%d, actual:%d)!!!\n", dis_alert, dis_actual);

	OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO,\
	    "Rear end danger alert(safty:%d, actual:%d)!!! Id:%d%d%d%d\n",\
	    dis_alert, dis_actual,p_vsa->remote.pid[0],p_vsa->remote.pid[1],\
	    p_vsa->remote.pid[2],p_vsa->remote.pid[3]);

    return 1;

}

static int crd_local_judge(vsa_envar_t *p_vsa)
{
	float relative_speed = 0;
	static  int8_t  send_flag = 1;
	vam_envar_t *p_vam = &p_cms_envar->vam;
	
	if(list_empty(&p_vam->neighbour_list))
		return 0;
		
	relative_speed = p_vsa->local.speed - p_vsa->remote.speed;

    if (p_vsa->remote.speed >= p_vsa->working_param.danger_detect_speed_threshold)		
        return 0;

	if(relative_speed <=0)
		return 0;
		
	if(relative_speed > 100)		
		vsa_add_event_queue(p_vsa, VSA_MSG_PEER_UPDATE, 0,0,NULL);
	else if((relative_speed < 100)&&(relative_speed > 50))
		{
			if(send_flag == 1)
				vsa_add_event_queue(p_vsa, VSA_MSG_PEER_UPDATE, 0,0,NULL);
			send_flag = -send_flag;
	   } 

	return 1;

}

static int ccw_add_list(uint32_t warning_id,vsa_position_node_t *p_pnt)
{
    
    vam_envar_t *p_vam = &p_cms_envar->vam;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
	vsa_crd_node_t *p_crd = NULL,*pos = NULL;
    rt_bool_t  ccw_flag;

    if(warning_id == 0)
        return warning_id;
    
    if (p_vsa->alert_mask & (1<<warning_id)){
        
            rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);
    
            if(list_empty(&p_vsa->crd_list))
                {
                    p_crd = (vsa_crd_node_t*)rt_malloc(sizeof(vsa_crd_node_t));
                    memcpy(p_crd->pid,p_pnt->vsa_position.pid,RCP_TEMP_ID_LEN);
                    p_crd->ccw_id = warning_id;
                    list_add(&p_crd->list,&p_vsa->crd_list);
                }
            else
              list_for_each_entry(pos,vsa_crd_node_t,&p_vsa->crd_list,list)
                    {
                        if(memcmp(p_pnt->vsa_position.pid,pos->pid,RCP_TEMP_ID_LEN) == 0)
                            {
                                ccw_flag = RT_FALSE;
                                break;
                            }   
                        else
                            ccw_flag = RT_TRUE;
                    }
            if(ccw_flag )
                {
                    p_crd = (vsa_crd_node_t*)rt_malloc(sizeof(vsa_crd_node_t));
                    memcpy(p_crd->pid,p_pnt->vsa_position.pid,RCP_TEMP_ID_LEN);
                    p_crd->ccw_id = warning_id;
                    list_add(&p_crd->list,&p_vsa->crd_list);
                }   
            rt_sem_release(p_vam->sem_sta);                 
        /* danger is detected */    
            if (p_vsa->alert_pend & (1<<warning_id)){
            /* do nothing */    
            }
            else{
            /* inform system to start alert */
                p_vsa->alert_pend |= (1<<warning_id);
                sys_add_event_queue(&p_cms_envar->sys, \
                                    SYS_MSG_START_ALERT, 0, warning_id, NULL);
            }

    }
return 0;
}


static int ccw_del_list(uint32_t warning_id,vsa_position_node_t *p_pnt)
{
    
    vam_envar_t *p_vam = &p_cms_envar->vam;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
	vsa_crd_node_t *p_crd = NULL,*pos = NULL;
    rt_bool_t  ccw_flag;

    if(warning_id == 0)
        return warning_id;
    
    if (p_vsa->alert_mask & (1<<warning_id)){

            rt_sem_take(p_vam->sem_sta, RT_WAITING_FOREVER);
            
              list_for_each_entry(pos,vsa_crd_node_t,&p_vsa->crd_list,list)                 
                    {
                        if(memcmp(p_pnt->vsa_position.pid,pos->pid,RCP_TEMP_ID_LEN) == 0)
                            {
                                list_del(&pos->list);
                                rt_free((vsa_crd_node_t*)list_entry(&pos->list,vsa_crd_node_t,list));
                            }   
                            
                    }   
              
             rt_sem_release(p_vam->sem_sta);                  
                
            if ((p_vsa->alert_pend & (1<<warning_id))&&(list_empty(&p_vsa->crd_list))){
            /* inform system to stop alert */
                p_vsa->alert_pend &= ~(1<<warning_id);
                sys_add_event_queue(&p_cms_envar->sys, \
                                    SYS_MSG_STOP_ALERT, 0, warning_id, NULL);
            }
    }
    return 0;
}




static int ccw_proc(vsa_envar_t *p_vsa, void *arg)
{
    int err = 1;  /* '1' represent is not handled. */
    int vsa_id;
    sys_msg_t *p_msg = (sys_msg_t *)arg;	
	vsa_crd_node_t *p_crd = NULL,*pos = NULL;
    vsa_position_node_t *p_pnt = NULL,*pos_pnt;
	rt_bool_t  crd_flag,crd_rear_flag;  
    vam_envar_t *p_vam = &p_cms_envar->vam;
    if(list_empty(&p_vsa->position_list))
        return err;
    else
        list_for_each_entry(pos_pnt,vsa_position_node_t,&p_vsa->position_list,list){
            vsa_id = ccw_judge(pos_pnt);
            if(vsa_id)
                ccw_add_list(vsa_id,pos_pnt);
            else
                ccw_del_list(vsa_id,pos_pnt);                            

        }
    return err;
}

void timer_ebd_send_callback(void* parameter)
{
	vam_cancel_alert(1);
	OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Cancel Emergency braking \n\n");
	sys_add_event_queue(&p_cms_envar->sys,SYS_MSG_ALARM_CANCEL, 0, VSA_ID_EBD, NULL);
                                            
}

static int ebd_judge(vsa_position_node_t *p_node)
{

    int32_t dis_actual, dis_alert;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    /* put the beginning only in order to output debug infomations */
    dis_actual = p_node->vsa_position.linear_distance;
    dis_alert = p_node->vsa_position.safe_distance;

    if (p_node->vsa_position.local_speed <= p_vsa->working_param.danger_detect_speed_threshold){
        
        return 0;
    }

    if (p_node->vsa_position.remote_speed <= p_vsa->working_param.danger_detect_speed_threshold){
        
        return 0;
    } 

    if (p_node->vsa_position.flag_dir < 0){

        return 0;
    }
#if 0
    int32_t dis_actual;

    dis_actual = vam_get_peer_relative_pos(p_vsa->remote.pid,0);
	    /* remote is behind of local */
    if (dis_actual <= 0)
        return 0;

    if (p_vsa->local.speed <= p_vsa->working_param.danger_detect_speed_threshold){
        return 0;
    }

    if (vam_get_peer_relative_dir(p_vsa->position_node,p_vsa->remote) < 0){
        return 0;
    }
#endif
	OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO,\
	    "Emergency Vehicle  Alert !!! Id:%d%d%d%d\n",p_vsa->remote.pid[0],\
	    p_vsa->remote.pid[1],p_vsa->remote.pid[2],p_vsa->remote.pid[3]);
    return 1;

}
#if 0
static int ebd_proc(vsa_envar_t *p_vsa, void *arg)
{
    int err = 1;  /* '1' represent is not handled. */	
    uint16_t peer_alert;
    sys_msg_t *p_msg = (sys_msg_t *)arg;
	vam_get_peer_alert_status(&peer_alert);
	switch(p_msg->id){
		case VSA_GSNR_EBD_DETECT:
			if ((p_vsa->local.speed >= p_vsa->working_param.danger_detect_speed_threshold))
			{
				vam_active_alert(1);
				rt_timer_stop(p_vsa->timer_ebd_send);
				rt_timer_start(p_vsa->timer_ebd_send);
				OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Detect Emergency braking \n\n");
				sys_add_event_queue(&p_cms_envar->sys, \
                                            SYS_MSG_ALARM_ACTIVE, 0, VSA_ID_EBD, NULL);
				}
			break;
		case VSA_MSG_ALARM_UPDATE:
			if((peer_alert&VAM_ALERT_MASK_EBD)&&(ebd_judge(p_vsa)>0))
				{
					  /* danger is detected */    
                    if (p_vsa->alert_pend & (1<<VSA_ID_EBD)){
                    /* do nothing */    
                }
                    else{
                    /* inform system to start alert */
                        p_vsa->alert_pend |= (1<<VSA_ID_EBD);
                        sys_add_event_queue(&p_cms_envar->sys, \
                                            SYS_MSG_START_ALERT, 0, VSA_ID_EBD, NULL);
                    }
				}
				else if (p_vsa->alert_pend & (1<<VSA_ID_EBD)){
                    /* inform system to stop alert */
                        p_vsa->alert_pend &= ~(1<<VSA_ID_EBD);
                        sys_add_event_queue(&p_cms_envar->sys, \
                                            SYS_MSG_STOP_ALERT, 0, VSA_ID_EBD, NULL);
                    }
				break;
					
		default:
			break;
	}

    return err;
}
#endif
static int alarm_update_proc(vsa_envar_t *p_vsa, void *arg)
{
    int err = 1;  /* '1' represent is not handled. */	
    uint16_t peer_alert;
    sys_msg_t *p_msg = (sys_msg_t *)arg;  
    vsa_position_node_t *p_pnt = NULL;
	vam_get_peer_alert_status(&peer_alert);

    list_for_each_entry(p_pnt,vsa_position_node_t,&p_vsa->position_list,list){

        if(memcpy(p_pnt->vsa_position.pid,p_vsa->remote.pid,RCP_TEMP_ID_LEN))
            break;

    }
    

    if((peer_alert&VAM_ALERT_MASK_EBD)&&(ebd_judge(p_pnt)>0)){           
            if (p_vsa->alert_pend & (1<<VSA_ID_EBD)){        
            }
            else{
            /* inform system to start alert */
                p_vsa->alert_pend |= (1<<VSA_ID_EBD);
                sys_add_event_queue(&p_cms_envar->sys, \
                                    SYS_MSG_START_ALERT, 0, VSA_ID_EBD, NULL);
            }
    }
    else if (p_vsa->alert_pend & (1<<VSA_ID_EBD)){
            /* inform system to stop alert */
            p_vsa->alert_pend &= ~(1<<VSA_ID_EBD);
            sys_add_event_queue(&p_cms_envar->sys, \
                                    SYS_MSG_STOP_ALERT, 0, VSA_ID_EBD, NULL);
    }
    if((peer_alert&VAM_ALERT_MASK_VBD)&&(vbd_judge(p_pnt)>0)){
    		/* danger is detected */	
    	  if (p_vsa->alert_pend & (1<<VSA_ID_VBD)){
    	  /* do nothing */	  
    }
    	  else{
    	  /* inform system to start alert */
    		  p_vsa->alert_pend |= (1<<VSA_ID_VBD);
    		  sys_add_event_queue(&p_cms_envar->sys, \
    							  SYS_MSG_START_ALERT, 0, VSA_ID_VBD, NULL);
    	  }
    }
   else if (p_vsa->alert_pend & (1<<VSA_ID_VBD)){
    	  /* inform system to stop alert */
    		  p_vsa->alert_pend &= ~(1<<VSA_ID_VBD);
    		  sys_add_event_queue(&p_cms_envar->sys, \
    							  SYS_MSG_STOP_ALERT, 0, VSA_ID_VBD, NULL);
    	}    

    return err;
}

static int gsnr_detect_proc(vsa_envar_t *p_vsa, void *arg)
{   
    if ((p_vsa->local.speed >= p_vsa->working_param.danger_detect_speed_threshold)){
        vam_active_alert(1);
        rt_timer_stop(p_vsa->timer_ebd_send);
        rt_timer_start(p_vsa->timer_ebd_send);
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Detect Emergency braking \n\n");
        sys_add_event_queue(&p_cms_envar->sys, \
                                    SYS_MSG_ALARM_ACTIVE, 0, VSA_ID_EBD, NULL);
        }
    return 0;
}


static int key_update_proc(vsa_envar_t *p_vsa, void *arg)
{
	int err = 1;  /* '1' represent is not handled. */ 
	uint16_t peer_alert;
	static uint8_t keycnt = 0xff;
	sys_msg_t *p_msg = (sys_msg_t *)arg;
	vam_get_peer_alert_status(&peer_alert);

    if(p_msg->argc == C_UP_KEY){
            if(keycnt){
                vam_active_alert(0);
                sys_add_event_queue(&p_cms_envar->sys, \
                              SYS_MSG_ALARM_ACTIVE, 0, VSA_ID_VBD, NULL);
                OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Active Vihicle Break Down Alert\n");
            }   
            else {   
                vam_cancel_alert(0);
                sys_add_event_queue(&p_cms_envar->sys, \
                              SYS_MSG_ALARM_CANCEL, 0, VSA_ID_VBD, NULL);
                OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Cancel Vihicle Break Down Alert\n");
            }
            keycnt = ~keycnt;
    }

    return err;
}
static int vbd_judge(vsa_position_node_t *p_node)
{
    int32_t dis_actual, dis_alert;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    /* put the beginning only in order to output debug infomations */
    dis_actual = p_node->vsa_position.linear_distance;
    dis_alert = p_node->vsa_position.safe_distance;

    if (p_node->vsa_position.local_speed <= p_vsa->working_param.danger_detect_speed_threshold){
        
        return 0;
    }

    if (p_node->vsa_position.remote_speed <= p_vsa->working_param.danger_detect_speed_threshold){
        
        return 0;
    } 

    if (p_node->vsa_position.flag_dir < 0){

        return 0;
    }


#if 0
    int32_t dis_actual;

    dis_actual = vam_get_peer_relative_pos(p_vsa->remote.pid,0);// relative position

    if (p_vsa->local.speed <= p_vsa->working_param.danger_detect_speed_threshold){
        return 0;
    }//<=30 Km/h

    if (vam_get_peer_relative_dir(p_vsa->remote.pid) < 0){
        return 0;
    }//driving direction

    /* remote is behind of local */
    if (dis_actual <= 0){
        return 0;
    }
#endif
	OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Vehicle Breakdown Alert!!! Id:%d%d%d%d\n",\
	    p_vsa->remote.pid[0],p_vsa->remote.pid[1],p_vsa->remote.pid[2],p_vsa->remote.pid[3]);
    return 1;

}

#if 0
static int vbd_proc(vsa_envar_t *p_vsa, void *arg)
{
	int err = 1;  /* '1' represent is not handled. */ 
	uint16_t peer_alert;
	static uint8_t keycnt = 0xff;
	sys_msg_t *p_msg = (sys_msg_t *)arg;
	vam_get_peer_alert_status(&peer_alert);
	switch(p_msg->id){
		  case VSA_MSG_KEY_UPDATE:
				if(p_msg->argc == C_UP_KEY)
					{
						if(keycnt)
							{
								vam_active_alert(0);
								sys_add_event_queue(&p_cms_envar->sys, \
											  SYS_MSG_ALARM_ACTIVE, 0, VSA_ID_VBD, NULL);
								OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Active Vihicle Break Down Alert\n");
							}	
						else 
							{	
								vam_cancel_alert(0);
								sys_add_event_queue(&p_cms_envar->sys, \
											  SYS_MSG_ALARM_CANCEL, 0, VSA_ID_VBD, NULL);
								OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "Cancel Vihicle Break Down Alert\n");
							}
						keycnt = ~keycnt;
					}
			   break;
		
		  case VSA_MSG_ALARM_UPDATE:
			  if((peer_alert&VAM_ALERT_MASK_VBD)&&(vbd_judge(p_vsa)>0))
				  {
						/* danger is detected */	
					  if (p_vsa->alert_pend & (1<<VSA_ID_VBD)){
					  /* do nothing */	  
				  }
					  else{
					  /* inform system to start alert */
						  p_vsa->alert_pend |= (1<<VSA_ID_VBD);
						  sys_add_event_queue(&p_cms_envar->sys, \
											  SYS_MSG_START_ALERT, 0, VSA_ID_VBD, NULL);
					  }
				  }
				  else if (p_vsa->alert_pend & (1<<VSA_ID_VBD)){
					  /* inform system to stop alert */
						  p_vsa->alert_pend &= ~(1<<VSA_ID_VBD);
						  sys_add_event_queue(&p_cms_envar->sys, \
											  SYS_MSG_STOP_ALERT, 0, VSA_ID_VBD, NULL);
					  }
				break;  
		  default:
			  break;
	  }
	
	  return err;

}
#endif
static void vsa_default_proc(vsa_envar_t *p_vsa, void *arg)
{
    sys_msg_t *p_msg = (sys_msg_t *)arg;
    
    switch(p_msg->id){
        case VSA_MSG_GPS_UPDATE:
            p_vsa->gps_status = p_msg->argc;
            sys_add_event_queue(&p_cms_envar->sys, \
                            SYS_MSG_GPS_UPDATE, 0, p_msg->argc, NULL);
        break;

        default:
        break;
    }
}

vsa_app_handler vsa_app_handler_tbl[] = {

    ccw_proc,
    alarm_update_proc,
    key_update_proc,
    gsnr_detect_proc,
    NULL
};

void rt_vsa_thread_entry(void *parameter)
{
    rt_err_t err;
    sys_msg_t msg, *p_msg = &msg;
    vsa_envar_t *p_vsa = (vsa_envar_t *)parameter;
    vsa_app_handler *handler;

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s: ---->\n", __FUNCTION__);

	while(1){
        err = rt_mq_recv(p_vsa->queue_vsa, p_msg, sizeof(sys_msg_t), RT_WAITING_FOREVER);
        if (err == RT_EOK){
            err = vsa_app_handler_tbl[p_msg->id-0x0301](p_vsa,p_msg);
            /*
            for(handler = &vsa_app_handler_tbl[0];*handler != NULL;handler++){
                err = (*handler)(p_vsa, p_msg);
                */
            if (err == 1){
                vsa_default_proc(p_vsa, p_msg);
            }
       }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s: rt_mq_recv error [%d]\n", __FUNCTION__, err);
        }
	}
}

rt_err_t vsa_add_event_queue(vsa_envar_t *p_vsa, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv)
{
    int err = OSAL_STATUS_NOMEM;
    sys_msg_t *p_msg;

    p_msg = osal_malloc(sizeof(sys_msg_t));
    if (p_msg){
        p_msg->id = msg_id;
        p_msg->len = msg_len;
        p_msg->argc = msg_argc;
        p_msg->argv = msg_argv;
        err = osal_queue_send(p_vsa->queue_vsa,p_msg);
    }

    if (err != OSAL_STATUS_SUCCESS){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x\n",\
                                   __FUNCTION__, err, msg_id);
        osal_free(p_msg);
    }

    return err;
}


/*****************************************************************************
 @funcname: vsa_init
 @brief   : vsa module initial
 @param   : None
 @return  : 
*****************************************************************************/
void vsa_init()
{
    int i;

    vsa_envar_t *p_vsa = &p_cms_envar->vsa;

    memset(p_vsa, 0, sizeof(vsa_envar_t));
    memcpy(&p_vsa->working_param, &p_cms_param->vsa, sizeof(vsa_config_t));

	
	INIT_LIST_HEAD(&p_vsa->crd_list);	
	INIT_LIST_HEAD(&p_vsa->position_list);	


    for(i = 0;i< VAM_NEIGHBOUR_MAXNUM;i++){
        list_add_tail(&p_vsa->position_node[i].list, &p_vsa->position_list);
    }
	//INIT_LIST_HEAD(&p_vsa->vbd_list);

    p_vsa->alert_mask = (1<<VSA_ID_CRD)|(1<<VSA_ID_CRD_REAR)|(1<<VSA_ID_VBD)|(1<<VSA_ID_EBD);

    /* os object for vsa */
    p_vsa->queue_vsa = osal_queue_create("q-vsa",  VSA_QUEUE_SIZE);
    osal_assert(p_vsa->queue_vsa != NULL);
    
	p_vsa->task_vsa = osal_task_create("t-vsa",
                           rt_vsa_thread_entry, p_vsa,
                           RT_VSA_THREAD_STACK_SIZE, RT_VSA_THREAD_PRIORITY);
    
    osal_assert(p_vsa->task_vsa != NULL);

    p_vsa->timer_ebd_send = osal_timer_create("tm-ebd",timer_ebd_send_callback,NULL,\
        VSA_EBD_SEND_PERIOD,RT_TIMER_FLAG_ONE_SHOT); 					
    osal_assert(p_vsa->timer_ebd_send != NULL);

    p_vsa->timer_position_prepro = osal_timer_create("tm-pos",\
        timer_preprocess_pos_callback,NULL,VSA_POS_PERIOD,RT_TIMER_FLAG_PERIODIC);
    osal_assert(p_vsa->timer_position_prepro != NULL);

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "vsa module initial\n");
                             
}

/*****************************************************************************
 @funcname: vsa_deinit
 @brief   : vsa module unstall
 @param   : None
 @return  : 
*****************************************************************************/
void vsa_deinit()
{
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "vsa module initial\n");
}
