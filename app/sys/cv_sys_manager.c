/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_sys_manager.c
 @brief  : this file include the system manage functions
 @author : wangyifeng
 @history:
           2014-6-20    wangyifeng    Created file
           ...
******************************************************************************/
//#pragma O3


#include "cv_osal.h"

#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_TRACE
#define MODULE_NAME "sysc"
#include "cv_osal_dbg.h"
OSAL_DEBUG_ENTRY_DEFINE(sysc)

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"

#include "led.h"
#include "key.h"
#include "cv_vsa.h"


#define HUMAN_ITERFACE_DEFAULT         SECOND_TO_TICK(1)

#define HUMAN_ITERFACE_VOC         	   SECOND_TO_TICK(3)

#define HUMAN_ITERFACE_GPS_VOC         SECOND_TO_TICK(5)

#define BREATH_CYCLE                   300 


#define AUDIO_START_ADDRESS     58 /* Offset relative to audio file header size */

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
extern const unsigned char CFCW_8K_16bits[];
extern const unsigned int CFCW_8K_16bitsLen;
extern const unsigned char CRCW_8K_16bits[];
extern const unsigned int CRCW_8K_16bitsLen;
extern const unsigned char EEBL_8K_16bits[];
extern const unsigned int EEBL_8K_16bitsLen;
extern const unsigned char VBD_8K_16bits[];
extern const unsigned int VBD_8K_16bitsLen;
extern const unsigned char init_8K_16bits[];
extern const unsigned int init_8K_16bitsLen;
extern const unsigned char test_8K_16bits[];
extern const unsigned int test_8K_16bitsLen;


extern void led_on(Led_TypeDef led);
extern void led_off(Led_TypeDef led);
extern void led_blink(Led_TypeDef led);

extern int param_set(uint8_t param, int32_t value);

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/


int sys_add_event_queue(sys_envar_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv)
{
    int err = OSAL_STATUS_NOMEM;
    sys_msg_t *p_msg;

    p_msg = osal_malloc(sizeof(sys_msg_t));
    if (p_msg) {
        p_msg->id = msg_id;
        p_msg->len = msg_len;
        p_msg->argc = msg_argc;
        p_msg->argv = msg_argv;
        err = osal_queue_send(p_sys->queue_sys_mng, p_msg);
    }

    if (err != OSAL_STATUS_SUCCESS) {
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x\n",\
                           __FUNCTION__, err, msg_id);
        osal_free(p_msg);                   
    }

    return err;
}

osal_status_t hi_add_event_queue(sys_envar_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv)
{
    osal_status_t err = OSAL_STATUS_NOMEM;
    sys_msg_t *hi_msg;
    hi_msg = osal_malloc(sizeof(sys_msg_t));
    if (hi_msg) {
        hi_msg->id = msg_id;
        hi_msg->len = msg_len;
        hi_msg->argc = msg_argc;
        hi_msg->argv = msg_argv;
        err = osal_queue_send(p_sys->queue_sys_hi, hi_msg);
    }

    if (err != OSAL_STATUS_SUCCESS) {
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x\n",\
                           __FUNCTION__, err, msg_id);
        osal_free(hi_msg);                   
    }

    return err;
}

osal_status_t voc_add_event_queue(vsa_envar_t *p_vsa, 
                             uint32_t msg_addr, 
                             uint32_t msg_size, 
                             uint8_t  msg_channel,
                             uint8_t  msg_cmd)
{
    int err = OSAL_STATUS_NOMEM;
    adpcm_t *p_msg;
    p_msg = osal_malloc(sizeof(adpcm_t));
    if (p_msg){
        p_msg->addr = msg_addr;
        p_msg->size = msg_size;
        p_msg->channel = msg_channel;
        p_msg->cmd = msg_cmd;
        err = osal_queue_send(p_vsa->queue_voc,p_msg);
    }

    if (err != OSAL_STATUS_SUCCESS){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x\n",\
                                   __FUNCTION__, err, msg_cmd);
        osal_free(p_msg);
    }
    return err;
}







