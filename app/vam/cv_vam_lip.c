
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"
#include "nmea.h"


#if 0

void timeout_callbak(void* parameter);

/*  变量分配4字节对齐 */
ALIGN(RT_ALIGN_SIZE)

static rt_timer_t timer_test;
static rt_tick_t timeout_value=10;

static rt_err_t timer_start(void)
{
    timer_test = rt_timer_create("timer1",timeout_callbak,RT_NULL,timeout_value,RT_TIMER_FLAG_PERIODIC); 					
    if (timer_test != RT_NULL){
        rt_timer_start(timer_test);
	}
    return 0;
}

void timer_conrol(void)
{
    timeout_value+=10;
    rt_timer_control(timer_test, RT_TIMER_CTRL_SET_TIME, (void *)&timeout_value);
//    rt_kprintf("timer timeout time set to %d !\n", timeout_value);
    if (timeout_value==500)    {
        rt_timer_stop(timer_test); /* 停止定时器 */
        rt_kprintf("timer stoped !\n");
    }
    if (timeout_value>=510) {
        /* 再次启动定时器 */
        rt_timer_start(timer_test);
        timeout_value=10;
        rt_timer_control(timer_test, RT_TIMER_CTRL_SET_TIME, (void *)&timeout_value);
    }

}

int32_t _vsm_get_local_status(vam_stastatus_t *local)
{
	t_nmea_rmc recvGPS;
	if(recvGPS.isTrue){
		nmea_get(&recvGPS, 1);
		local->pos.latitude = recvGPS.latitude;
		local->pos.longitude = recvGPS.longitude ;
		local->speed = recvGPS.speed ;
	}
	else{
		rt_memset(local,0,sizeof(local));
	}
	return RT_EOK ;
}

void rt_vam_thread_entry(void *parameter)
{
	vam_stastatus_t location ;
	rt_thread_delay(500);
	timer_start();
	while(1){
		_vsm_get_local_status(&location) ;
		rt_thread_delay(10);
	}
}


/* 超时时回调的处理函数 */
void timeout_callbak(void* parameter)
{
    timer_conrol();
}




#endif

void lip_gps_proc(vam_envar_t *p_vam, uint8_t *databuf, uint32_t len)
{
    nmea_parse(databuf, len);
}

void lip_update_local(t_nmea_rmc *p_rmc, float *p_accu)
{
    vam_envar_t *p_vam = &p_cms_envar->vam;
    
    if (p_rmc){
        p_vam->local.pos.lat = (float)p_rmc->latitude;
    	p_vam->local.pos.lon = (float)p_rmc->longitude;
        if (p_vam->local.speed != (float)p_rmc->speed)
        {
    	    p_vam->local.speed = (float)p_rmc->speed;
            vsm_update_bsm_bcast_timer(p_vam);
        }
        p_vam->local.dir = (float)p_rmc->heading;

        //dump_pos(&p_vam->local);

        if (!(p_vam->flag&VAM_FLAG_GPS_FIXED)){
            p_vam->flag |= VAM_FLAG_GPS_FIXED;
            rt_kprintf("gps is captured.\n");
            if (p_vam->evt_handler[VAM_EVT_GPS_STATUS]){
                (p_vam->evt_handler[VAM_EVT_GPS_STATUS])((void *)1);
            }
        }

        /* refresh the timer */
        rt_timer_stop(p_vam->timer_gps_life);
        rt_timer_start(p_vam->timer_gps_life);

        if(p_vam->evt_handler[VAM_EVT_LOCAL_UPDATE]){
            (p_vam->evt_handler[VAM_EVT_LOCAL_UPDATE])(&p_vam->local); 
        }
    }

    if (p_accu){
        p_vam->local.pos.accu = *p_accu;
    }
}

void lip_update_local_acc(float x, float y, float z)
{
    vam_stastatus_t *p_local = &(p_vam_envar->local);
    p_local->acce.lat = x;
    p_local->acce.lon = y;
    p_local->acce.vert = z;
}

