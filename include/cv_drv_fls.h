
#ifndef CV_DRV_FLS_H_
#define CV_DRV_FLS_H_

#include "flash.h"
#include "types.h"

/* using CRC32 check when load environment variable from Flash */
#define FLASH_ENV_USING_CRC_CHECK
/* using wear leveling mode or normal mode */
/* #define FLASH_ENV_USING_WEAR_LEVELING_MODE */
#define FLASH_ENV_USING_NORMAL_MODE

/* Flash debug print function. Must be implement by user. */
#define FLASH_DEBUG(...) osal_printf(__FILE__, __LINE__, __VA_ARGS__)
/* Flash routine print function. Must be implement by user. */
#define FLASH_INFO(...) osal_printf(__VA_ARGS__)
/* Flash assert for developer. */
#define FLASH_ASSERT(EXPR)                                                    \
if (!(EXPR))                                                                  \
{                                                                             \
    FLASH_DEBUG("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);     \
    while (1);                                                                \
}

typedef struct _flash_env{
    char *key;
    char *value;
	char *value;
}flash_env, *flash_env_t;



/* flash.c */
FlashErrCode flash_init(void);

/* flash_env.c */
void flash_load_env(void);
void flash_print_env(void);
char *flash_get_env(const char *key);
FlashErrCode flash_set_env(const char *key, const char *value);
FlashErrCode flash_save_env(void);
FlashErrCode flash_env_set_default(void);
uint32_t flash_get_env_total_size(void);
uint32_t flash_get_env_used_size(void);

#endif
