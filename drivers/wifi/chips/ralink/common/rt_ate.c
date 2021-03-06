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

extern RTMP_ADAPTER rtmp_adapter;
extern CHAR system_eeprom_data[];
extern rt_mq_t queue_usbm;

extern int drv_wifi_mac_header_len(void);
extern int drv_wifi_send(wnet_txinfo_t *txinfo, uint8_t *pdata, int32_t length);
extern VOID RTMPReadChannelPwr(IN PRTMP_ADAPTER pAd);

#ifdef WIFI_ATE_MODE

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

VOID ate_rx_stop(VOID)
{
    if (ate_rx_enable){
        UINT correct_ratio;
        ate_rx_enable = FALSE;
        
        correct_ratio = ate_rx_count*100/ate_rx_total_count;

        rt_kprintf("Actual rx:count=%d, correct=%d%%.\n\n", \
            ate_rx_count, correct_ratio);
    }
}

VOID ate_rx_frame(PRTMP_ADAPTER pAd, PUCHAR pData, ULONG RxBufferLength)
{
    PRXINFO_STRUC  pRxD;
    ULONG ThisFrameLen;
    BOOLEAN Stoped = 0;

    if (!ate_rx_enable){
        return;
    }

    ThisFrameLen = *pData + (*(pData+1)<<8);
    if ((ThisFrameLen == 0)||((ThisFrameLen&0x3) != 0)) {        
         goto exit;
    }   

    if ((ThisFrameLen + 8)> RxBufferLength) {
         goto exit;
    }

    pRxD = (PRXINFO_STRUC)(pData + RT2870_RXDMALEN_FIELD_SIZE + ThisFrameLen);

    if (pRxD->Crc) {
        //rt_kprintf("Crc Error\n");
        goto exit;
    }

    if (++ate_rx_count >= ate_rx_total_count) {
       rt_kprintf("Rx test is interrupt because the TX frame is too much.\n");
       rt_kprintf("Please set the right @TotalCount.\n");
       ate_rx_stop();
       Stoped = 1;
    }

exit:
    if (!Stoped) {
        usb_bulkin(pAd->pUsb_Dev);
    }
}

VOID ate_tx(UCHAR Channel, UCHAR Rate, UCHAR Power)
{   
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)&rtmp_adapter;
    if(!ate_tx_enable){
        rt_kprintf("Channel=%d, Rate=%d, Power=%d.\n", \
            Channel, Rate, Power);

        pAd->CommonCfg.TxRate = Rate;
        memset(&system_eeprom_data[EEPROM_G_TX_PWR_OFFSET], Power, 56);
        
        RTMPReadChannelPwr(pAd);
    	AsicSwitchChannel(pAd, Channel, TRUE);

        ate_tx_enable = TRUE;
        ate_tx_frame();
    }
}
FINSH_FUNCTION_EXPORT(ate_tx, @Channel:@Rate:@Power);

VOID ate_rx(UCHAR Channel, UINT TotalCount, UCHAR LNAGain)
{
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)&rtmp_adapter;
    if(!ate_rx_enable){
        rt_kprintf("Channel=%d, TotalCount=%d, LNAGain=%d.\n", \
            Channel, TotalCount, LNAGain);

        pAd->BLNAGain = LNAGain;

        if ((Channel>=1)&&(Channel<=14)){
            pAd->CommonCfg.Channel = Channel;
        	AsicSwitchChannel(pAd, Channel, TRUE);
        }

        ate_rx_total_count = TotalCount;
        ate_rx_count = 0;
        ate_rx_enable = TRUE;

        usb_bulkin(pAd->pUsb_Dev);
    }

}
FINSH_FUNCTION_EXPORT(ate_rx, @Channel:@TotalCount:@LNAGain);

VOID ate_stop(VOID)
{
    if (ate_tx_enable){
        ate_tx_enable = FALSE;
    }

    if (ate_rx_enable){
        ate_rx_stop();
    }
}
FINSH_FUNCTION_EXPORT(ate_stop, Stop tx/rx test);

#if 0
UCHAR ate_macaddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
VOID ate_setaddr(UCHAR addr0, UCHAR addr1, UCHAR addr2, UCHAR addr3, UCHAR addr4, UCHAR addr5)
{
    ate_macaddr[0] = addr5;
    ate_macaddr[1] = addr4;
    ate_macaddr[2] = addr3;
    ate_macaddr[3] = addr2;
    ate_macaddr[4] = addr1;
    ate_macaddr[5] = addr0;
}
FINSH_FUNCTION_EXPORT(ate_setaddr, Set mac address for ATE);
#endif

#endif

