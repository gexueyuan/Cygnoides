/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_drv_file.c
 @brief  : this file include filesysterm function
 @author : fred
 @history:
           2015-7-7    fred    Created file
           ...
******************************************************************************/

#define _CV_DRV_FILE_MODELE_
#include "components.h"
#include "cv_drv_file.h"
#include "cv_osal_dbg.h"
#include "SPI_MSD0_Driver.h"


#define FILE_BUF_NUM     (2)
#define FILE_MAX_SIZE     (1024*30)
/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/

uint8_t sd_state;

/* need be global  */
FATFS file_sys;
uint32_t file_label;

log_file_port log_file;

rtc_manage_t rtc_manage;

uint8_t month_day[12] ={31,28,31,30,31,30,31,31,30,31,30,31};
/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
void  rtc_init(void)
{
    memset(&rtc_manage,0x00,sizeof(rtc_manage));
}

void update_time_zone(void)
{
    if (rtc_manage.rtc_time.hour >=24) {
        rtc_manage.rtc_time.hour -= 24;
        rtc_manage.rtc_time.day +=1;
    }

    if ((rtc_manage.rtc_time.mon == 1) ||(rtc_manage.rtc_time.mon == 3)
        ||(rtc_manage.rtc_time.mon == 5) ||(rtc_manage.rtc_time.mon == 7) 
        || (rtc_manage.rtc_time.mon == 8) ||(rtc_manage.rtc_time.mon == 10)) {

        if (rtc_manage.rtc_time.day >31) {
            rtc_manage.rtc_time.day = 1;
            rtc_manage.rtc_time.mon++;
        }
    }
    else if ((rtc_manage.rtc_time.mon == 4) || (rtc_manage.rtc_time.mon == 6) 
        ||(rtc_manage.rtc_time.mon == 9) ||(rtc_manage.rtc_time.mon == 11)) {

        if (rtc_manage.rtc_time.day >30) {
            rtc_manage.rtc_time.day = 1;
            rtc_manage.rtc_time.mon++;
        }
    }
    else if (rtc_manage.rtc_time.mon == 12) {

        if (rtc_manage.rtc_time.day >31) {
            rtc_manage.rtc_time.day = 1;
            rtc_manage.rtc_time.mon = 1;
            rtc_manage.rtc_time.year ++;   
        }
    }
    else if (rtc_manage.rtc_time.mon == 2) {
        if (rtc_manage.rtc_time.year%4 == 0) {
            if (rtc_manage.rtc_time.day > 29) {
                rtc_manage.rtc_time.mon++;
                rtc_manage.rtc_time.day  =1;
            }
        }
        else {
            if (rtc_manage.rtc_time.day > 28) {
                rtc_manage.rtc_time.mon++;
                rtc_manage.rtc_time.day  =1;
            }
        }
    }
}


