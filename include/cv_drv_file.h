/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_drv_file.h
 @brief  : cv_drv_file.c header file
 @author : fred
 @history:
           2015-7-7    fred    Created file
           ...
******************************************************************************/



/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

#ifndef __CV_DRV_FILE_H__
#define __CV_DRV_FILE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "cv_osal.h"
#include "diskio.h"
#include "ff.h"
#include "nmea.h"


#define LOG_SD_CARD   (1)
#define LOG_UART          (2)
#define LOG_BOTH         (3)

typedef struct _log_store_
{
    uint8_t log_media;
    uint8_t sd_state;
    
}log_store_t;

/* log file manage struct */
typedef struct _log_file_port_
{
    uint8_t  state;
    FIL         file_des;
    uint32_t file_sleek;
    uint8_t name_flag;
    uint8_t  name[20];
    uint8_t  path[10];
    uint8_t  write_flag;
    uint8_t *write_ptr;
    uint32_t write_len;
}log_file_port;


typedef struct _share_spi_
{
    osal_sem_t *spi_mutex;
    
}share_spi_t;

#define FILE_WRITE_LEN  (512)
#define FILE_BUF_MAX   (FILE_WRITE_LEN*2)


typedef struct _file_write_manage_
{
    uint8_t flag;
    uint32_t file_wptr;
    uint8_t file_buf[FILE_BUF_MAX];
}file_write_manage_t;


typedef struct _rtc_time_
{
    uint32_t year;
    uint32_t mon;
    uint32_t day;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
}rtc_time_t;

typedef struct _rtc_manage_
{
    uint8_t flag;
    rtc_time_t rtc_time;  
}rtc_manage_t;


#ifdef _CV_DRV_FILE_MODELE_
#define DRV_FILE_EXTERN   
#else 
#define DRV_FILE_EXTERN   extern 
#endif

DRV_FILE_EXTERN   share_spi_t    share_spi;
DRV_FILE_EXTERN   log_store_t    log_store;
DRV_FILE_EXTERN file_write_manage_t file_write_m;

extern uint8_t file_systerm_init(void);
uint8_t log_file_new(void);
uint8_t log_file_write(void *data,uint32_t len);
uint8_t take_spi_control(int32_t wait_time);
void file_spi_phase_set(void);
uint8_t release_spi_control(void);
uint8_t log_sd_init(void) ;
void rtc_init(void);
void updata_rtc_time(t_time *time);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CV_DRV_FILE_H__ */
