/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_vam_rcp.c
 @brief  : this file realize the vehicle Remote Communicat Protocol
 @author : wangyifeng
 @history:
           2014-6-17    wangyifeng    Created file
           2014-7-29    wanglei       modified file: process evam msg 
           ...
******************************************************************************/
#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "rcp"
#include "cv_osal_dbg.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"
#include "cv_wnet.h"


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
itis_codes_t itiscode[RSA_TYPE_MAX+1] = 
{
#undef xx
#define xx(SEQ, TYPE, ITISCODE) (ITISCODE),
    VAM_RSA_TYPE_2_ITISCODE
};

double rcp_dbg_distance = 0;

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/


__COMPILE_INLINE__ int32_t encode_longtitude(float x)
{
    int32_t r;
    r = (int32_t)(x*10000000.0f);
    return cv_ntohl(r);
}


__COMPILE_INLINE__ float decode_longtitude(uint32_t x)
{
    float r;
    r = ((int32_t)cv_ntohl(x)) / 10000000.0f;
    return (float)r;
}

#define encode_latitude(x) encode_longtitude(x) 
#define decode_latitude(x) decode_longtitude(x) 

#define encode_accuracy(x) encode_longtitude(x) 
#define decode_accuracy(x) decode_longtitude(x) 

__COMPILE_INLINE__ uint16_t encode_elevation(float x)
{
    return cv_ntohs((uint16_t)x);
}

__COMPILE_INLINE__ float decode_elevation(uint16_t x)
{
    return (float)cv_ntohs(x);
}

__COMPILE_INLINE__ uint16_t encode_speed(float x)
{
    uint16_t r;
    r = (uint16_t)(x*100000.0f/3600);
    return cv_ntohs(r);
}


__COMPILE_INLINE__ float decode_speed(uint16_t x)
{
    float r;
    r = cv_ntohs(x)*3600 / 100000.0f;
    return (float)r;
}

__COMPILE_INLINE__ uint16_t encode_heading(float x)
{
    double r;
    r = x*80.0f;
    return cv_ntohs((uint16_t)r);
}

__COMPILE_INLINE__ float decode_heading(uint16_t x)
{
    double r;
    r = cv_ntohs(x)/80.0;
    return (float)r;
}

__COMPILE_INLINE__ uint16_t encode_acce_lon(float x)
{
    return cv_ntohs((uint16_t)x);
}

__COMPILE_INLINE__ float decode_acce_lon(uint16_t x)
{
    return (float)cv_ntohs(x);
}

__COMPILE_INLINE__ uint16_t encode_acce_lat(float x)
{
    return cv_ntohs((uint16_t)x);
}

__COMPILE_INLINE__ float decode_acce_lat(uint16_t x)
{
    return (float)cv_ntohs(x);
}

__COMPILE_INLINE__ uint16_t encode_acce_vert(float x)
{
    return cv_ntohs((uint16_t)x);
}

__COMPILE_INLINE__ float decode_acce_vert(uint16_t x)
{
    return (float)cv_ntohs(x);
}

__COMPILE_INLINE__ uint8_t encode_acce_yaw(float x)
{
    return (uint8_t)(x);
}

__COMPILE_INLINE__ float decode_acce_yaw(uint8_t x)
{
    return (float)(x);
}


__COMPILE_INLINE__ uint16_t encode_vehicle_alert(uint16_t x)
{
    uint16_t r = 0;
    if (x & VAM_ALERT_MASK_VBD){
        r |= EventHazardLights;        
    }
    else {
        r &= ~EventHazardLights;
    }

    if (x & VAM_ALERT_MASK_EBD){
        r |= EventHardBraking;        
    }
    else {
        r &= ~EventHardBraking;
    }
    
    if (x & VAM_ALERT_MASK_VOT){
        r |= EventDisabledVehicle;
    }
    else {
        r &= ~EventDisabledVehicle;
    }

    return cv_ntohs(r);
}

__COMPILE_INLINE__ uint16_t decode_vehicle_alert(uint16_t x)
{
    uint16_t r = 0;
    x = cv_ntohs(x);
    if (x & EventHazardLights) {
        r |= VAM_ALERT_MASK_VBD;        
    }
   
    if (x & EventHardBraking){
        r |= VAM_ALERT_MASK_EBD;        
    }

    if (x & EventDisabledVehicle){
        r |= VAM_ALERT_MASK_VOT;        
    }
    return r;
}

