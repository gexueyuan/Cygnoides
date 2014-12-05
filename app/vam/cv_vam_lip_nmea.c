#include <ctype.h>
#include <rthw.h>
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"
#include "nmea.h"
#include "gps.h"

//OS_MUT mutexNmea;
static t_nmea_cfg nmeaCfg;
uint8_t IsLocate = __FALSE;

//GPS采集周期
uint32_t gpsCycTime = CYC_GPS_DEFALUT;

driving_action_st G_Action ;
float vehicle_long_accel_value = 0.0;	//longitudinal acceleration
#define Grav_accel_value (9.80665f)	//Gravitational acceleration
#define DRIVING_RUSH_ADD_THRESHOLD		(5.5f)
#define DRIVING_RUSH_STOP_THRESHOLD	(-5.5f)

void nmea_add(t_nmea_rmc *param);
int32_t nmea_rmc_time(char *pStrS, char *pStrE, t_time *tt);
int32_t nmea_rmc_date(char *pStrS, char *pStrE, t_time *tt);
int32_t nmea_rmc_lat(char *pStrS, char *pStrE, double *latitude);
int32_t nmea_rmc_lon(char *pStrS, char *pStrE, double *longitude);

void nmea_init(void) {
	rt_mutex_init(&mutex_gps,"mutex_gps", RT_IPC_FLAG_FIFO) ;
	memset(&nmeaCfg, 0, sizeof(nmeaCfg));
}

