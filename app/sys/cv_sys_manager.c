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
extern const unsigned char bibi_front_16k_8bits[];
extern const unsigned char bibi_behind_16k_8bits[];
extern const unsigned int bibi_front_16k_8bitsLen;
extern const unsigned int bibi_behind_16k_8bitsLen;
extern const uint16_t AUDIO_SAMPLE[];
extern void voc_play(uint32_t sample_rate, uint8_t *p_data, uint32_t length);
extern void led_on(Led_TypeDef led);
extern void led_off(Led_TypeDef led);
extern void led_blink(Led_TypeDef led);

extern int param_set(uint8_t param, int32_t value);
extern uint32_t Pt8211_AUDIO_Play(uint16_t * pBuffer,uint32_t Size);
extern const unsigned short AUDIO_SAMPLE[];
extern void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor);


void voc_play(uint32_t sample_rate, uint8_t *p_data, uint32_t length)
{

}

/* Flash error code */
typedef enum {
    FLASH_NO_ERR,
    FLASH_ERASE_ERR,
    FLASH_WRITE_ERR,
    FLASH_ENV_NAME_ERR,
    FLASH_ENV_NAME_EXIST,
    FLASH_ENV_FULL,
} FlashErrCode;

uint8_t flash_set_env(const char *key, const char *value) {
    FlashErrCode result = FLASH_NO_ERR;
    uint32_t readbuffer;

    /* if value is null, delete it */
    if (*value == NULL) {
        readbuffer = *value;
        osal_printf("not crap!!!\n\n  value = %lu",readbuffer);
    }
    return result;
}

FINSH_FUNCTION_EXPORT(flash_set_env, debug: );

//extern void Delay(volatile uint32_t nCount);
/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
void Delay_led(volatile uint32_t nCount)
	{
	  volatile uint32_t index = 0; 
	  for(index = (10 * nCount); index != 0; index--)
	  {
	  }
	}


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


rt_err_t hi_add_event_queue(sys_envar_t *p_sys, 
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
    err = rt_mq_send(p_sys->queue_sys_hi, &msg, sizeof(sys_msg_t));

    if (err != RT_EOK){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: failed=[%d], msg=%04x\n", __FUNCTION__, err, msg_id);
    }

    return err;
}


