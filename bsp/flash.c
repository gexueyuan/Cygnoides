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
#include "flash.h"

FlashErrCode flash_read(uint32_t flash_address, uint8_t *p_databuf, uint32_t length)
{
    memcpy(p_databuf, (uint8_t *)flash_address, length);
	return FLASH_NO_ERR;
}

FlashErrCode flash_erase(uint32_t  sector)
{
    FlashErrCode err = FLASH_NO_ERR;
    
    /* Enable the flash control register access */
    FLASH_Unlock();

    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

    if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE){
        err = FLASH_ERASE_ERR;
    }

    /* Disable the flash control register access */
    FLASH_Lock();

	return err;
}


FlashErrCode flash_write(uint32_t flash_address, uint8_t *p_databuf, uint32_t length)
{
    FlashErrCode err = FLASH_NO_ERR;
    
    /* Enable the flash control register access */
    FLASH_Unlock();

    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

    while(length > 0){
        if ((FLASH_ProgramByte(flash_address, *p_databuf)) != FLASH_COMPLETE){
            err = FLASH_WRITE_ERR;
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