/* BEGIN: Added by wanglei, 2015/1/4. for rsa test */
__COMPILE_INLINE__ uint16_t encode_itiscode(uint16_t rsa_mask, __packed itis_codes_t *p_des)
{
    uint16_t r;
    int bit;

    for(bit=0; bit<9; bit++)
    {
        if(rsa_mask & (1<<bit)){
            if (0 == bit){
                r = cv_ntohs(itiscode[bit]);
            }
            else{
                p_des[bit-1] = cv_ntohs(itiscode[bit]);
            }
        }
    }
    
    return r;
}

__COMPILE_INLINE__ uint16_t decode_itiscode(itis_codes_t typeEvent, __packed itis_codes_t *p_des)
{
    uint16_t k, i, r, rsa_mask;
    r = cv_ntohs(typeEvent);
    goto getbitmask;
    
    for(k=0; k<8; k++)
    {
        r = cv_ntohs(p_des[k]);
getbitmask:
        if (r)
        {
            for (i=0; i<RSA_TYPE_MAX; i++)
            {
                if (itiscode[i] == r){
                   rsa_mask |= 1<<i;
                   break;
                }
            }
        }
    }
    return rsa_mask;
}

int rcp_mda_process(uint8_t msg_hops, 
                      uint8_t msg_count,
                      uint8_t *p_temp_id, 
                      uint8_t * data,
                      uint32_t datalen)
{
    mda_msg_info_t src;
    mda_envar_t * p_mda;
    int ret;

    p_mda = &p_cms_envar->mda;
    src.left_hops = msg_hops - 1;
    src.msg_count = msg_count;
    memcpy(src.temorary_id, p_temp_id, RCP_TEMP_ID_LEN);
    
    ret = mda_handle(p_mda, &src, NULL, data, datalen);
    return ret;
}

/* END:   Added by wanglei, 2015/1/4 */

int16_t rcp_get_system_time(void)
{
    return osal_get_systemtime();
}

int rcp_parse_bsm(vam_envar_t *p_vam,
                  wnet_rxinfo_t *rxinfo,
                  uint8_t *databuf, 
                  uint32_t datalen)
{
    vam_sta_node_t *p_sta;
    rcp_msg_basic_safty_t *p_bsm;
    uint16_t alert_mask;  

    if (datalen < (sizeof(rcp_msg_basic_safty_t) - sizeof(vehicle_safety_ext_t))){
        return -1;
    }

    p_bsm = (rcp_msg_basic_safty_t *)databuf;
    if (0 == memcmp(p_bsm->header.temporary_id, p_vam->local.pid, RCP_TEMP_ID_LEN)){
        return 0;
    }
    
    rcp_mda_process(p_bsm->header.msg_id.hops, p_bsm->header.msg_count, 
                     p_bsm->header.temporary_id, databuf, datalen);

    p_sta = vam_find_sta(p_vam, p_bsm->header.temporary_id);

    if (p_sta){
        p_sta->life = VAM_NEIGHBOUR_MAXLIFE;
        p_sta->s.timestamp = p_bsm->dsecond;
        
        /* BEGIN: Added by wanglei, 2014/10/16 */
        p_sta->s.time = osal_get_systemtime();
        /* END:   Added by wanglei, 2014/10/16 */

        p_sta->s.pos.lon = decode_longtitude(p_bsm->position.lon);
        p_sta->s.pos.lat = decode_latitude(p_bsm->position.lat);
        p_sta->s.pos.elev = decode_elevation(p_bsm->position.elev);
        p_sta->s.pos.accu = decode_accuracy(p_bsm->position.accu);

        p_sta->s.dir = decode_heading(p_bsm->motion.heading);
        p_sta->s.speed = decode_speed(p_bsm->motion.speed);
        p_sta->s.acce.lon = decode_acce_lon(p_bsm->motion.acce.lon);
        p_sta->s.acce.lat = decode_acce_lat(p_bsm->motion.acce.lat);
        p_sta->s.acce.vert = decode_acce_vert(p_bsm->motion.acce.vert);
        p_sta->s.acce.yaw = decode_acce_yaw(p_bsm->motion.acce.yaw);

        //dump_pos(&p_sta->s);
        /* for test  */
        rcp_dbg_distance = vsm_get_distance(&p_vam->local.pos, &p_sta->s.pos); 

        if (p_vam->evt_handler[VAM_EVT_PEER_UPDATE]){
            (p_vam->evt_handler[VAM_EVT_PEER_UPDATE])(&p_sta->s);
        }

        if(datalen > sizeof(rcp_msg_basic_safty_t) - sizeof(vehicle_safety_ext_t)){
            p_sta->alert_life = VAM_REMOTE_ALERT_MAXLIFE;
            alert_mask = decode_vehicle_alert(p_bsm->safetyExt.events);
            p_sta->s.alert_mask = alert_mask;
            /* inform the app layer once */
            if (p_vam->evt_handler[VAM_EVT_PEER_ALARM]){
                (p_vam->evt_handler[VAM_EVT_PEER_ALARM])(&p_sta->s);
            }        
        }

        
    }

    return 0;
}
int rcp_parse_evam(vam_envar_t *p_vam,
                  wnet_rxinfo_t *rxinfo,
                  uint8_t *databuf, 
                  uint32_t datalen)
{
    vam_sta_node_t *p_sta;
    rcp_msg_emergency_vehicle_alert_t *p_evam;
    uint16_t alert_mask;

    if (datalen < sizeof(rcp_msg_emergency_vehicle_alert_t)){
        return -1;
    }

    p_evam = (rcp_msg_emergency_vehicle_alert_t *)databuf;

    if (0 == memcmp(p_evam->temporary_id, p_vam->local.pid, RCP_TEMP_ID_LEN)){
        return 0;
    }
    
    rcp_mda_process(p_evam->msg_id.hops, p_evam->rsa.msg_count, 
                     p_evam->temporary_id, databuf, datalen);


    //TBD
    alert_mask = decode_itiscode(p_evam->rsa.typeEvent, p_evam->rsa.description);
    //rt_kprintf("recv evam: alert_mask = 0x%04x\r\n", alert_mask);

    p_sta = vam_find_sta(p_vam, p_evam->temporary_id);
    if (p_sta){
        p_sta->alert_life = VAM_REMOTE_ALERT_MAXLIFE;

        /* BEGIN: Added by wanglei, 2014/10/16 */
        p_sta->s.time = osal_get_systemtime();
        /* END:   Added by wanglei, 2014/10/16 */

        p_sta->s.pos.lon = decode_longtitude(p_evam->rsa.position.lon);
        p_sta->s.pos.lat = decode_latitude(p_evam->rsa.position.lat);
        p_sta->s.pos.elev = decode_elevation(p_evam->rsa.position.elev);

        p_sta->s.dir = decode_heading(p_evam->rsa.position.heading);
        p_sta->s.speed = decode_speed(p_evam->rsa.position.speed.speed);
#if 0
        p_sta->s.acce.lon = decode_acce_lon(p_evam->motion.acce.lon);
        p_sta->s.acce.lat = decode_acce_lat(p_evam->motion.acce.lat);
        p_sta->s.acce.vert = decode_acce_vert(p_evam->motion.acce.vert);
        p_sta->s.acce.yaw = decode_acce_yaw(p_evam->motion.acce.yaw);     
#endif
        p_sta->s.alert_mask = alert_mask;

        /* inform the app layer once */
        if (p_vam->evt_handler[VAM_EVT_EVA_UPDATE]){
            (p_vam->evt_handler[VAM_EVT_EVA_UPDATE])(&p_sta->s);
        }
    }
    return 0;
}