void sys_manage_proc(sys_envar_t *p_sys, sys_msg_t *p_msg)
{
	uint32_t type = 0;
	vsa_envar_t *p_vsa = &p_cms_envar->vsa;

    char c_value = NULL;
    char *value = &c_value;
	
    switch(p_msg->id){
        case SYS_MSG_INITED:
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_TRACE, "%s: initialize complete\n", __FUNCTION__);

            vam_start();
            vsa_start();

            //hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,HI_OUT_SYS_INIT, 0);

            break;
		case SYS_MSG_BSM_UPDATE:
			hi_add_event_queue(p_sys, SYS_MSG_HI_OUT_UPDATE,0,p_msg->argc, 0);			
			break;
			
		case SYS_MSG_KEY_PRESSED:
			if(p_msg->argc == C_UP_KEY){
                
			    vsa_add_event_queue(p_vsa, VSA_MSG_KEY_UPDATE, 0,p_msg->argc,NULL);
                p_vsa->adpcm_data.addr = (uint32_t)AUDIO_SAMPLE;
                p_vsa->adpcm_data.size = bibi_front_16k_8bitsLen;
                p_vsa->adpcm_data.channel = 0;
                p_vsa->adpcm_data.cmd = VOC_PLAY;
              // rt_mb_send(p_vsa->mb_sound,(uint32_t)&(p_vsa->adpcm_data));
              //adpcm_play((char*)AUDIO_SAMPLE, bibi_front_16k_8bitsLen);
               // voc_add_event_queue(p_vsa,p_vsa->adpcm_data.addr,p_vsa->adpcm_data.size,0,VOC_PLAY);
               flash_set_env("abc",value);
             }
			else if(p_msg->argc == C_DOWN_KEY)
				{
					rt_kprintf("gsnr param is resetting .....\n");
					param_set(19,0);
                    
                   // Pt8211_AUDIO_Play((uint16_t*)(AUDIO_SAMPLE), bibi_front_16k_8bitsLen);
					//rt_kprintf("restart......\n\n");
					//NVIC_SystemReset();
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
	timevalue = HUMAN_ITERFACE_VOC;
	

	if(p_vsa->alert_pend & (1<<VSA_ID_EBD))	
		voc_play(16000, (uint8_t *)bibi_front_16k_8bits, bibi_front_16k_8bitsLen);// vioce,EBD最优先,同时报警选择EBD,VBD次之

	else if(p_vsa->alert_pend & (1<<VSA_ID_VBD))
		voc_play(16000, (uint8_t *)bibi_front_16k_8bits, bibi_front_16k_8bitsLen);// 

	else if(p_vsa->alert_pend & (1<<VSA_ID_CRD))	
		voc_play(16000, (uint8_t *)bibi_front_16k_8bits, bibi_front_16k_8bitsLen);// 
	else if(p_vsa->alert_pend & (1<<VSA_ID_CRD_REAR))	
		voc_play(16000, (uint8_t *)bibi_behind_16k_8bits, bibi_behind_16k_8bitsLen);// 

	rt_timer_control(p_cms_envar->sys.timer_voc,RT_TIMER_CTRL_SET_TIME,(void*)&timevalue);
}

#if 0
void timer_out_cpuusage(void* parameter)
{
	uint8_t cpuusage_maj,cpuusage_min;
	
	cpu_usage_get(&cpuusage_maj,&cpuusage_min);

	rt_kprintf("cpu usage = %d%\n",cpuusage_maj);
	
}
#endif
void sys_human_interface_proc(sys_envar_t *p_sys, sys_msg_t *p_msg)
{
    if (p_msg->id == SYS_MSG_HI_OUT_UPDATE){
        switch(p_msg->argc){
			case HI_OUT_SYS_INIT:
				p_sys->led_priority |= 1<<HI_OUT_GPS_LOST;
				voc_play(16000, (uint8_t *)bibi_behind_16k_8bits, 3200);
				break;

			case HI_OUT_BSM_UPDATE:
					p_sys->led_priority |= 1<<SYS_MSG_BSM_UPDATE;
				break;
			case HI_OUT_BSM_NONE:
					p_sys->led_priority &= ~(1<<SYS_MSG_BSM_UPDATE);
				break;				
				
            case HI_OUT_CRD_ALERT:
				if(!p_sys->voc_flag)
					{
                		voc_play(16000, (uint8_t *)bibi_front_16k_8bits, bibi_front_16k_8bitsLen);
						p_sys->voc_flag = 0xff;
					}
                rt_timer_start(p_cms_envar->sys.timer_hi);
				if(p_sys->led_priority&(1<<HI_OUT_CRD_ALERT))
					return;
				else
					{
             			p_sys->led_priority |= 1<<HI_OUT_CRD_ALERT;
						//p_sys->led_priority &= ~(1<<HI_OUT_SYS_BSM);
					}
				break;

			case HI_OUT_CRD_REAR_ALERT:
                if(!p_sys->voc_flag)
					{
                		voc_play(16000, (uint8_t *)bibi_behind_16k_8bits, bibi_behind_16k_8bitsLen);
						p_sys->voc_flag = 0xff;
					}
                rt_timer_start(p_cms_envar->sys.timer_hi);
				if(p_sys->led_priority&(1<<HI_OUT_CRD_REAR_ALERT))
					return;
				else
					{
             			p_sys->led_priority |= 1<<HI_OUT_CRD_REAR_ALERT;
						//p_sys->led_priority &= ~(1<<HI_OUT_SYS_BSM);
					}	
				break;
				
            case HI_OUT_VBD_ALERT:
                rt_timer_start(p_cms_envar->sys.timer_voc);
				if(p_sys->led_priority&(1<<HI_OUT_VBD_ALERT))
					return;
				else
					{
			   			p_sys->led_priority |= 1<<HI_OUT_VBD_ALERT;
						//p_sys->led_priority &= ~(1<<HI_OUT_SYS_BSM);
						OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_INFO,"vbd alert!!\n\n");
					}	
                break;				

			case HI_OUT_EBD_ALERT:
              //  voc_play(16000, (uint8_t *)voice_16k_8bits, voice_16k_8bitsLen);
              	rt_timer_start(p_cms_envar->sys.timer_voc);            
			   	p_sys->led_priority |= 1<<HI_OUT_EBD_ALERT;
                break;

			case HI_OUT_CRD_CANCEL:
				//if(p_cms_envar->vsa.alert_pend == 0)
					//rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_CRD_ALERT);

				break;

			case HI_OUT_CRD_REAR_CANCEL:
				//if(p_cms_envar->vsa.alert_pend == 0)
					//rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_CRD_REAR_ALERT);

				break;	

			case HI_OUT_VBD_CANCEL:
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_VBD_ALERT);
                OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_INFO,"vbd alert  cancel!!\n\n");
				break;

			case HI_OUT_EBD_CANCEL:
				if(p_cms_envar->vsa.alert_pend == 0)
					rt_timer_stop(p_cms_envar->sys.timer_voc);
				p_sys->led_priority &= ~(1<<HI_OUT_EBD_ALERT);

				break;
				
			case HI_OUT_VBD_STATUS:
				p_sys->led_priority |= 1<<HI_OUT_VBD_STATUS;
				break;
			case HI_OUT_EBD_STATUS:
				p_sys->led_priority |= 1<<HI_OUT_EBD_STATUS;
				break;

			case HI_OUT_VBD_STOP:
				p_sys->led_priority &= ~(1<<HI_OUT_VBD_STATUS);
				break;
			case HI_OUT_EBD_STOP:
				p_sys->led_priority &= ~(1<<HI_OUT_EBD_STATUS);
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
                //voc_play(16000, (uint8_t *)notice_16k_8bits, 6400);
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

    }
	

	if((p_sys->led_priority&(1<<HI_OUT_CRD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_CRD_REAR_ALERT))\
		||(p_sys->led_priority&(1<<HI_OUT_EBD_ALERT))||(p_sys->led_priority&(1<<HI_OUT_VBD_ALERT)))
		{
			p_sys->led_color = LED_RED;//r=1,b=0,g=0
		    p_sys->led_blink_duration = 0xFFFF;
            p_sys->led_blink_period = 15;
            p_sys->led_blink_cnt = 0;
	}
	else if((p_sys->led_priority&(1<<HI_OUT_EBD_STATUS))||(p_sys->led_priority&(1<<HI_OUT_VBD_STATUS)))
		{
			p_sys->led_color = LED_GREEN;//r=1,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
            p_sys->led_blink_period = 15;
            p_sys->led_blink_cnt = 0;
	}

	else if(p_sys->led_priority&(1<<HI_OUT_GPS_LOST))
		{
			p_sys->led_color = LED_BLUE;//r=1,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
            p_sys->led_blink_period = 25;
            p_sys->led_blink_cnt = 0;
		}
	else if(p_sys->led_priority&(1<<SYS_MSG_BSM_UPDATE))
		{
            p_sys->led_color = LED_BLUE;//r=1,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
			p_sys->led_blink_period = 25;
			p_sys->led_blink_cnt = 0;
			//p_sys->led_priority &= ~(1<<HI_OUT_SYS_BSM);

	}
	else {
			p_sys->led_color = LED_BLUE;//r=0,b=0,g=1
			p_sys->led_blink_duration= 0xFFFF;
			p_sys->led_blink_period = 0xFFFF;
			p_sys->led_blink_cnt = 0;

	}
		
	
}

