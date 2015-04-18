

#ifndef FLASH_H_
#define FLASH_H_
#include <stdint.h>
#include <stddef.h>

/* Flash error code */
typedef enum {
    FLASH_NO_ERR,
    FLASH_ERASE_ERR,
    FLASH_WRITE_ERR,
    FLASH_ENV_NAME_ERR,
    FLASH_ENV_NAME_EXIST,
    FLASH_ENV_FULL,
} FlashErrCode;


/* flash.c */
FlashErrCode flash_read(uint32_t flash_address, uint8_t *p_databuf, uint32_t length);
FlashErrCode flash_erase(uint32_t  sector);
FlashErrCode flash_write(uint32_t flash_address, uint8_t *p_databuf, uint32_t length);
void *flash_malloc(size_t size);
void flash_free(void *p);

#endif /* FLASH_H_ */