uint8_t  rtc_time_get(rtc_time_t *time)
{

    if (rtc_manage.flag == TRUE) {
        memcpy(time,&rtc_manage.rtc_time,sizeof(rtc_time_t));
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void updata_rtc_time(t_time *time)
{
    rtc_manage.flag = TRUE;
    rtc_manage.rtc_time.year = time->year;
    rtc_manage.rtc_time.mon = time->mon;
    rtc_manage.rtc_time.day = time->day;
    rtc_manage.rtc_time.hour = time->hour;
    rtc_manage.rtc_time.min =  time->min;
    rtc_manage.rtc_time.sec = time->sec;

    /* Time zone correct */
    rtc_manage.rtc_time.hour +=8;
    if (rtc_manage.rtc_time.hour >=24) {
        update_time_zone();
    }

    #if 0
   osal_printf("RTC: %d-%d-%d %d:%d:%d\r\n", rtc_manage.rtc_time.year, \
      rtc_manage.rtc_time.mon, rtc_manage.rtc_time.day, rtc_manage.rtc_time.hour, \
      rtc_manage.rtc_time.min, rtc_manage.rtc_time.sec);
     #endif
}


void file_spi_phase_set(void)
{   
    MSD0_SPIPhase_set();
}

uint8_t take_spi_control(int32_t wait_time)
{
    osal_status_t result;

    if (share_spi.spi_mutex) {
        result = osal_sem_take(share_spi.spi_mutex, wait_time);
        if (result == OSAL_STATUS_SUCCESS) {
            return TRUE;
        }
            return FALSE;
    }
    return TRUE;
}

uint8_t release_spi_control(void)
{
    if (share_spi.spi_mutex) {
        osal_sem_release(share_spi.spi_mutex);
    }
    return 0;
}


uint8_t log_file_new(void)
{
    FRESULT res;
    uint8_t file_name[20]={0};
    rtc_time_t time;

    //memset(&log_file,0x00,sizeof(log_file));
    log_file.state = TRUE;
    if (rtc_time_get(&time) == TRUE) {
        sprintf((char *)file_name,"%d%d%d-%d%d.txt",(time.year%100),time.mon,time.day,time.hour,time.min);
        log_file.name_flag = TRUE;
    }
    else {
        file_label++;
        sprintf((char *)file_name,"%d.txt",file_label);
        log_file.name_flag = FALSE;
    }
    
    memcpy(log_file.name,file_name,sizeof(file_name));
    {
        file_spi_phase_set();
        res = f_open(&log_file.file_des,(char *)log_file.name,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
        if (res != FR_OK) {
            log_file.state = FALSE;
            osal_printf("log->new file error:%d\r\n",res);
        }
        f_lseek(&log_file.file_des,f_size((&log_file.file_des)));
        return (res == FR_OK)?TRUE:FALSE;
    }
    return FALSE;
}


uint8_t log_file_create(void)
{
    FRESULT res;
    uint8_t file_name[20]={0};
    rtc_time_t time;

    memset(&log_file,0x00,sizeof(log_file));
    log_file.state = TRUE;
    if (rtc_time_get(&time) == TRUE) {
        sprintf((char *)file_name,"%d%d%d-%d%d.txt",(time.year%100),time.mon,time.day,time.hour,time.min);
        log_file.name_flag = TRUE;
    }
    else {
        file_label++;
        sprintf((char *)file_name,"%d.txt",file_label);
        log_file.name_flag = FALSE;
    }
    
    memcpy(log_file.name,file_name,sizeof(file_name));
    {
        file_spi_phase_set();
        res = f_open(&log_file.file_des,(char *)log_file.name,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
        if (res != FR_OK) {
            osal_printf("log->new file error:%d\r\n",res);
        }
        f_lseek(&log_file.file_des,f_size((&log_file.file_des)));
        return (res == FR_OK)?TRUE:FALSE;
    }
		return FALSE;
}

uint8_t log_file_write(void *data,uint32_t len)
{
    FRESULT res;
    uint32_t w_len;
    uint32_t w_count;
    uint8_t *log_data = (uint8_t *)data;
    //uint32_t  time1,time2,time3,time4;
    rtc_time_t time;
    uint8_t file_name[20]={"wang.txt"};
    uint8_t f_count;
    static uint32_t count;

    for(w_count=0;w_count<len;w_count++) {
        file_write_m.file_buf[file_write_m.file_wptr++] = log_data[w_count];

        if (file_write_m.file_wptr == FILE_WRITE_LEN) {
            log_file.write_ptr = &file_write_m.file_buf[0];
            log_file.write_len = FILE_WRITE_LEN;
            log_file.write_flag = TRUE;
        }
        if (file_write_m.file_wptr >= FILE_WRITE_LEN*2) {
            file_write_m.file_wptr = 0;
            log_file.write_ptr = &file_write_m.file_buf[FILE_WRITE_LEN];
            log_file.write_len = FILE_WRITE_LEN;
            log_file.write_flag = TRUE;
        }
    }
    
    if ((TRUE == log_file.write_flag) && (TRUE == log_file.state)) {

        do {
            /*rename file name*/
            if (log_file.name_flag == FALSE) {
                if (rtc_time_get(&time) == TRUE){
                    #if 0
                        log_file_new();
                        log_file.name_flag = TRUE;
                    #else 
                    do {
                        log_file.name_flag = TRUE; 
                        sprintf((char *)file_name,"%02d%02d%02d-%02d%02d.txt",(time.year%100),time.mon,time.day,time.hour,time.min);

                        /* rename file  */
                        f_close(&log_file.file_des);
                        res = f_rename((char *)log_file.name,(char *)file_name);
                        if (res != FR_OK) {
                            break;
                        }
                        memcpy(log_file.name,file_name,sizeof(file_name));
                        res = f_open(&log_file.file_des,(char *)log_file.name,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
                         if (res != FR_OK) {
                            break;
                        }
                        f_lseek(&log_file.file_des,f_size((&log_file.file_des)));
                    }while(0);
                    
                    if (res !=FR_OK) {
                        log_file_new();
                    }
                    #endif 
                }
            }
            /* write data to file */
            res = f_write(&log_file.file_des,log_file.write_ptr,log_file.write_len,&w_len);
            if ((res != FR_OK) || (w_len == 0)) {
                osal_printf("write data to file error\n");
                log_file.state = FALSE;
                break;
            }
            else {
                res = f_sync(&log_file.file_des);
                if (res != FR_OK) {
                    osal_printf("sync data to file error\n");
                    log_file.state = FALSE;
                    break;
                }
            }
            log_file.write_flag = FALSE;
        }while(0);
    }
    return 0;
		
}

uint8_t file_sys_init(void)
{
    FRESULT res;
    FIL  fdes;
    uint8_t test_buf[]="fred";
    uint8_t read_buf[10];
    uint32_t w_len;
    uint32_t r_len;

    /* mount file systerm */
    res = f_mount(0,&file_sys);
    if (res != FR_OK) {
        return FALSE;
    }

    //#if (0)
    res = f_open(&fdes,"test.txt",FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
    if (res != FR_OK) {
        return FALSE;
    }

    res = f_write(&fdes,test_buf,sizeof(test_buf),&w_len);
    if ((res != FR_OK) || (w_len == 0)) {
        return FALSE;
    }

    f_close(&fdes);

    if(res != FR_OK) {
        return FALSE;
    }

    res = f_open(&fdes, "test.txt", FA_OPEN_EXISTING | FA_READ);
    res = f_read(&fdes,read_buf,sizeof(test_buf),&r_len);
    if ((res != FR_OK) || (r_len == 0)) {
        return FALSE;
    }

    if (memcmp(read_buf,test_buf,sizeof(test_buf))) {
        return FALSE;
    }

    f_close(&fdes);

    return TRUE;

    //#endif

}


uint8_t disk_sd_init(void)
{
    //uint8_t sd_return;

    //disk_initialize(0);
    /*Samsung card need init twice*/
    if (disk_initialize(0) == 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}
/*
uint8_t creat_file_system(void)
{
    uint8_t f_return;
    
    f_return = file_sys_init();
    if (f_return == TRUE) {
        osal_printf("file systerm init OK!\r\n");
    } else {
        osal_printf("file systerm init error!\r\n");

    }
}
*/
static uint8_t file_systerm_init(void)
{
    uint8_t sd_condition = FALSE;
    
    /* init sd card */
    if (disk_cart_check() == TRUE) {
        if (disk_sd_init()) {
            osal_printf("sd card init OK\n");
            if (file_sys_init()) {
                sd_condition = TRUE;
            }
        }
        else {
            osal_printf("sd card init ERROR\n");
        }
    }
    else {
        osal_printf("sd card not exist!\r\n");
    }
    return sd_condition;
}


uint8_t log_sd_init(void) 
{
    if (file_systerm_init()) {
        memset(&log_file,0x00,sizeof(log_file));
        if (log_file_create()) {
            memset(&file_write_m,0,sizeof(file_write_m));
            return TRUE;
        }
    }
    return FALSE;
}

void log_store_set(uint8_t data)
{
    if ((data == 1) || (data == 2) ||(data == 3)) {
        log_store.log_media = data;
    }
    else {
        osal_printf("input data error\n");
    }
}

FINSH_FUNCTION_EXPORT(log_store_set, set log output media 1:sd card 2: uart 3: both)