osal_status_t led_add_event_queue(sys_envar_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv)
{

    osal_status_t err = OSAL_STATUS_NOMEM;
    sys_msg_t *p_msg;
    p_msg = osal_malloc(sizeof(sys_msg_t));
    if (p_msg) {
        p_msg->id = msg_id;
        p_msg->len = msg_len;
        p_msg->argc = msg_argc;
        p_msg->argv = msg_argv;
        err = osal_queue_send(p_sys->queue_hi_led, p_msg);
    }

    if (err != OSAL_STATUS_SUCCESS) {
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x\n",\
                           __FUNCTION__, err, msg_id);
        osal_free(p_msg);                   
    }
    return err;
}


void voc_contrl(uint32_t cmd, uint8_t *p_data, uint32_t length)
{
	vsa_envar_t *p_vsa = &p_cms_envar->vsa;

    p_vsa->adpcm_data.addr = (uint32_t)p_data;
    p_vsa->adpcm_data.size = length;
    p_vsa->adpcm_data.channel = 0;
    p_vsa->adpcm_data.cmd = cmd;
    voc_add_event_queue(p_vsa,p_vsa->adpcm_data.addr,p_vsa->adpcm_data.size,0,cmd);
}

void sys_manage_proc(sys_envar_t *p_sys, sys_msg_t *p_msg)
{
	uint32_t type = 0; 
	static uint8_t keycnt = 0xff;
	vsa_envar_t *p_vsa = &p_cms_envar->vsa;
	
    switch(p_msg->id){
        case SYS_MSG_INITED:
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_TRACE, "%s: initialize complete\n", __FUNCTION__);

            vam_start();
            vsa_start();
            hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,HI_OUT_SYS_INIT, 0);
            break;
		case SYS_MSG_BSM_UPDATE:
			hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,p_msg->argc, 0);			
			break;
			
		case SYS_MSG_KEY_PRESSED:
			if(p_msg->argc == C_UP_KEY){             
			        //vsa_add_event_queue(p_vsa, VSA_MSG_MANUAL_BC, 0,keycnt,NULL);
			        //keycnt = ~keycnt;
                    voc_contrl(VOC_PLAY, (uint8_t *)test_8K_16bits, test_8K_16bitsLen);
             }
			else if(p_msg->argc == C_DOWN_KEY)
				{
					rt_kprintf("gsnr param is resetting .....\n");
					param_set(19,0);                  
			}
			break;
        case SYS_MSG_START_ALERT:
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_TRACE, "%s:alert start!!!.\n", __FUNCTION__);
            //rt_mq_send(p_sys->queue_sys_hi, p_msg, sizeof(sys_msg_t));
            {

                if (p_msg->argc == VSA_ID_CRD){
                    type = HI_OUT_CRD_ALERT;
                }
                else if (p_msg->argc == VSA_ID_CRD_REAR){
                    type = HI_OUT_CRD_REAR_ALERT;
                }
                else if (p_msg->argc == VSA_ID_VBD){
                    type = HI_OUT_VBD_ALERT;
                }
                else if (p_msg->argc == VSA_ID_EBD){
                    type = HI_OUT_EBD_ALERT;
                }
                
                hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,type, 0);
            }

            break;
            
        case SYS_MSG_STOP_ALERT:
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_TRACE, "%s:alert stop.\n", __FUNCTION__);
			
				//uint32_t type = 0;
			
				if (p_msg->argc == VSA_ID_CRD){
					type = HI_OUT_CRD_CANCEL;
				}
				if (p_msg->argc == VSA_ID_CRD_REAR){
					type = HI_OUT_CRD_REAR_CANCEL;
				}
				else if (p_msg->argc == VSA_ID_VBD){
					type = HI_OUT_VBD_CANCEL;
				}
				else if (p_msg->argc == VSA_ID_EBD){
					type = HI_OUT_EBD_CANCEL;
				}
				//don't distinguish  message of  alert canceling   for the time being
            	hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,type, 0);
            break;
			
		case SYS_MSG_ALARM_ACTIVE:
			
			 	if (p_msg->argc == VSA_ID_VBD)
					type = HI_OUT_VBD_STATUS;
				
				else if (p_msg->argc == VSA_ID_EBD)
					type = HI_OUT_EBD_STATUS;
				
			    hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,type, 0);				
				break;
				
		case SYS_MSG_ALARM_CANCEL:
				if (p_msg->argc == VSA_ID_VBD)
					type = HI_OUT_VBD_STOP;
				
				else if (p_msg->argc == VSA_ID_EBD)
					type = HI_OUT_EBD_STOP;
			 	hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,type, 0);
				break;		

        case SYS_MSG_GPS_UPDATE:
            hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,\
                ((p_msg->argc == 0)? HI_OUT_GPS_LOST:HI_OUT_GPS_CAPTURED) , 0);
            break;
		
        default:
            break;
    }
}

