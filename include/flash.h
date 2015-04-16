

#ifndef FLASH_H_
#define FLASH_H_


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
FlashErrCode flash_read(uint32_t addr, uint32_t *buf, size_t size);
FlashErrCode flash_erase(uint32_t addr, size_t size);
FlashErrCode flash_write(uint32_t addr, const uint32_t *buf, size_t size);

#endif /* FLASH_H_ */