int rcp_parse_rsa(vam_envar_t *p_vam,
                  wnet_rxinfo_t *rxinfo,
                  uint8_t *databuf, 
                  uint32_t datalen)
{
    rcp_msg_roadside_alert_t *p_rsa;
    vam_rsa_evt_info_t param;
        
    if (datalen < sizeof(rcp_msg_roadside_alert_t)){
        return -1;
    }

    p_rsa = (rcp_msg_roadside_alert_t *)databuf;

    param.rsa_mask = decode_itiscode(p_rsa->typeEvent, p_rsa->description);
    param.pos.lon = decode_longtitude(p_rsa->position.lon);
    param.pos.lat = decode_longtitude(p_rsa->position.lat);

    if (p_vam->evt_handler[VAM_EVT_RSA_UPDATE]){
        (p_vam->evt_handler[VAM_EVT_RSA_UPDATE])(&param);
    }

    return 0;
}


int rcp_parse_msg(vam_envar_t *p_vam,
                  wnet_rxinfo_t *rxinfo, 
                  uint8_t *databuf, 
                  uint32_t datalen)
{
    rcp_msgid_t *p_msgid;

    if (datalen < sizeof(rcp_msg_head_t)){
        return -1;
    }

    p_msgid = (rcp_msgid_t *)databuf;

    switch(p_msgid->id){
    case RCP_MSG_ID_BSM:
        rcp_parse_bsm(p_vam, rxinfo, databuf, datalen);
        break;
#ifndef RSU_TEST
    case RCP_MSG_ID_EVAM:
        /* receive evam, then pause sending bsm msg */
        if(2 == p_vam->working_param.bsm_pause_mode)
        {
            vsm_pause_bsm_broadcast(p_vam);
        }
        rcp_parse_evam(p_vam, rxinfo, databuf, datalen);
        break;
    case RCP_MSG_ID_RSA:
        rcp_parse_rsa(p_vam, rxinfo, databuf, datalen);
        break;
#endif    
    default:
        break;
    }

    return p_msgid->id;
}