void rt_hi_thread_entry(void *parameter)
{
    rt_err_t err;
    sys_msg_t msg, *p_msg = &msg;
    sys_envar_t *p_sys = (sys_envar_t *)parameter;
	//static uint8_t ledss = 0xff;

    rt_timer_start(p_sys->timer_hi);

	while(1){
        err = rt_mq_recv(p_sys->queue_sys_hi, p_msg, sizeof(sys_msg_t), RT_WAITING_NO);
        if (err == RT_EOK){
            sys_human_interface_proc(p_sys, p_msg);
        }
/*
		if(p_sys->led_priority&(1<<HI_OUT_SYS_BSM))
		if(list_empty(&(p_cms_envar->vam.neighbour_list)))		
		p_sys->led_priority &= ~(1<<HI_OUT_SYS_BSM);
*/
        /* update led status */    
#if 0
            if (p_sys->led_blink_period == 0){/* always off */
                led_off(p_sys->led_color);
            }
            else if (p_sys->led_blink_period == 0xFFFF){ /* always on */
				led_on(p_sys->led_color);
				#if 0
				if(led_light_dark )
					{
                		led_on(p_sys->led_color);
						Delay_led(breath_led);
						//rt_thread_delay(breath_led)
						led_off(p_sys->led_color);
						Delay_led(BREATH_CYCLE - breath_led);
						breath_led++;
						if(breath_led >= BREATH_CYCLE) 
							{

								led_light_dark = 0;
								//rt_thread_delay(200);
								led_on(p_sys->led_color);
								Delay_led(50);
							}
					}
				else{
						led_on(p_sys->led_color);
						Delay_led(breath_led);
						led_off(p_sys->led_color);
						Delay_led(BREATH_CYCLE - breath_led);
						breath_led--;
						if(breath_led <= 0) 
							{

								led_light_dark = 1;
								//rt_thread_delay(200);
								Delay_led(50);
							}
					}
				#endif
            }
            else{ /* blink periodly */
                if (++p_sys->led_blink_cnt >= p_sys->led_blink_period){
                    led_blink(p_sys->led_color);
                    p_sys->led_blink_cnt = 0;
                    if(p_sys->led_blink_duration != 0xFFFF){
                        p_sys->led_blink_duration--;
                        if(p_sys->led_blink_duration <= 0){
                            p_sys->led_blink_period = 0;
                        }
                    }
                }
            
        }
#endif
        rt_thread_delay(1);
	}
}

