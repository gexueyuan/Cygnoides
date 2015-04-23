/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_drv_led.c
 @brief  : this file include the LED display functions
 @author : wangyifeng
 @history:
           2014-6-30    wangyifeng    Created file
           ...
******************************************************************************/
#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "led"
#include "cv_osal_dbg.h"



#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"
#include "led.h"


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
#define LED_PERIOD           MS_TO_TICK(500)


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

void led_init(void)
{
    STM_EVAL_LEDInit(LED_RED);
    STM_EVAL_LEDInit(LED_BLUE);
    STM_EVAL_LEDInit(LED_GREEN);
    STM_EVAL_LEDOff(LED_RED);
    STM_EVAL_LEDOff(LED_BLUE);
    STM_EVAL_LEDOff(LED_GREEN);

}


void led_on(Led_TypeDef led)
{
    if (led < LEDn){
        STM_EVAL_LEDOff((Led_TypeDef)led);
    }

}

void led_off(Led_TypeDef led)
{
    if (led < LEDn){
        STM_EVAL_LEDOn((Led_TypeDef)led);
    }
}

void led_blink(Led_TypeDef led)
{
    if (led < LEDn){
        STM_EVAL_LEDBlink((Led_TypeDef)led);
    }


}

void  timer_red_blink_callback( void *parameter )
{

#ifdef HARDWARE_MODULE_WIFI_V2
    led_off((Led_TypeDef)LED_GREEN);
    led_off((Led_TypeDef)LED_BLUE);
#endif
    led_blink(LED_RED);

}

void  timer_green_blink_callback( void *parameter )
{

#ifdef HARDWARE_MODULE_WIFI_V2
    led_off((Led_TypeDef)LED_RED);
    led_off((Led_TypeDef)LED_BLUE);
#endif
    led_blink(LED_GREEN);
}

void  timer_blue_blink_callback( void *parameter )
{
#ifdef HARDWARE_MODULE_WIFI_V2
    led_off((Led_TypeDef)LED_GREEN);
    led_off((Led_TypeDef)LED_RED);
#endif
    led_blink(LED_BLUE);
}

#ifdef HARDWARE_MODULE_WIFI_V1
void led_proc(sys_envar_t *p_sys, sys_msg_t *p_msg)
{


	switch(p_msg->id){

		case LED_RED:
			if(p_msg->argc == LED_ON){
					osal_timer_stop(p_sys->timer_red);
					led_on((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_OFF){
					osal_timer_stop(p_sys->timer_red);
					led_off((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_BLINK)
					osal_timer_start(p_sys->timer_red);
			break;
		case LED_GREEN:
			if(p_msg->argc == LED_ON){
				
					osal_timer_stop(p_sys->timer_green);
					led_on((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_OFF){
					osal_timer_stop(p_sys->timer_green);
					led_off((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_BLINK)
					osal_timer_start(p_sys->timer_green);
			break;
		case LED_BLUE:
			if(p_msg->argc == LED_ON){
					osal_timer_stop(p_sys->timer_blue);
					led_on((Led_TypeDef)p_msg->id);					
				}
			else if(p_msg->argc == LED_OFF){
					osal_timer_stop(p_sys->timer_blue);
					led_off((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_BLINK)
				osal_timer_start(p_sys->timer_blue);
			break;
		default:
			break;

	}


}
#else
void led_proc(sys_envar_t *p_sys, sys_msg_t *p_msg)
{


	switch(p_msg->id){

		case LED_RED:
			if(p_msg->argc == LED_ON){
					osal_timer_stop(p_sys->timer_red);
					led_on((Led_TypeDef)LED_RED);
                    led_off((Led_TypeDef)LED_GREEN);
                    led_off((Led_TypeDef)LED_BLUE);
				}
			else if(p_msg->argc == LED_OFF){
					osal_timer_stop(p_sys->timer_red);
					led_off((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_BLINK)
					osal_timer_start(p_sys->timer_red);
			break;
		case LED_GREEN:
			if(p_msg->argc == LED_ON){
				
					osal_timer_stop(p_sys->timer_green);
					led_on((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_OFF){
					osal_timer_stop(p_sys->timer_green);
					led_off((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_BLINK)
					osal_timer_start(p_sys->timer_green);
			break;
		case LED_BLUE:
			if(p_msg->argc == LED_ON){
					osal_timer_stop(p_sys->timer_blue);
					led_on((Led_TypeDef)p_msg->id);					
				}
			else if(p_msg->argc == LED_OFF){
					osal_timer_stop(p_sys->timer_blue);
					led_off((Led_TypeDef)p_msg->id);
				}
			else if(p_msg->argc == LED_BLINK)
				    osal_timer_start(p_sys->timer_blue);
			break;
		default:
			break;

	}


}
#endif

static void led_thread_entry(void *parameter)
{

    osal_status_t err;
    sys_msg_t msg, *p_msg = &msg;
    sys_envar_t *p_sys = (sys_envar_t *)parameter;
    while(1){
        err = osal_queue_recv(p_sys->queue_hi_led, &p_msg, OSAL_WAITING_FOREVER);
        if (err == OSAL_STATUS_SUCCESS){
            led_proc(p_sys, p_msg);
            osal_free(p_msg);
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: osal_queue_recv error [%d]\n", __FUNCTION__, err);           
            osal_free(p_msg);
        }
    }

}

int rt_led_init(void)
{
    osal_task_t  *led_tid;
    sys_envar_t *p_sys = &p_cms_envar->sys;

    led_init();
    
    led_tid = osal_task_create("t-led",
                               led_thread_entry, p_sys,
                               RT_KEY_THREAD_STACK_SIZE, RT_PLAY_THREAD_PRIORITY);
    osal_assert(led_tid != NULL);

    p_sys->queue_hi_led = osal_queue_create("q-led",  2*VOC_QUEUE_SIZE);
    osal_assert(p_sys->queue_hi_led != NULL);

    
    p_sys->timer_red = osal_timer_create("tm-red",\
        timer_red_blink_callback,NULL,LED_PERIOD,RT_TIMER_FLAG_PERIODIC);
    osal_assert(p_sys->timer_red != NULL);

    p_sys->timer_green= osal_timer_create("tm-gren",\
        timer_green_blink_callback,NULL,LED_PERIOD,RT_TIMER_FLAG_PERIODIC);
    osal_assert(p_sys->timer_red != NULL);

	p_sys->timer_blue = osal_timer_create("tm-blue",\
	timer_blue_blink_callback,NULL,LED_PERIOD,RT_TIMER_FLAG_PERIODIC);
	osal_assert(p_sys->timer_red != NULL);
	
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial\n\n");   
	return 0;
}