/*****************************************************************************
 @funcname: vam_rcp_recv
 @brief   : RCP receive data frame from network layer
 @param   : wnet_rxinfo_t *rxinfo  
 @param   : uint8_t *databuf      
 @param   : uint32_t datalen      
 @return  : 
*****************************************************************************/
int vam_rcp_recv(wnet_rxinfo_t *rxinfo, uint8_t *databuf, uint32_t datalen)
{
    vam_envar_t *p_vam = &p_cms_envar->vam;

    vam_add_event_queue(p_vam, VAM_MSG_RCPRX, datalen, (uint32_t)databuf, rxinfo);
    return 0;
}


int32_t rcp_send_bsm(vam_envar_t *p_vam)
{
    rcp_msg_basic_safty_t *p_bsm;
    vam_stastatus_t *p_local = &p_vam->local;
    wnet_txbuf_t *txbuf;
    wnet_txinfo_t *txinfo;
    vam_stastatus_t current;
	int len = sizeof(rcp_msg_basic_safty_t);
	
    txbuf = wnet_get_txbuf();
    if (txbuf == NULL) {
        return -1;
    }

    vam_get_local_current_status(&current);
    p_local = &current;

    p_bsm = (rcp_msg_basic_safty_t *)WNET_TXBUF_DATA_PTR(txbuf);

    p_bsm->header.msg_id.hops = p_vam->working_param.bsm_hops;
    p_bsm->header.msg_id.id = RCP_MSG_ID_BSM;

    p_bsm->header.msg_count = p_vam->tx_bsm_msg_cnt++;
    memcpy(p_bsm->header.temporary_id, p_local->pid, RCP_TEMP_ID_LEN);
    p_bsm->dsecond = rcp_get_system_time();

    p_bsm->position.lon = encode_longtitude(p_local->pos.lon);
    p_bsm->position.lat = encode_latitude(p_local->pos.lat);
    p_bsm->position.elev = encode_elevation(p_local->pos.elev);
    p_bsm->position.accu = encode_accuracy(p_local->pos.accu);

    p_bsm->motion.heading = encode_heading(p_local->dir);
    p_bsm->motion.speed = encode_speed(p_local->speed);
    p_bsm->motion.acce.lon = encode_acce_lon(p_local->acce.lon);
    p_bsm->motion.acce.lat = encode_acce_lat(p_local->acce.lat);
    p_bsm->motion.acce.vert = encode_acce_vert(p_local->acce.vert);
    p_bsm->motion.acce.yaw = encode_acce_yaw(p_local->acce.yaw);

    if(p_vam->flag & VAM_FLAG_TX_BSM_ALERT)
    {
        /* need to send part2 safetyextenrion */
        p_bsm->safetyExt.events = encode_vehicle_alert(p_vam->local.alert_mask);
    }
    else
    {
        len -= sizeof(vehicle_safety_ext_t);
    }


    txinfo = WNET_TXBUF_INFO_PTR(txbuf);
    memset(txinfo, 0, sizeof(wnet_txinfo_t));
    memcpy(txinfo->dest.dsmp.addr, "\xFF\xFF\xFF\xFF\xFF\xFF", MACADDR_LENGTH);
    txinfo->dest.dsmp.aid = 0x00000020;
    txinfo->protocol = WNET_TRANS_PROT_DSMP;
    txinfo->encryption = WNET_TRANS_ENCRYPT_NONE;
    txinfo->prority = WNET_TRANS_RRORITY_NORMAL;
    txinfo->timestamp = osal_get_systemtime();

    return wnet_send(txinfo, (uint8_t *)p_bsm, len);
}

