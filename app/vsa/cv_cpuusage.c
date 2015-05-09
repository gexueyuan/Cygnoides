#include <rtthread.h>
#include <rthw.h>
#include "components.h"
//#include "cv_cms_def.h"

#define CPU_USAGE_CALC_TICK    10
#define CPU_USAGE_LOOP        100

static rt_uint8_t  cpu_usage_major = 0, cpu_usage_minor= 0;
static rt_uint32_t total_count = 0;

static void cpu_usage_idle_hook()
{
    rt_tick_t tick;
    rt_uint32_t count;
    volatile rt_uint32_t loop;

    if (total_count == 0)
    {
        /* get total count */
        rt_enter_critical();
        tick = rt_tick_get();
        while(rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
        {
            total_count ++;
            loop = 0;
            while (loop < CPU_USAGE_LOOP) loop ++;
        }
        rt_exit_critical();
    }

    count = 0;
    /* get CPU usage */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
    {
        count ++;
        loop  = 0;
        while (loop < CPU_USAGE_LOOP) loop ++;
    }

    /* calculate major and minor */
    if (count < total_count)
    {
        count = total_count - count;
        cpu_usage_major = (count * 100) / total_count;
        cpu_usage_minor = ((count * 100) % total_count) * 100 / total_count;
    }
    else
    {
        total_count = count;

        /* no CPU usage */
        cpu_usage_major = 0;
        cpu_usage_minor = 0;
    }
}

void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor)
{
    RT_ASSERT(major != RT_NULL);
    RT_ASSERT(minor != RT_NULL);

    *major = cpu_usage_major;
    *minor = cpu_usage_minor;
}

void cpu_usage_print(void* parameter)
{
	rt_uint8_t cpuusage_maj,cpuusage_min;
	
	cpu_usage_get(&cpuusage_maj,&cpuusage_min);

	rt_kprintf("cpu usage = %d.%d%\n",cpuusage_maj,cpuusage_min);
	
}
FINSH_FUNCTION_EXPORT(cpu_usage_print,get cpu usage);

void cpu_usage_init()
{
    rt_timer_t tm_cpu_uage;

    /* set idle thread hook */
    rt_thread_idle_sethook(cpu_usage_idle_hook);
#if 1
     tm_cpu_uage = rt_timer_create("tm-cpu",cpu_usage_print,NULL,\
        ((5)*RT_TICK_PER_SECOND),RT_TIMER_FLAG_PERIODIC); 
     rt_timer_start(tm_cpu_uage);

#endif
}



