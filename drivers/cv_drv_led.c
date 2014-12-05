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


void led_on(led_color_t led)
{
    if (led.r)
       STM_EVAL_LEDOn((Led_TypeDef)LED_RED);
	else
	   STM_EVAL_LEDOff((Led_TypeDef)LED_RED);
    if (led.g)
       STM_EVAL_LEDOn((Led_TypeDef)LED_GREEN);
	else
	   STM_EVAL_LEDOff((Led_TypeDef)LED_GREEN);
    if (led.b)
       STM_EVAL_LEDOn((Led_TypeDef)LED_BLUE);
	else
	   STM_EVAL_LEDOff((Led_TypeDef)LED_BLUE);
}

void led_off(led_color_t led)
{
       STM_EVAL_LEDOff((Led_TypeDef)LED_RED);
       STM_EVAL_LEDOff((Led_TypeDef)LED_GREEN);
       STM_EVAL_LEDOff((Led_TypeDef)LED_BLUE);

}

void led_blink(Led_TypeDef led)
{
    if (led < LEDn){
        STM_EVAL_LEDBlink((Led_TypeDef)led);
    }


}

