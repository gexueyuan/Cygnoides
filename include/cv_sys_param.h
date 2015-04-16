/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_sys_param.h
 @brief  : head file of vanet's param 
 @author : gexueyuan
 @history:
           2015-2-4    gexueyuan    Created file
           ...
******************************************************************************/

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/


/* page size for stm32 flash */
#define PAGE_SIZE     128*1-24
/* Environment variables start address */
#define FLASH_ENV_START_ADDR            //(FLASH_BASE + 100 * 1024) /* from the chip position: 100KB */
/* the minimum size of flash erasure */
#define FLASH_ERASE_MIN_SIZE             PAGE_SIZE                /* it is one page for STM32 */
/* Environment variables bytes size */
#define FLASH_ENV_SECTION_SIZE           PAGE_SIZE           /* 1 pages */
/* print debug information of flash */
#define FLASH_PRINT_DEBUG



typedef struct _flash_param{
    char *key;
    char *value_cus;
    char *value_hw;
    char *value_mt;
    char *value_ct;
}flash_param, *flash_param_t;

/* Flash error code */
typedef enum {
    FLASH_NO_ERR,
    FLASH_ERASE_ERR,
    FLASH_WRITE_ERR,
    FLASH_ENV_NAME_ERR,
    FLASH_ENV_NAME_EXIST,
    FLASH_ENV_FULL,
} FlashErrCode;



typedef struct {

	cfg_param_t cus_param;
	cfg_param_t hw_param;
	cfg_param_t mt_param;
	cfg_param_t ct_param;

}vanet_param;


#define PARAM_FLAG_ADDR     ((uint32_t)0x80E0000)

#define PARAM_MODE_ADDR     ((uint32_t)0x80E0010)

#define PARAM_ADDR    		((uint32_t)0x80E0020)

#define name_to_str(name)   (#name)






static const flash_param default_param_data[] = {

    {"0_ID","0","0","0","0"},
    {"1_vam.bsm_hops","0","0","0","0"},
    {"2_vam.bsm_boardcast_mode","0","0","0","0"},
    {"3_vam.bsm_boardcast_saftyfactor","0","0","0","0"},
    {"4_vam.bsm_pause_mode","0","0","0","0"},

};
