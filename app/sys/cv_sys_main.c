/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"

#define FIRMWARE_VERSION "V1.0.000" 
#ifdef NDEBUG
#define FIRMWARE_IDEN "rel" 
#else
#define FIRMWARE_IDEN "dbg" 
#endif

extern void osal_dbg_init(void);
extern void rt_platform_init(void);
extern void param_init(void);
extern void gps_init(void);
extern void voc_init(void);
extern void led_init(void);
extern void wnet_init(void);
extern void vam_init(void);
extern void vsa_init(void);
extern void sys_init(void);
extern void gsnr_init(void);
extern int usb_init(void);
extern int rt_key_init(void);
extern void mda_init(void);
extern void voc_init(void);
extern void cpu_usage_init(void);
cms_global_t cms_envar, *p_cms_envar;


void global_init(void)
{
    p_cms_envar = &cms_envar;
    memset(p_cms_envar, 0, sizeof(cms_global_t));
}


void rt_init_thread_entry(void *parameter)
{
    osal_dbg_init();

    global_init();
    param_init();
    cpu_usage_init();
    gps_init();
  	nmea_init();
    //voc_init();
    led_init();
	rt_key_init();
    usb_init();

    wnet_init();
    vam_init();
    mda_init();
    gsnr_init();
    voc_init();
    sys_init();
    vsa_init();    


    //EVAL_AUDIO_Init(4,100, I2S_AudioFreq_16k);
    //quit...
}

int rt_application_init(void)
{
    rt_thread_t tid;

    rt_components_init();
    rt_platform_init();

    osal_printf("\n\n");
    osal_printf("CID : %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", 
                des(0), des(1), des(2), des(3), des(4), des(5), des(6), des(7), des(8), des(9), des(10), des(11));
    osal_printf("CLK : %dMHz\n", SystemCoreClock/1000000);
    osal_printf("Firm: %s[%s,%s %s]\n\n", FIRMWARE_VERSION, FIRMWARE_IDEN, __TIME__, __DATE__);

    tid = osal_task_create("init", rt_init_thread_entry, RT_NULL,
                           RT_INIT_THREAD_STACK_SIZE, RT_INIT_THREAD_PRIORITY);
    osal_assert(tid != NULL);

    return 0;
}