void nmea_add(t_nmea_rmc *param) {
	static float speed_diff = 0.0 , old_speed = 0.0;
	static float angle_diff = 0.0 , old_heading = 0.0;

	static int time_diff = 0;	
	static t_time tt_old;

	//驾驶习惯发生点的信息
//static driving_rush_value_st G_tmp;
    
	rt_mutex_take(&mutex_gps,RT_WAITING_FOREVER);
#if 0
	int32_t diffT = 0;
    memcpy(&(nmeaCfg.gpsLastBuff), param, sizeof(t_nmea_rmc));

//    diffT = param->updateTime.sec - nmeaCfg.diffsec;
//    if ( diffT >= (gpsCycTime / GPS_MAX_SIZE) || diffT < 0) {
        nmeaCfg.crtIndex++;
        if (nmeaCfg.crtIndex >= GPS_MAX_SIZE) nmeaCfg.crtIndex = 0;
        memcpy((nmeaCfg.gpsBuff + nmeaCfg.crtIndex), param, sizeof(t_nmea_rmc));
        nmeaCfg.diffsec = param->updateTime.sec;
//    }
#endif
	//加速度计算
	NMEA_DEBUG("NMEA->上次时间:%d\n",tt_old.sec);
	NMEA_DEBUG("NMEA->本次时间:%d\n",param->tt.sec);
	NMEA_DEBUG("NMEA->上次速度:%f\n",old_speed);
	NMEA_DEBUG("NMEA->本次速度:%f\n",param->speed);
	
	speed_diff = param->speed - old_speed;					//速度差	km/h
	time_diff = param->tt.sec - tt_old.sec;			        //时间差
	time_diff = (time_diff < 0) ? (time_diff+60) : time_diff;
    angle_diff = param->heading - old_heading;              //角度差
    G_Action.diff_angle = abs(angle_diff);
    G_Action.speed = param->speed;

    if(param->speed==0 && speed_diff == 0)
	{
        /* 车停 */
        G_Action.carRun = 0;
	}
	if(speed_diff != 0 && time_diff < 1)
	{
        G_Action.carRun = 1;
		vehicle_long_accel_value = speed_diff * 3.6f /time_diff;		//速度单位m/s 时间单位s
		G_Action.vehicle_accel_value = vehicle_long_accel_value ;
#if 0
        uint8_t strbuf[100] = {0};
        sprintf(strbuf, "NMEA->时间间隔%d, 加速度值%f, diff_angle", time_diff, vehicle_long_accel_value, G_Action.diff_angle);
        NMEA_DEBUG("%s\r\n", strbuf);
		if(G_tmp.type == None)
		{
			if(vehicle_long_accel_value > DRIVING_RUSH_ADD_THRESHOLD)
			{				
				G_tmp.type		= Rush_Add;				
				G_tmp.value		= vehicle_long_accel_value;
				G_tmp.latitude	= param->latitude;
				G_tmp.longitude	= param->longitude;
				G_tmp.speed		= param->speed;
				memcpy(&G_tmp.time , &param->tt , sizeof(t_time));
				
				NMEA_DEBUG("NMEA->车辆开始急加速\n类型:%d 值:%f\n时间:%04d-%02d-%02d %02d:%02d:%02d\nGPS:%f,%f,%f\n",
					G_tmp.type,			G_tmp.value,
					G_tmp.time.year,	G_tmp.time.mon,		G_tmp.time.day,	
					G_tmp.time.hour,	G_tmp.time.min,	G_tmp.time.sec,				
					G_tmp.latitude,		G_tmp.longitude,	G_tmp.speed);
			}
			else if(vehicle_long_accel_value < DRIVING_RUSH_STOP_THRESHOLD)
			{
				G_tmp.type		= Rush_Stop;
				G_tmp.value		= vehicle_long_accel_value;
				G_tmp.latitude	= param->latitude;
				G_tmp.longitude	= param->longitude;
				G_tmp.speed		= param->speed;
				
				memcpy(&G_tmp.time , &param->tt , sizeof(t_time));
				
				NMEA_DEBUG("NMEA->车辆开始急减速\n类型:%d 值:%f\n时间:%04d-%02d-%02d %02d:%02d:%02d\nGPS:%f,%f,%f\n",
					G_tmp.type,			G_tmp.value,
					G_tmp.time.year,	G_tmp.time.mon,		G_tmp.time.day,	
					G_tmp.time.hour,	G_tmp.time.min,	G_tmp.time.sec,				
					G_tmp.latitude,		G_tmp.longitude,	G_tmp.speed);
			}
		}
		else if(G_tmp.type == Rush_Add)
		{
			if(vehicle_long_accel_value > DRIVING_RUSH_ADD_THRESHOLD)	//持续ADD
			{
				NMEA_DEBUG("NMEA->车辆持续急加速中\n");
			}
			else
			{
                G_tmp.type = None;
				NMEA_DEBUG("NMEA->车辆停止急加速\n");
			}
		}
		else if(G_tmp.type == Rush_Stop)
		{
			if(vehicle_long_accel_value < DRIVING_RUSH_STOP_THRESHOLD)	//持续STOP
			{
				NMEA_DEBUG("NMEA->车辆持续急减速中\n");
			}
			else
			{				
                G_tmp.type = None;
				NMEA_DEBUG("NMEA->车辆停止急减速\n");
			}		
		}
#endif      
	}
    else
	{
		NMEA_DEBUG("NMEA->时间间隔[%d]s 不符合要求\n",time_diff);
	}

	old_speed = param->speed;
	tt_old.sec = param->updateTime.sec ;
    old_heading = param->heading;
	rt_mutex_release(&mutex_gps);
}

int32_t nmea_get(t_nmea_rmc *recvBuff, int8_t flag) {
	rt_mutex_take(&mutex_gps,RT_WAITING_FOREVER);

    if (flag < 0) {
        memcpy(recvBuff, &(nmeaCfg.gpsLastBuff), sizeof(t_nmea_rmc));
    }
    else if (flag < GPS_MAX_SIZE){
        memcpy(recvBuff, (nmeaCfg.gpsBuff + flag), sizeof(t_nmea_rmc));
    }

	rt_mutex_release(&mutex_gps);

    return 0;
}