void sysc_thread_entry(void *parameter)
{
    int err;
    sys_msg_t *p_msg;
    sys_envar_t *p_sys = (sys_envar_t *)parameter;

	while(1){
        err = osal_queue_recv(p_sys->queue_sys_mng, &p_msg, OSAL_WAITING_FOREVER);
        if (err == OSAL_STATUS_SUCCESS){
            sys_manage_proc(p_sys, p_msg);
            osal_free(p_msg);
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: osal_queue_recv error [%d]\n", __FUNCTION__, err);
        }
	}
}


void timer_human_interface_callback(void* parameter)
{
    sys_envar_t *p_sys = (sys_envar_t *)parameter;
    

    p_sys->voc_flag = 0;
}

void timer_out_vsa_process(void* parameter)
{
	int  timevalue;
	vsa_envar_t* p_vsa  = (vsa_envar_t*)parameter;
	timevalue = HUMAN_ITERFACE_GPS_VOC;
	
#if 0
	if(p_vsa->alert_pend & (1<<VSA_ID_EBD))	
		voc_contrl(VOC_PLAY, (uint8_t *)EEBL_8K_16bits, EEBL_8K_16bitsLen);// vioce,EBD最优先,同时报警选择EBD,VBD次之

	else if(p_vsa->alert_pend & (1<<VSA_ID_VBD))
		voc_contrl(VOC_PLAY, (uint8_t *)VBD_8K_16bits, VBD_8K_16bitsLen);// 

	else if(p_vsa->alert_pend & (1<<VSA_ID_CRD))	
		voc_contrl(VOC_PLAY, (uint8_t *)CFCW_8K_16bits, CFCW_8K_16bitsLen);// 
	else if(p_vsa->alert_pend & (1<<VSA_ID_CRD_REAR))	
		voc_contrl(VOC_PLAY, (uint8_t *)CRCW_8K_16bits, CRCW_8K_16bitsLen);// 
#endif
    voc_contrl(VOC_PLAY, (uint8_t *)init_8K_16bits, init_8K_16bitsLen);
	rt_timer_control(p_cms_envar->sys.timer_voc,RT_TIMER_CTRL_SET_TIME,(void*)&timevalue);
}


