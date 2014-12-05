/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_wnet_main.c
 @brief  : this file realize wireless network managment
 @author : wangyifeng
 @history:
           2014-6-17    wangyifeng    Created file
           2014-7-29    wanglei       modified file: process evam msg 
           ...
******************************************************************************/
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"

unsigned int RtmpMACHeaderResvLength(void);
void RtmpSendFrame(unsigned char *pPayload, unsigned int PayloadLen);

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
__align(4) uint8_t wnet_buffer[1024]; /* only a test */


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

int32_t wnet_dataframe_send(rcp_txinfo_t *txinfo, uint8_t *databuf, uint32_t datalen)
{
    uint8_t *pdest;
    uint32_t offset = RtmpMACHeaderResvLength();

    pdest = &wnet_buffer[offset];
    memcpy(pdest, databuf, datalen);
    RtmpSendFrame(pdest, datalen);

    return 0;
}


int32_t wnet_dataframe_recv(uint8_t *databuf, uint32_t datalen)
{
    rcp_rxinfo_t rxinfo;
    
    vam_rcp_recv(&rxinfo, databuf, datalen);

    return 0;
}




