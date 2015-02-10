/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : rt_ate.c
 @brief  : This file include the chip test functions
 @author : wangyifeng
 @history:
           2014-9-28    wangyifeng    Created file
           ...
******************************************************************************/
#include <string.h>
#include "..\include\rt_include.h"
#include "components.h"
#include "cv_wnet.h"

#ifdef WIFI_ATE_MODE

extern RTMP_ADAPTER rtmp_adapter;
extern CHAR system_eeprom_data[];
extern rt_mq_t queue_usbm;

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
UINT  ate_tx_enable = 0;
UINT  ate_rx_enable = 0;
UINT  ate_rx_total_count = 0;
UINT  ate_rx_count = 0;
__align(4) UCHAR ate_tx_data[128];

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
VOID ate_rx_stop(VOID);

VOID ate_tx_frame(VOID)
{
    uint8_t *pdest;
    uint32_t offset = drv_wifi_mac_header_len();

    pdest = &ate_tx_data[offset];
    memset(pdest, 0xAA, 16);
    drv_wifi_send(NULL, pdest, 16);
}

VOID ate_tx_complete(VOID)
{
    if (ate_tx_enable){
        ate_tx_frame();
    }
}

VOID ate_rx_frame(VOID)
{
    if (ate_rx_enable){
        ate_rx_count++;

        if (ate_rx_count < 10){ /* for indicate RX's error  */
           rt_kprintf(":");
        }
        
        if (ate_rx_count >= ate_rx_total_count){
           rt_kprintf("Rx test is interrupt because the TX frame is too much.\n");
           rt_kprintf("Please set the right @TotalCount.\n");
           ate_rx_stop();
        }
    }
}

VOID _ate_rx_start(UCHAR Channel, UINT TotalCount)
{
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)&rtmp_adapter;
    if(!ate_rx_enable){
        rt_kprintf("Channel=%d, TotalCount=%d.\n", \
            Channel, TotalCount);

        if ((Channel>=1)&&(Channel<=14)){
            pAd->CommonCfg.Channel = Channel;
        	AsicSwitchChannel(pAd, Channel, TRUE);
        }

        ate_rx_total_count = TotalCount;
        ate_rx_count = 0;
        ate_rx_enable = TRUE;
    }
}

VOID ate_tx_start(UCHAR Channel, UCHAR Rate, UCHAR Power)
{   
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)&rtmp_adapter;
    if(!ate_tx_enable){
        rt_kprintf("Channel=%d, Rate=%d, Power=%d.\n", \
            Channel, Rate, Power);

        pAd->CommonCfg.TxRate = Rate;
        memset(&system_eeprom_data[EEPROM_G_TX_PWR_OFFSET], Power, 28);
        
        RTMPReadChannelPwr(pAd);
    	AsicSwitchChannel(pAd, Channel, TRUE);

        ate_tx_enable = TRUE;
        ate_tx_frame();
    }
}
FINSH_FUNCTION_EXPORT(ate_tx_start, @Channel:Rate:Power);


VOID ate_tx_stop(VOID)
{
    if (ate_tx_enable){
        ate_tx_enable = FALSE;
    }
}
FINSH_FUNCTION_EXPORT(ate_tx_stop, @none);


//VOID ate_rx_start(UCHAR Channel, UINT TotalCount)
VOID ate_rx_start(UCHAR Channel, UINT TotalCount)
{
#if 0
    rt_err_t err = RT_ENOMEM;
    sys_msg_t msg;
//    UCHAR Channel=6;
//    UINT TotalCount = 10000;
    

    if ((Channel<1)||(Channel>14)){
        rt_kprintf("Channel=%d is invalid, [1~14] is allowed.\n", Channel);
        return;
    }

    if (TotalCount > 65535){
        rt_kprintf("TotalCount=%d is invalid, [1~65535] is allowed.\n", TotalCount);
        return;
    }

    msg.id = ATE_MSG_RX_START;
    msg.argc = (Channel<<16) + (TotalCount&0xFFFF);
    err = rt_mq_send(queue_usbm, &msg, sizeof(sys_msg_t));
    if (err != RT_EOK){
        rt_kprintf("%s: failed=[%d], msg=%04x\n", __FUNCTION__, err, msg.id);
    }
#else
    _ate_rx_start(Channel, TotalCount);
#endif
}
FINSH_FUNCTION_EXPORT(ate_rx_start, @Channel:TotalCount);


VOID ate_rx_stop(VOID)
{
    UINT correct_ratio;
    
    if (ate_rx_enable){
        ate_rx_enable = FALSE;
        
        correct_ratio = ate_rx_count*100/ate_rx_total_count;

        rt_kprintf("Actual rx:count=%d, correct=%d%%.\n\n", \
            ate_rx_count, correct_ratio);
    }
}
FINSH_FUNCTION_EXPORT(ate_rx_stop, @none);

#endif