void sys_human_interface_proc(sys_envar_t *p_sys, sys_msg_t *p_msg)
{
    if (p_msg->id == SYS_MSG_HI_OUT_UPDATE){
        switch(p_msg->argc){
			case HI_OUT_SYS_INIT:
				p_sys->led_priority |= 1<<HI_OUT_GPS_LOST;
				voc_contrl(VOC_PLAY, (uint8_t *)init_8K_16bits, init_8K_16bitsLen/6);
				break;

			case HI_OUT_BSM_UPDATE:
					p_sys->led_priority |= 1<<SYS_MSG_BSM_UPDATE;
				break;
			case HI_OUT_BSM_NONE:
					p_sys->led_priority &= ~(1<<SYS_MSG_BSM_UPDATE);
				break;				
				
            case HI_OUT_CRD_ALERT:
                voc_contrl(VOC_PLAY, (uint8_t *)CFCW_8K_16bits, CFCW_8K_16bitsLen);               
                rt_timer_start(p_cms_envar->sys.timer_hi);
                #if 0
				if(!p_sys->voc_flag)
					{
                		voc_contrl(VOC_PLAY, (uint8_t *)CFCW_8K_16bits, CFCW_8K_16bitsLen);
						p_sys->voc_flag = 0xff;
					}
                rt_timer_start(p_cms_envar->sys.timer_hi);
                #endif
				if(p_sys->led_priority&(1<<HI_OUT_CRD_ALERT))
					return;
				else
					{
             			p_sys->led_priority |= 1<<HI_OUT_CRD_ALERT;
					}
				break;

			case HI_OUT_CRD_REAR_ALERT:
                
                voc_contrl(VOC_PLAY, (uint8_t *)CRCW_8K_16bits, CRCW_8K_16bitsLen);
                rt_timer_start(p_cms_envar->sys.timer_hi);
                #if 0
                if(!p_sys->voc_flag)
					{
                		voc_contrl(VOC_PLAY, (uint8_t *)CRCW_8K_16bits, CRCW_8K_16bitsLen);
						p_sys->voc_flag = 0xff;
					}
                rt_timer_start(p_cms_envar->sys.timer_hi);
                #endif
				if(p_sys->led_priority&(1<<HI_OUT_CRD_REAR_ALERT))
					return;
				else
					{
             			p_sys->led_priority |= 1<<HI_OUT_CRD_REAR_ALERT;
					}	
				break;
				
            case HI_OUT_VBD_ALERT:
                
                voc_contrl(VOC_PLAY, (uint8_t *)VBD_8K_16bits, VBD_8K_16bitsLen);                
                rt_timer_start(p_cms_envar->sys.timer_voc);
				if(p_sys->led_priority&(1<<HI_OUT_VBD_ALERT))
					return;
				else
					{
			   			p_sys->led_priority |= 1<<HI_OUT_VBD_ALERT;
						OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_INFO,"HI vbd alert!!\n\n");
					}	
                break;				

			case HI_OUT_EBD_ALERT:
                voc_contrl(VOC_PLAY, (uint8_t *)EEBL_8K_16bits, EEBL_8K_16bitsLen);
              	rt_timer_start(p_cms_envar->sys.timer_voc);            
			   	p_sys->led_priority |= 1<<HI_OUT_EBD_ALERT;
                break;

			case HI_OUT_CRD_CANCEL:
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_CRD_ALERT);

				break;

			case HI_OUT_CRD_REAR_CANCEL:
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_CRD_REAR_ALERT);

				break;	

			case HI_OUT_VBD_CANCEL:
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_VBD_ALERT);
                OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_INFO,"HI vbd alert  cancel!!\n\n");
				break;

			case HI_OUT_EBD_CANCEL://cancel alarm
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_EBD_ALERT);
                OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_INFO,"HI ebd alert  cancel!!\n\n");
				break;
				
			case HI_OUT_VBD_STATUS:
				p_sys->led_priority |= 1<<HI_OUT_VBD_STATUS;
				break;
			case HI_OUT_EBD_STATUS:
				p_sys->led_priority |= 1<<HI_OUT_EBD_STATUS;
				break;

			case HI_OUT_VBD_STOP:
				p_sys->led_priority &= ~(1<<HI_OUT_VBD_STATUS);//stop broadcast
				break;
			case HI_OUT_EBD_STOP:
				p_sys->led_priority &= ~(1<<HI_OUT_EBD_STATUS);//stop broadcast
				break;	

				
            case HI_OUT_CANCEL_ALERT:
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
			#if 0	
                p_sys->led_blink_duration[LED_RED] = 0;
                p_sys->led_blink_period[LED_RED] = 0;
                p_sys->led_blink_cnt[LED_RED] = 0;

                /* recover gps led */
                if (p_cms_envar->vsa.gps_status){
                    p_sys->led_blink_period[LED_GREEN] = 0xFFFF; 
                }
                else{
                    p_sys->led_blink_period[LED_GREEN] = 20;
                }
			#endif	
				p_sys->led_priority &= ~((1<<HI_OUT_EBD_ALERT)|(1<<HI_OUT_VBD_ALERT)|(1<<HI_OUT_CRD_ALERT));
                break;

            case HI_OUT_GPS_LOST:
                //voc_contrl(16000, (uint8_t *)notice_16k_8bits, 6400);
               	//rt_timer_start(p_cms_envar->sys.timer_gps);
                //p_sys->led_blink_duration[LED_GREEN] = 0xFFFF;
                //p_sys->led_blink_period[LED_GREEN] = 20;
                //p_sys->led_blink_cnt[LED_GREEN] = 0;
                p_sys->led_priority |= 1<<HI_OUT_GPS_LOST;			
                
                break;

            case HI_OUT_GPS_CAPTURED:
				//rt_timer_stop(p_cms_envar->sys.timer_gps);
                //p_sys->led_blink_duration[LED_GREEN] = 0xFFFF;
                //p_sys->led_blink_period[LED_GREEN] = 0xFFFF;
                //p_sys->led_blink_cnt[LED_GREEN] = 0;
                p_sys->led_priority &= ~(1<<HI_OUT_GPS_LOST);
                break;

            default:
                break;
        }
    }
    else if (p_msg->id == SYS_MSG_HI_IN_UPDATE){
        
        switch(p_msg->argc){
            case HI_IN_KEY_PRESSED:
                sys_add_event_queue(p_sys,SYS_MSG_KEY_PRESSED, 0, p_msg->len, NULL);
                break;
            default:
                break;
            }

    }