int32_t nmea_rmc_time(char *pStrS, char *pStrE, t_time *tt)
{
    uint8_t TmpBuff[5];

    memset(TmpBuff, 0, sizeof(TmpBuff));
    TmpBuff[0] = *(pStrS);
    TmpBuff[1] = *(pStrS + 1);
    tt->hour = (uint8_t) atoi((char *)TmpBuff);

    memset( TmpBuff, 0, sizeof(TmpBuff));
    TmpBuff[0] = *(pStrS + 2);
    TmpBuff[1] = *(pStrS + 3);
    tt->min = (uint8_t) atoi((char *)TmpBuff);

    memset( TmpBuff, 0, sizeof(TmpBuff));
    TmpBuff[0] = *(pStrS + 4);
    TmpBuff[1] = *(pStrS + 5);
    tt->sec = (uint8_t) atoi((char *)TmpBuff);

    return 0;
}

int32_t nmea_rmc_date(char *pStrS, char *pStrE, t_time *tt)
{
    uint8_t TmpBuff[5];

    memset(TmpBuff, 0, sizeof(TmpBuff));
    TmpBuff[0] = *(pStrS);
    TmpBuff[1] = *(pStrS + 1);
    tt->day = (uint8_t) atoi((char *)TmpBuff);

    memset(TmpBuff, 0, sizeof(TmpBuff));
    TmpBuff[0] = *(pStrS + 2);
    TmpBuff[1] = *(pStrS + 3);
    tt->mon = (uint8_t) atoi((char *)TmpBuff);

    memset(TmpBuff, 0, sizeof(TmpBuff));
    TmpBuff[0] = *(pStrS + 4);
    TmpBuff[1] = *(pStrS + 5);
    tt->year = (uint16_t) atoi((char *)TmpBuff) + 2000;

    return 0;
}

int32_t nmea_rmc_lat(char *pStrS, char *pStrE, double *latitude)
{
    uint8_t TmpBuff[15];
    uint16_t i = 0;

    if ( *(pStrE + 1) != 'N' && *(pStrE + 1) != 'S' && *(pStrS + 4) != '.'){
        return (-1);
    }

    //度
    memset(TmpBuff, 0, sizeof(TmpBuff));
    memcpy(TmpBuff, pStrS, 2);
    (*latitude) = (double)atoi((char *)TmpBuff);
    //分
    memset(TmpBuff, 0, sizeof(TmpBuff));
    for(i = 0; i < 8; i++) {
        if (isdigit(*(pStrS + 2 + i)) || *(pStrS + 2 + i) == '.') {
            TmpBuff[i] = *(pStrS + 2 + i);
        }
        else break;
    }
    (*latitude) += (atof((char *)TmpBuff) / 60.0);

    if ( *(pStrE + 1) == 'S' )  (*latitude) *= -1;

    return 0;
}

int32_t nmea_rmc_lon(char *pStrS, char *pStrE, double *longitude)
{
    uint8_t TmpBuff[15];
    uint16_t i = 0;

    if ( *(pStrE + 1) != 'W' && *(pStrE + 1) != 'E' && *(pStrS + 5) != '.'){
        return (-1);
    }

    //度
    memset(TmpBuff, 0, sizeof(TmpBuff));
    memcpy(TmpBuff, pStrS, 3);
    (*longitude) = (double)atoi((char *)TmpBuff);
    //分
    memset(TmpBuff, 0, sizeof(TmpBuff));
    //memcpy(TmpBuff, pStrS + 3, 8);
    for(i = 0; i < 8; i++) {
        if (isdigit(*(pStrS + 3 + i)) || *(pStrS + 3 + i) == '.') {
            TmpBuff[i] = *(pStrS + 3 + i);
        }
        else break;
    }
    (*longitude) += (atof((char *)TmpBuff) / 60.0);

    if ( *(pStrE + 1) == 'W' )  (*longitude) *= -1;

    return 0;
}