int32_t rcp_send_evam(vam_envar_t *p_vam)
{
    rcp_msg_emergency_vehicle_alert_t *p_evam;
    vam_stastatus_t *p_local = &p_vam->local;
    wnet_txbuf_t *txbuf;
    wnet_txinfo_t *txinfo;
    vam_stastatus_t current;
	
    txbuf = wnet_get_txbuf();
    if (txbuf == NULL) {
        return -1;
    }

    vam_get_local_current_status(&current);
    p_local = &current;

    p_evam = (rcp_msg_emergency_vehicle_alert_t *)WNET_TXBUF_DATA_PTR(txbuf);

    p_evam->msg_id.hops = p_vam->working_param.evam_hops;
    p_evam->msg_id.id = RCP_MSG_ID_EVAM;
    memcpy(p_evam->temporary_id, p_local->pid, RCP_TEMP_ID_LEN);

    p_evam->rsa.msg_count = p_vam->tx_evam_msg_cnt++;
    p_evam->rsa.position.lon = encode_longtitude(p_local->pos.lon);
    p_evam->rsa.position.lat = encode_latitude(p_local->pos.lat);
    p_evam->rsa.position.elev = encode_elevation(p_local->pos.elev);
    p_evam->rsa.position.heading = encode_heading(p_local->dir);
    p_evam->rsa.position.speed.transmissionState = TRANS_STATE_Forward;
    p_evam->rsa.position.speed.speed = encode_speed(p_local->speed);  
    //TBD
    p_evam->rsa.typeEvent = encode_itiscode(p_local->alert_mask, p_evam->rsa.description); 
    

    txinfo = WNET_TXBUF_INFO_PTR(txbuf);
    memset(txinfo, 0, sizeof(wnet_txinfo_t));
    memcpy(txinfo->dest.dsmp.addr, "\xFF\xFF\xFF\xFF\xFF\xFF", MACADDR_LENGTH);
    txinfo->dest.dsmp.aid = 0x00000020;
    txinfo->protocol = WNET_TRANS_PROT_DSMP;
    txinfo->encryption = WNET_TRANS_ENCRYPT_NONE;
    txinfo->prority = WNET_TRANS_RRORITY_EMERGENCY;
    txinfo->timestamp = osal_get_systemtime();

    return wnet_send(txinfo, (uint8_t *)p_evam, sizeof(rcp_msg_emergency_vehicle_alert_t));
}



int32_t rcp_send_rsa(vam_envar_t *p_vam)
{
    rcp_msg_roadside_alert_t *p_rsa;
    vam_stastatus_t *p_local = &p_vam->local;
    wnet_txbuf_t *txbuf;
    wnet_txinfo_t *txinfo;

    txbuf = wnet_get_txbuf();
    if (txbuf == NULL) {
        return -1;
    }

    /* The RSU position is fixed */
#if 0
    vam_stastatus_t current;
    vam_get_local_current_status(&current);
    p_local = &current;
#endif

    p_rsa = (rcp_msg_roadside_alert_t *)WNET_TXBUF_DATA_PTR(txbuf);

    p_rsa->msg_id.hops = p_vam->working_param.bsm_hops;
    p_rsa->msg_id.id = RCP_MSG_ID_RSA;
    p_rsa->msg_count = p_vam->tx_rsa_msg_cnt++;

    p_rsa->typeEvent = encode_itiscode(p_local->alert_mask, p_rsa->description);
        
    p_rsa->position.lon = encode_longtitude(p_local->pos.lon);
    p_rsa->position.lat = encode_latitude(p_local->pos.lat);
    p_rsa->position.elev = encode_elevation(p_local->pos.elev);
    p_rsa->position.heading = encode_heading(p_local->dir);
    p_rsa->position.speed.speed = encode_speed(p_local->speed);

    txinfo = WNET_TXBUF_INFO_PTR(txbuf);
    memset(txinfo, 0, sizeof(wnet_txinfo_t));
    memcpy(txinfo->dest.dsmp.addr, "\xFF\xFF\xFF\xFF\xFF\xFF", MACADDR_LENGTH);
    txinfo->dest.dsmp.aid = 0x00000020;
    txinfo->protocol = WNET_TRANS_PROT_DSMP;
    txinfo->encryption = WNET_TRANS_ENCRYPT_NONE;
    txinfo->prority = WNET_TRANS_RRORITY_EMERGENCY;
    txinfo->timestamp = osal_get_systemtime();

    return wnet_send(txinfo, (uint8_t *)p_rsa, sizeof(rcp_msg_roadside_alert_t));
}


int rcp_send_forward_msg(wnet_txbuf_t *txbuf)
{
    wnet_txinfo_t *txinfo;

    txinfo = WNET_TXBUF_INFO_PTR(txbuf);
    memset(txinfo, 0, sizeof(wnet_txinfo_t));
    memcpy(txinfo->dest.dsmp.addr, "\xFF\xFF\xFF\xFF\xFF\xFF", MACADDR_LENGTH);
    txinfo->dest.dsmp.aid = 0x00000020;
    txinfo->protocol = WNET_TRANS_PROT_DSMP;
    txinfo->encryption = WNET_TRANS_ENCRYPT_NONE;
    txinfo->prority = WNET_TRANS_RRORITY_EMERGENCY;
    txinfo->timestamp = osal_get_systemtime();

    return wnet_send(txinfo, WNET_TXBUF_DATA_PTR(txbuf), txbuf->data_len);
}
wnet_txbuf_t *rcp_create_forward_msg(uint8_t left_hops, uint8_t *pdata, uint32_t length)
{
    rcp_msgid_t *p_msg;
    wnet_txbuf_t *txbuf;

    p_msg = (rcp_msgid_t *)pdata;
    p_msg->hops = left_hops;

    txbuf = wnet_get_txbuf();
    if (txbuf == NULL) {
        return NULL;
    }

    memcpy(WNET_TXBUF_DATA_PTR(txbuf), pdata, length);
    txbuf->data_len = length;

    
    return txbuf;
}