#ifdef HARDWARE_MODULE_WIFI_V1
	if((p_sys->led_priority&(1<<HI_OUT_CRD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_CRD_REAR_ALERT))\
		||(p_sys->led_priority&(1<<HI_OUT_EBD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_VBD_ALERT)))
		{
			p_sys->led_color = LED_RED;//r=1,b=0,g=0
		    p_sys->led_blink_duration = 0xFFFF;
            p_sys->led_blink_period = 15;
            p_sys->led_blink_cnt = 0;
			led_add_event_queue(p_sys,LED_RED,0,LED_BLINK,NULL);
	}
	else if((p_sys->led_priority&(1<<HI_OUT_EBD_STATUS))||(p_sys->led_priority&(1<<HI_OUT_VBD_STATUS)))
		{
			p_sys->led_color = LED_GREEN;//r=1,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
            p_sys->led_blink_period = 15;
            p_sys->led_blink_cnt = 0;
			led_add_event_queue(p_sys,LED_RED,0,LED_ON,NULL);
	}
	else {
		    led_add_event_queue(p_sys,LED_RED,0,LED_OFF,NULL);

	}

	 if(p_sys->led_priority&(1<<HI_OUT_GPS_LOST))
		{
			p_sys->led_color = LED_BLUE;//r=1,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
            p_sys->led_blink_period = 25;
            p_sys->led_blink_cnt = 0;
			led_add_event_queue(p_sys,LED_GREEN,0,LED_OFF,NULL);
		}
	 else{
		    led_add_event_queue(p_sys,LED_GREEN,0,LED_ON,NULL);

	 }
	 if(p_sys->led_priority&(1<<SYS_MSG_BSM_UPDATE))
		{
            p_sys->led_color = LED_BLUE;//r=1,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
			p_sys->led_blink_period = 25;
			p_sys->led_blink_cnt = 0;
			led_add_event_queue(p_sys,LED_BLUE,0,LED_BLINK,NULL);

	}
	else {
			p_sys->led_color = LED_BLUE;//r=0,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
			p_sys->led_blink_period = 0xFFFF;
			p_sys->led_blink_cnt = 0;
			led_add_event_queue(p_sys,LED_BLUE,0,LED_OFF,NULL);
	}
 #elif defined(HARDWARE_MODULE_WIFI_V2)
 
 if((p_sys->led_priority&(1<<HI_OUT_CRD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_CRD_REAR_ALERT))\
     ||(p_sys->led_priority&(1<<HI_OUT_EBD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_VBD_ALERT)))
     {
         p_sys->led_color = RED_STATE;//
         led_add_event_queue(p_sys,RED_STATE,0,LED_BLINK,NULL);
 }
 else if((p_sys->led_priority&(1<<HI_OUT_EBD_STATUS))||(p_sys->led_priority&(1<<HI_OUT_VBD_STATUS)))
     {
         p_sys->led_color = YELLOW_STATE;//
         led_add_event_queue(p_sys,YELLOW_STATE,0,LED_ON,NULL);
 }

 else if(p_sys->led_priority&(1<<HI_OUT_GPS_LOST))
     {
         p_sys->led_color = GREEN_STATE;//
         led_add_event_queue(p_sys,GREEN_STATE,0,LED_BLINK,NULL);
     }
  else if(p_sys->led_priority&(1<<SYS_MSG_BSM_UPDATE))
     {
         p_sys->led_color = BLUE_STATE;//r=1,b=0,g=1
         led_add_event_queue(p_sys,BLUE_STATE,0,LED_BLINK,NULL);
 
 }
 else if(p_sys->led_priority&(~(1<<HI_OUT_GPS_LOST))){
         p_sys->led_color = GREEN_STATE;//
         led_add_event_queue(p_sys,GREEN_STATE,0,LED_ON,NULL);
 }
 else {
     p_sys->led_color = LIGHT_STATE;//
     led_add_event_queue(p_sys,LIGHT_STATE,0,LED_OFF,NULL);
 }
 #endif
}

