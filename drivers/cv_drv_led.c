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
        STM_EVAL_LEDOn((Led_TypeDef)led);
    }

}

void led_off(Led_TypeDef led)
{
    if (led < LEDn){
        STM_EVAL_LEDOff((Led_TypeDef)led);
    }
}

void led_blink(Led_TypeDef led)
{
    if (led < LEDn){
        STM_EVAL_LEDBlink((Led_TypeDef)led);
    }


}

static void led_thread_entry(void *parameter)
{

    osal_status_t err;
    sys_msg_t msg, *p_msg = &msg;
    sys_envar_t *p_sys = (sys_envar_t *)parameter;

    while(1){
        osal_printf("this hi thread!!\n\n");
        if (++RED_blink_cnt >= RED_blink_period){
            led_blink(LED_RED);
            RED_blink_cnt = 0;
            }
        err = osal_queue_recv(p_sys->queue_sys_hi, &p_msg, OSAL_WAITING_FOREVER);
        if (err == OSAL_STATUS_SUCCESS){
            sys_human_interface_proc(p_sys, p_msg);
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

    
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial\n\n");         
	return 0;
}