//////////////////////////////////////////////////////////////
//all below just for test
//////////////////////////////////////////////////////////////

void timer_send_rsa_callback(void* parameter)
{
    vam_envar_t *p_vam = (vam_envar_t *)parameter;   
    rcp_send_rsa(p_vam);
}
void test_rsa(int flag)
{
    vam_envar_t *p_vam = &p_cms_envar->vam;
    osal_printf("rsatype = %d , %d\r\n", RSA_TYPE_SPEED_RESTRICTION, RSA_TYPE_MAX);
    if(flag && !p_vam->timer_send_rsa){
        vam_stop();  
        p_vam->timer_send_rsa = osal_timer_create("tm_rsa", timer_send_rsa_callback, p_vam, SECOND_TO_TICK(1), 0x2);
        osal_timer_start(p_vam->timer_send_rsa);
    }
    else{
        if(p_vam->timer_send_rsa){
            osal_timer_stop(p_vam->timer_send_rsa);
        }
    }   
}
FINSH_FUNCTION_EXPORT(test_rsa, debug: test sending rsa);


#if 0
rcp_msg_basic_safty_t test_bsm_rx;
rcp_rxinfo_t test_rxbd;
rt_timer_t timer_test_bsm_rx;

rcp_msg_basic_safty_t test_bsm_rx_2;
rcp_rxinfo_t test_rxbd_2;
rt_timer_t timer_test_bsm_rx_2;

rcp_msg_basic_safty_t test_bsm_rx_3;
rcp_rxinfo_t test_rxbd_3;
rt_timer_t timer_test_bsm_rx_3;

void timer_test_bsm_rx_callback(void* parameter)
{
    vam_rcp_recv(&test_rxbd, (uint8_t *)&test_bsm_rx, sizeof(rcp_msg_basic_safty_t));
}

void timer_test_bsm_rx_callback_2(void* parameter)
{
    vam_rcp_recv(&test_rxbd_2, (uint8_t *)&test_bsm_rx_2, sizeof(rcp_msg_basic_safty_t));
}

void timer_test_bsm_rx_callback_3(void* parameter)
{
    vam_rcp_recv(&test_rxbd_3, (uint8_t *)&test_bsm_rx_3, sizeof(rcp_msg_basic_safty_t));
}

void test_bsm(void)
{
    rcp_msg_basic_safty_t *p_bsm = &test_bsm_rx;
    vam_stastatus_t sta;
    vam_stastatus_t *p_local = &sta;

    #if 1 
    memset(p_local, 0, sizeof(vam_stastatus_t));
    p_local->pos.lat = 40.0; //39.5427f;
    p_local->pos.lon = 120.0;//116.2317f;
    p_local->dir = 90.0;//
    #else
    memcpy(p_local, &p_cms_envar->vam.local, sizeof(vam_stastatus_t));
    #endif
    p_local->pid[0] = 0x02;
    p_local->pid[1] = 0x04;
    p_local->pid[2] = 0x06;
    p_local->pid[3] = 0x08;
    
    /* construct a fake message */
    p_bsm->header.msg_id = RCP_MSG_ID_BSM;
    p_bsm->header.msg_count = 0;
    memcpy(p_bsm->header.temporary_id, p_local->pid, RCP_TEMP_ID_LEN);
    p_bsm->header.dsecond = rcp_get_system_time();

    p_bsm->position.lon = encode_longtitude(p_local->pos.lon);
    p_bsm->position.lat = encode_latitude(p_local->pos.lat);
    p_bsm->position.elev = encode_elevation(p_local->pos.elev);
    p_bsm->position.accu = encode_accuracy(p_local->pos.accu);

    p_bsm->motion.heading = encode_heading(p_local->dir);
    p_bsm->motion.speed = encode_speed(p_local->speed);
    p_bsm->motion.acce.lon = encode_acce_lon(p_local->acce.lon);
    p_bsm->motion.acce.lat = encode_acce_lat(p_local->acce.lat);
    p_bsm->motion.acce.vert = encode_acce_vert(p_local->acce.vert);
    p_bsm->motion.acce.yaw = encode_acce_yaw(p_local->acce.yaw);

    //dump((uint8_t *)p_bsm, sizeof(rcp_msg_basic_safty_t));

    timer_test_bsm_rx = rt_timer_create("tm-tb",timer_test_bsm_rx_callback,NULL,\
        MS_TO_TICK(2400),RT_TIMER_FLAG_PERIODIC); 					

    rt_timer_start(timer_test_bsm_rx);
}


