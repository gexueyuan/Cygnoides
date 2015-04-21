#ifndef FLASH_H_
#define FLASH_H_
#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "param"
#include "cv_osal_dbg.h"

#include "types.h"
#include "cv_sys_param.h"
/* using CRC32 check when load environment variable from Flash */
//#define FLASH_ENV_USING_CRC_CHECK
/* the user setting size of ENV, must be word alignment */
#define FLASH_USER_SETTING_ENV_SIZE     (2 * 1024)                /* default 2K */
/* using wear leveling mode or normal mode */
/* #define FLASH_ENV_USING_WEAR_LEVELING_MODE */
#define FLASH_ENV_USING_NORMAL_MODE

/* Flash debug print function. Must be implement by user. */
#define FLASH_DEBUG(...) OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, __VA_ARGS__)
/* Flash routine print function. Must be implement by user. */
#define FLASH_INFO(...) osal_printf(__VA_ARGS__)
/* Flash assert for developer. */
#define FLASH_ASSERT(EXPR)                                                    \
if (!(EXPR))                                                                  \
{                                                                             \
    FLASH_DEBUG("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);      \
    while (1);                                                                \
}

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
FlashErrCode flash_init(void);
FlashErrCode flash_port_init(uint32_t *env_addr, size_t *env_total_size, size_t *erase_min_size,
        flash_param const **default_env, size_t *default_env_size);


/* flash_env.c flash_env_wl.c */
void flash_load_env(void);
void flash_print_env(void);
char *flash_get_env(const char *key);
FlashErrCode flash_set_env(const char *key, const char *value);
FlashErrCode flash_save_env(void);
FlashErrCode flash_env_set_default(void);
size_t flash_get_env_total_size(void);
size_t flash_get_env_write_bytes(void);



/* flash_port.c */
FlashErrCode flash_read(uint32_t addr, uint32_t *buf, size_t size);
FlashErrCode flash_erase(uint32_t addr, size_t size);
FlashErrCode flash_write(uint32_t addr, const uint32_t *buf, size_t size);
void flash_print(const char *format, ...);

#endif /* FLASH_H_ */




