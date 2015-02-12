/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_osal_debug.c
 @brief  : This is the global debug realization.
 @author : wangyf
 @history:
           2014-11-13    wangyf    Created file
           ...
******************************************************************************/
#include "cv_osal.h"
#include "cv_osal_dbg.h"
#include "components.h"

#if (OSAL_GLOBAL_DEBUG)
/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
/**
 *  [block begin] Debug level configuration
 *  You can add your module here and DONOT modify any other code in the block.
 */
const debug_entry_t debug_entry_table_start = {NULL, NULL};
OSAL_DEBUG_ENTRY_DECLARE(sysc)
OSAL_DEBUG_ENTRY_DECLARE(vam)
OSAL_DEBUG_ENTRY_DECLARE(vsa)
OSAL_DEBUG_ENTRY_DECLARE(voc)


// add your module here...

const debug_entry_t debug_entry_table_end = {NULL, NULL};
/**
 *  [block end]
 */

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
int osal_dbg_set_level(char *module, int level)
{
	debug_entry_t *cmdtp;

	/* Search command table - Use linear search - it's a small table */
	for (cmdtp = (debug_entry_t *)(&debug_entry_table_start + 1); cmdtp->module; cmdtp++) {
		if (strcmp (module, cmdtp->module) == 0) {
		    if (cmdtp->handler){
                (cmdtp->handler)(level);
                return TRUE;
		    }
		}
	}
    return FALSE;
}

void osal_dbg_dump_data(uint8_t *p, uint32_t len)
{
    int i;
    #define LINE_WIDTH 16

    if (len > 0){
        osal_printf("====================== dump data ======================\n");
        osal_printf(" Addr| ");

        for (i=0; i<LINE_WIDTH; i++) {
            osal_printf("%02x ", i);
        }
        osal_printf("\n -----------------------------------------------------");
        
        for (i=0;i<len;i++) {
            if ((i%LINE_WIDTH) == 0) {
                osal_printf("\n %04x| ", i);
            }
            osal_printf("%02x ", *(p+i));
        }
        osal_printf("\n======================== end ==========================\n");
    }
}

#ifdef OS_RT_THREAD
/**
 * Export to finsh of RT-thread
 */
void debug(char *module, int level)
{
	if (osal_dbg_set_level(module, level)) {
        osal_printf("success.\n");
	}
	else {
        osal_printf("cannot find module \"%s\" !\n", module);
	}
}
FINSH_FUNCTION_EXPORT(debug, 0-off 1-err 2-warn 3-info 4-trace 5-loud)

void dump(uint32_t addr, uint32_t len)
{
    osal_dbg_dump_data((uint8_t *)addr, len);
}
FINSH_FUNCTION_EXPORT(dump, printf the raw data)
#endif /* OS_RT_THREAD */
#endif /* OSAL_GLOBAL_DEBUG */