void tb2(void)
{
    rcp_msg_basic_safty_t *p_bsm = &test_bsm_rx_2;
    vam_stastatus_t sta;
    vam_stastatus_t *p_local = &sta;

    #if 1 
    memset(p_local, 0, sizeof(vam_stastatus_t));
    p_local->pos.lat = 40.0; //39.5427f;
    p_local->pos.lon = 120.1;//116.2317f;
    p_local->dir = 90.0;//
    #else
    memcpy(p_local, &p_cms_envar->vam.local, sizeof(vam_stastatus_t));
    #endif
    p_local->pid[0] = 0x01;
    p_local->pid[1] = 0x03;
    p_local->pid[2] = 0x05;
    p_local->pid[3] = 0x07;
    
    /* construct a fake message */
    p_bsm->header.msg_id = RCP_MSG_ID_BSM;
    p_bsm->header.msg_count = 0;
    memcpy(p_bsm->header.temporary_id, p_local->pid, RCP_TEMP_ID_LEN);
    p_bsm->header.dsecond = rcp_get_system_time();

    p_bsm->position.lon = encode_longtitude(p_local->pos.lon);
    p_bsm->position.lat = encode_latitude(p_local->pos.lat);
    p_bsm->position.elev = encode_elevation(p_local->pos.elev);
    p_bsm->position.accu = encode_accuracy(p_local->pos.accu);

    p_bsm->motion.heading = encode_heading(p_local->dir);
    p_bsm->motion.speed = encode_speed(p_local->speed);
    p_bsm->motion.acce.lon = encode_acce_lon(p_local->acce.lon);
    p_bsm->motion.acce.lat = encode_acce_lat(p_local->acce.lat);
    p_bsm->motion.acce.vert = encode_acce_vert(p_local->acce.vert);
    p_bsm->motion.acce.yaw = encode_acce_yaw(p_local->acce.yaw);

    //dump((uint8_t *)p_bsm, sizeof(rcp_msg_basic_safty_t));

    timer_test_bsm_rx_2 = rt_timer_create("tm-tb",timer_test_bsm_rx_callback_2,NULL,\
        MS_TO_TICK(2400),RT_TIMER_FLAG_PERIODIC); 					

    rt_timer_start(timer_test_bsm_rx_2);
}

void tb3(void)
{
    rcp_msg_basic_safty_t *p_bsm = &test_bsm_rx_3;
    vam_stastatus_t sta;
    vam_stastatus_t *p_local = &sta;

    #if 1 
    memset(p_local, 0, sizeof(vam_stastatus_t));
    p_local->pos.lat = 40.0; //39.5427f;
    p_local->pos.lon = 120.2;//116.2317f;
    p_local->dir = 90.0;//
    #else
    memcpy(p_local, &p_cms_envar->vam.local, sizeof(vam_stastatus_t));
    #endif
    p_local->pid[0] = 0x01;
    p_local->pid[1] = 0x02;
    p_local->pid[2] = 0x03;
    p_local->pid[3] = 0x04;
    
    /* construct a fake message */
    p_bsm->header.msg_id = RCP_MSG_ID_BSM;
    p_bsm->header.msg_count = 0;
    memcpy(p_bsm->header.temporary_id, p_local->pid, RCP_TEMP_ID_LEN);
    p_bsm->header.dsecond = rcp_get_system_time();

    p_bsm->position.lon = encode_longtitude(p_local->pos.lon);
    p_bsm->position.lat = encode_latitude(p_local->pos.lat);
    p_bsm->position.elev = encode_elevation(p_local->pos.elev);
    p_bsm->position.accu = encode_accuracy(p_local->pos.accu);

    p_bsm->motion.heading = encode_heading(p_local->dir);
    p_bsm->motion.speed = encode_speed(p_local->speed);
    p_bsm->motion.acce.lon = encode_acce_lon(p_local->acce.lon);
    p_bsm->motion.acce.lat = encode_acce_lat(p_local->acce.lat);
    p_bsm->motion.acce.vert = encode_acce_vert(p_local->acce.vert);
    p_bsm->motion.acce.yaw = encode_acce_yaw(p_local->acce.yaw);

    //dump((uint8_t *)p_bsm, sizeof(rcp_msg_basic_safty_t));

    timer_test_bsm_rx_3 = rt_timer_create("tm-tb",timer_test_bsm_rx_callback_3,NULL,\
        MS_TO_TICK(2400),RT_TIMER_FLAG_PERIODIC); 					

    rt_timer_start(timer_test_bsm_rx_3);
}