void rt_hi_thread_entry(void *parameter)
{
    osal_status_t err;
    sys_msg_t msg, *p_msg = &msg;
    sys_envar_t *p_sys = (sys_envar_t *)parameter;
	//static uint8_t ledss = 0xff;

    rt_timer_start(p_sys->timer_hi);

    while(1){
        err = osal_queue_recv(p_sys->queue_sys_hi, &p_msg, OSAL_WAITING_FOREVER);
        if (err == OSAL_STATUS_SUCCESS){
            sys_human_interface_proc(p_sys, p_msg);
            osal_free(p_msg);
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: osal_queue_recv error [%d]\n", __FUNCTION__, err);           
            osal_free(p_msg);
        }

#if 0
	if((p_sys->led_priority&(1<<HI_OUT_CRD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_CRD_REAR_ALERT))\
		||(p_sys->led_priority&(1<<HI_OUT_EBD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_VBD_ALERT)))
		{
			if (++RED_blink_cnt >= RED_blink_period){
				led_blink(LED_RED);
				RED_blink_cnt = 0;
				}
	}
	else if((p_sys->led_priority&(1<<HI_OUT_EBD_STATUS))||(p_sys->led_priority&(1<<HI_OUT_VBD_STATUS))){
			led_on(LED_RED);
	}
	else{
        
			led_off(LED_RED);
	}

	if(p_sys->led_priority&(1<<HI_OUT_GPS_LOST))
		{
			if (++GREEN_blink_cnt >= GREEN_blink_period){
				led_blink(LED_GREEN);
				GREEN_blink_cnt = 0;
				}
		}
	else {
			led_off(LED_GREEN);

	}
	
	if(p_sys->led_priority&(1<<SYS_MSG_BSM_UPDATE))
		{
			//p_sys->led_priority &= ~(1<<HI_OUT_SYS_BSM);
			if (++BLUE_blink_cnt >= BLUE_blink_period){
				led_blink(LED_BLUE);
				BLUE_blink_cnt = 0;
				}

	}
	else{
			led_off(LED_BLUE);
	}
    
 #endif
	}
}
void sys_init(void)
{
    sys_envar_t *p_sys = &p_cms_envar->sys;
	
	vsa_envar_t *p_vsa = &p_cms_envar->vsa;

    /* object for sys */
    p_sys->queue_sys_mng = osal_queue_create("q-sys", SYS_QUEUE_SIZE);
    osal_assert(p_sys->queue_sys_mng != NULL);

    p_sys->task_sys_mng = osal_task_create("t-sys",
                           sysc_thread_entry, p_sys,
                           RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);
    osal_assert(p_sys->task_sys_mng != NULL);

    /* object for human interface */
    p_sys->queue_sys_hi = osal_queue_create("q-hi", SYS_QUEUE_SIZE);
    osal_assert(p_sys->queue_sys_mng != NULL);


    p_sys->timer_hi = rt_timer_create("tm-hi",timer_human_interface_callback,p_sys,\
        HUMAN_ITERFACE_VOC,RT_TIMER_FLAG_ONE_SHOT); 					
    RT_ASSERT(p_sys->timer_hi != RT_NULL);

    p_sys->timer_voc= rt_timer_create("tm-voc",timer_out_vsa_process,p_vsa,\
        HUMAN_ITERFACE_GPS_VOC,RT_TIMER_FLAG_PERIODIC); 					
    RT_ASSERT(p_sys->timer_hi != RT_NULL);


    p_sys->task_sys_hi = osal_task_create("t-hi",
                           rt_hi_thread_entry, p_sys,
                           RT_HI_THREAD_STACK_SIZE, RT_HI_THREAD_PRIORITY);
    osal_assert(p_sys->task_sys_hi != NULL);

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial\n");
}