void sys_init(void)
{
    sys_envar_t *p_sys = &p_cms_envar->sys;
	
	vsa_envar_t *p_vsa = &p_cms_envar->vsa;

    /* object for sys */
    p_sys->queue_sys_mng = osal_queue_create("sysc", SYS_QUEUE_SIZE);
    osal_assert(p_sys->queue_sys_mng != NULL);

    p_sys->task_sys_mng = osal_task_create("sysc",
                           sysc_thread_entry, p_sys,
                           RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);
    osal_assert(p_sys->task_sys_mng != NULL);

    /* object for human interface */
    p_sys->queue_sys_hi = rt_mq_create("q-hi", sizeof(sys_msg_t), SYS_QUEUE_SIZE, RT_IPC_FLAG_FIFO);
    RT_ASSERT(p_sys->queue_sys_hi != RT_NULL);

    p_sys->task_sys_hi = rt_thread_create("t-hi",
                           rt_hi_thread_entry, p_sys,
                           RT_HI_THREAD_STACK_SIZE, RT_HI_THREAD_PRIORITY, 20);
    RT_ASSERT(p_sys->task_sys_hi != RT_NULL)
    rt_thread_startup(p_sys->task_sys_hi);

    p_sys->timer_hi = rt_timer_create("tm-hi",timer_human_interface_callback,p_sys,\
        HUMAN_ITERFACE_VOC,RT_TIMER_FLAG_ONE_SHOT); 					
    RT_ASSERT(p_sys->timer_hi != RT_NULL);

    p_sys->timer_voc= rt_timer_create("tm-voc",timer_out_vsa_process,p_vsa,\
        1,RT_TIMER_FLAG_PERIODIC); 					
    RT_ASSERT(p_sys->timer_hi != RT_NULL);

#if 0
    p_sys->timer_cpuusage= rt_timer_create("tm-cpuusage",timer_out_cpuusage,NULL,\
    SECOND_TO_TICK(3),RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER); 					
    RT_ASSERT(p_sys->timer_cpuusage != RT_NULL);

    rt_timer_start(p_sys->timer_cpuusage);
#endif
#if 0
    p_sys->timer_crd= rt_timer_create("tm-crd",timer_out_crd_process,p_vsa,\
        HUMAN_ITERFACE_VOC,RT_TIMER_FLAG_ONE_SHOT); 					
    RT_ASSERT(p_sys->timer_crd != RT_NULL);
#endif
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial\n");
}


/* below are for debug */
void test_alert(void)
{
    hi_add_event_queue(&p_cms_envar->sys, SYS_MSG_HI_OUT_UPDATE,0,HI_OUT_VBD_ALERT, 0);

	p_cms_envar->vsa.alert_pend |= VSA_ID_CRD;
}
FINSH_FUNCTION_EXPORT(test_alert, debug: testing alert voice and led);

void start_voc( uint8_t  type)
{

    voc_play(16000, (uint8_t *)bibi_behind_16k_8bits, bibi_behind_16k_8bitsLen);// 

}
	

FINSH_FUNCTION_EXPORT(start_voc, debug: testing alert voice);