rcp_msg_emergency_vehicle_alert_t test_vbd_evam;
rcp_rxinfo_t test_rxbd_vbd;
rt_timer_t timer_test_evam_vbd;

void timer_test_vbd_rx_callback(void* parameter)
{
    vam_rcp_recv(&test_rxbd_vbd, (uint8_t *)&test_vbd_evam, sizeof(rcp_msg_emergency_vehicle_alert_t));
}

void  start_vbd(void)
{
    rcp_msg_emergency_vehicle_alert_t *p_bsm = &test_vbd_evam;
    vam_stastatus_t sta;
    vam_stastatus_t *p_local = &sta;

    memset(p_local, 0, sizeof(vam_stastatus_t));
    p_local->pos.lat = 40.0; //39.5427f;
    p_local->pos.lon = 120.2;//116.2317f;
    p_local->dir = 90.0;//

    p_local->pid[0] = 0x03;
    p_local->pid[1] = 0x03;
    p_local->pid[2] = 0x03;
    p_local->pid[3] = 0x03;
    
    /* construct a fake message */
    p_bsm->header.msg_id = RCP_MSG_ID_EVAM;
    p_bsm->header.msg_count = 0;
    memcpy(p_bsm->header.temporary_id, p_local->pid, RCP_TEMP_ID_LEN);
    p_bsm->header.dsecond = rcp_get_system_time();

    p_bsm->position.lon = encode_longtitude(p_local->pos.lon);
    p_bsm->position.lat = encode_latitude(p_local->pos.lat);
    p_bsm->position.elev = encode_elevation(p_local->pos.elev);
    p_bsm->position.accu = encode_accuracy(p_local->pos.accu);

    p_bsm->motion.heading = encode_heading(p_local->dir);
    p_bsm->motion.speed = encode_speed(p_local->speed);
    p_bsm->motion.acce.lon = encode_acce_lon(p_local->acce.lon);
    p_bsm->motion.acce.lat = encode_acce_lat(p_local->acce.lat);
    p_bsm->motion.acce.vert = encode_acce_vert(p_local->acce.vert);
    p_bsm->motion.acce.yaw = encode_acce_yaw(p_local->acce.yaw);


	p_bsm->alert_mask |=(1<<VAM_ALERT_MASK_EBD);
	
    //dump((uint8_t *)p_bsm, sizeof(rcp_msg_basic_safty_t));

    timer_test_evam_vbd = rt_timer_create("tm-vbd",timer_test_vbd_rx_callback,NULL,\
        MS_TO_TICK(2400),RT_TIMER_FLAG_PERIODIC); 					

    rt_timer_start(timer_test_evam_vbd);


}

FINSH_FUNCTION_EXPORT(start_vbd, debug: testing vbd  application);

void stop_test_bsm(void)
{
	rt_timer_stop(timer_test_bsm_rx);
}
void stop_test_bsm_2(void)
{
	rt_timer_stop(timer_test_bsm_rx_2);
}


FINSH_FUNCTION_EXPORT(test_bsm, debug: testing when bsm is received);

FINSH_FUNCTION_EXPORT(stop_test_bsm, debug: testing when bsm stop);

FINSH_FUNCTION_EXPORT(tb2, debug: testing when bsm is received);

FINSH_FUNCTION_EXPORT(stop_test_bsm_2, debug: testing when bsm stop);

FINSH_FUNCTION_EXPORT(tb3, debug: testing when bsm is received);
void test_data(void)
{
    float f1 = 5.22222, f2=-3.82222;
    int32_t i1, i2;
    uint32_t u1, u2;
    uint8_t buf[64] = {0};

    sprintf(buf, "f1=%f, f2=%f \n", f1, f2);
    rt_kprintf("%s", buf);  

    i1 = (int32_t)f1;
    i2 = (int32_t)f2;
    rt_kprintf("i1=%d, i2=%d\n", i1,i2);

    u1 = (uint32_t)i1;
    u2 = (uint32_t)i2;
    rt_kprintf("u1=%d, u2=%d\n", u1,u2);
}

FINSH_FUNCTION_EXPORT(test_data, debug: testing datatype);
#endif