void nmea_parse(uint8_t *buff, uint32_t len)
{
    uint8_t crcCk = 0x00;
    uint8_t crcCp = 0x00;
    uint32_t index = 0;
    t_time tt;
    double latitude = 0.0;
    double longitude = 0.0;
    double speed = 0.0;
    double heading = 0.0;
    float accu = 0.0;
    char *pStrS = NULL;
    char *pStrE = NULL;
    e_nmea_type CurPackType = GPS_PACK_UNKNOWN;
    t_nmea_rmc nmeaRmc;

    if (memcmp(buff, "$GPRMC", 6) == 0){
        CurPackType = GPS_PACK_GPRMC;

        //NMEA_DEBUG("NMEA->%s\n", buff);
    }
    else if (memcmp(buff, "$GPGSA", 6) == 0){
        CurPackType = GPS_PACK_GPGSA;

        //NMEA_DEBUG("NMEA->%s\n", buff);
    }
    else return;
    //if (len <= 50) return;

    if (buff[len - 1] != '\n' || buff[len - 2] != '\r') {
        NMEA_DEBUG("NMEA->No \\r\\n\n");
        return;
    }
    if (buff[len - 5] != '*') {
        NMEA_DEBUG("NMEA->No *\n");
        return;
    }

    //Check CRC
    for (index = 1; index < len -5; index++) {
        crcCk ^= buff[index];
    }

    //Convent
    if (isdigit(buff[len - 4])) {
        crcCp = (buff[len - 4] << 4) & 0xf0;
    }
    else if (isalpha(buff[len - 4])) {
        crcCp = ((buff[len - 4] << 4) + 0x90 ) & 0xf0;
    }
    else {
        NMEA_DEBUG("NMEA->Error 0\n");
        return;
    }

    if (isdigit(buff[len - 3])) {
        crcCp |= (buff[len - 3] & 0x0f);
    }
    else if (isalpha(buff[len - 3])) {
        crcCp |= ((buff[len - 3] + 0x09 ) & 0x0f);
    }
    else {
        NMEA_DEBUG("NMEA->Error 1\n");
        return;
    }

    //Compare
    if (crcCk != crcCp) {
        NMEA_DEBUG("NMEA->数据帧CRC校验错误\n");
        return;
    }

    if (CurPackType == GPS_PACK_GPRMC){
        if (IsLocate != __TRUE) {
            NMEA_DEBUG("NMEA->定位精度不够\n");
            return;
        }

        pStrS = (char *)buff;
        if ((pStrS = strchr((char *)buff, ',')) != NULL) {
            pStrS++;
            if ((pStrS = strchr(pStrS, ',')) != NULL) {
                pStrS++;
            }
            else return;
        }
        else return;

        if ((*pStrS) == 'A') {
            pStrS = (char *)buff;
            //跳转一次逗号至时间
            if ((pStrS = strchr(pStrS, ',')) != NULL) {
                pStrS++;
                if ((pStrE = strchr(pStrS, ',')) == NULL) return;
            }
            else return;
            if (nmea_rmc_time(pStrS, pStrE, &tt) < 0) return;
            //跳转两次逗号至纬度
            if ((pStrS = strchr((pStrE+1), ',')) != NULL) {
                pStrS++;
                if ((pStrE = strchr(pStrS, ',')) == NULL) return;
            }
            else return;
            if (nmea_rmc_lat(pStrS, pStrE, &latitude) < 0) return;
            //跳转两次逗号至经度
            if ((pStrS = strchr((pStrE+1), ',')) != NULL) {
                pStrS++;
                if ((pStrE = strchr(pStrS, ',')) == NULL) return;
            }
            else return;
            if (nmea_rmc_lon(pStrS, pStrE, &longitude) < 0) return;
            //跳转两次逗号至速度
            if ((pStrS = strchr((pStrE+1), ',')) != NULL) {
                uint8_t Tmp[10];
                pStrS++;
                if ((pStrE = strchr(pStrS, ',')) == NULL) return;
                memset(Tmp, 0, sizeof(Tmp));
                memcpy(Tmp, pStrS, pStrE - pStrS);
                speed = atof((char *)Tmp)* 1.852f;
            }
            else return;

            // added by wangyf
            //跳转一次逗号至方向
            if ((pStrS = strchr((pStrS), ',')) != NULL) {
                uint8_t Tmp[10];
                pStrS++;
                if ((pStrE = strchr(pStrS, ',')) == NULL) return;
                memset(Tmp, 0, sizeof(Tmp));
                memcpy(Tmp, pStrS, pStrE - pStrS);
                heading = atof((char *)Tmp);
            }
            else return;
            //add end

            if ( (pStrS = strchr(pStrS + 1, ',')) != NULL &&\
                 (pStrE = strchr(pStrS + 1, ',')) != NULL &&\
                 nmea_rmc_date(pStrS + 1, pStrE, &tt) == 0)
            {
				//char buf[100]={0};
				nmeaRmc.isTrue = 1;
				nmeaRmc.latitude = latitude;
				nmeaRmc.longitude = longitude;
				nmeaRmc.speed = speed;
                nmeaRmc.heading = heading;
				memcpy(&(nmeaRmc.tt), &tt, sizeof(tt));
//				nmeaRmc.updateTime = get_current_time();
				//sprintf(buf,"NMEA->Lat:%f Lon:%f Speed:%f Heading:%f\n", latitude, longitude, speed, heading);
                //rt_kprintf(buf);

                lip_update_local(&nmeaRmc, NULL);
                nmea_add(&nmeaRmc);
                //UTC 时间
//                if (time_check() == __FALSE) {
//                    time_set(tt);
//                }
            }
            else return;
        }
        else {
            IsLocate = __FALSE;
            NMEA_DEBUG("NMEA->未定位\n");
        }
    }
    else if (CurPackType == GPS_PACK_GPGSA) {
        uint8_t Tmp[16];
        uint8_t i = 0;
        uint8_t IsError = __FALSE;

        //判断定位
        for (pStrS = (char *)buff, i = 0; i < 2; i++) {
            if ((pStrS = strchr(pStrS + 1, ',')) == NULL) {
                IsError = __TRUE;
                break;
            }
        }
        if (IsError == __FALSE) {
            pStrS++;
            if (*(pStrS) == ',') {
                IsLocate = __FALSE;
                NMEA_DEBUG("NMEA->未定位\n");
                return;
            }
            if ((pStrE = strchr(pStrS, ',')) == NULL) return;
            if (pStrE - pStrS > (sizeof(Tmp) - 1))  return;

            memset(Tmp, 0, sizeof(Tmp));
            memcpy(Tmp, pStrS, pStrE - pStrS);

            NMEA_DEBUG("NMEA->定位标志:%s\n", Tmp);

            if (atoi((char *)Tmp) == 1) {
                IsLocate = __FALSE;
                NMEA_DEBUG("NMEA->未定位\n");
                return;
            }
        }
        else {
            return;
        }
        //判断定位精度
        IsError = __FALSE;
        for (pStrS = (char *)buff, i = 0; i < 15; i++) {
            if ((pStrS = strchr(pStrS + 1, ',')) == NULL) {
                IsError = __TRUE;
                break;
            }
        }
        if (IsError == __FALSE) {
            pStrS++;
            if (*(pStrS) == ',') {
                IsLocate = __FALSE;
                NMEA_DEBUG("NMEA->定位精度不够\n");
                return;
            }
            if ((pStrE = strchr(pStrS, ',')) == NULL) return;
            if (pStrE - pStrS > (sizeof(Tmp) - 1))  return;

            memset(Tmp, 0, sizeof(Tmp));
            memcpy(Tmp, pStrS, pStrE - pStrS);

            NMEA_DEBUG("NMEA->定位精度:%s\n", Tmp);

            accu = (float)atof((char *)Tmp);

            if ( accu > 49.0f) {
                IsLocate = __FALSE;
                NMEA_DEBUG("NMEA->定位精度不够\n");
            }
            else {
                IsLocate = __TRUE;
                lip_update_local(NULL, &accu);
                NMEA_DEBUG("NMEA->有效定位精度\n");
            }
        }
        else {
            ;
        }
    }
}
