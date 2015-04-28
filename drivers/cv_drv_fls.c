/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_drv_fls.c
 @brief  : this file include the flash driver functions
 @author : wangyifeng
 @history:
           2014-6-26    wangyifeng    Created file
           ...
******************************************************************************/
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

/**
    below functions are only simple realization for testing
*/

int drv_fls_read(uint32_t flash_address, uint8_t *p_databuf, uint32_t length)
{
    memcpy(p_databuf, (uint8_t *)flash_address, length);
	return 0;
}

int drv_fls_erase(uint32_t  sector)
{
    int err = 0;
    
    /* Enable the flash control register access */
    FLASH_Unlock();

    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

    if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE){
        err = -1;
    }

    /* Disable the flash control register access */
    FLASH_Lock();

	return err;
}


int drv_fls_write(uint32_t flash_address, uint8_t *p_databuf, uint32_t length)
{
    int err = 0;
    
    /* Enable the flash control register access */
    FLASH_Unlock();

    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

    while(length > 0){
        if ((FLASH_ProgramByte(flash_address, *p_databuf)) != FLASH_COMPLETE){
            err = -1;
            break;
        }
        flash_address++;
        p_databuf++;
        length--;
    }

    /* Disable the flash control register access */
    FLASH_Lock();

	return err;
}

