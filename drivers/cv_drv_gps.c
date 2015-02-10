
#include "cv_osal.h"

#include "components.h"
#include "cv_vam.h"
#include "cv_cms_def.h"

#include "gps.h"
#include "nmea.h"

t_gps_buff __GPSBuff;
gps_data_callback gps_recv_cb = NULL;
uint8_t _GPSBuffer[GPS_BUFF_SIZE];
uint8_t get_gps = 0 ;

struct rt_mutex  mutex_gps ;



void gps_callback_register(gps_data_callback fp)
{
	gps_recv_cb = fp;
}


static void ubx_pkt_checknum_calc(uint8_t *buf, int len, uint8_t *cknumA, uint8_t *cknumB)
{
    uint16_t ckA=0, ckB=0;
    int i=0;
    for(i=0; i<len; i++)
    {
        ckA = ckA + buf[i];
        ckB = ckB + ckA;
    }
    *cknumA = ckA;
    *cknumB = ckB;
}

static void ubx_cfg_msg_std_nmea(ubx_cfg_msg_nmea_id_t nmea_id, uint8_t enable)
{
 	rt_device_t dev; 
    int len = 0;
    uint8_t buf[20];
 
    gps_ubx_pkt_hdr_t pkt;
    gps_ubx_cfg_msg_t cfg;
    memset(&cfg, 0x0, sizeof(cfg));
    pkt.syncChar1 = UBX_SYSN_CHAR1;
    pkt.syncChar2 = UBX_SYSN_CHAR2;
    pkt.msgClass = 0x06;
    pkt.msgId = 0x01;
    pkt.length = sizeof(gps_ubx_cfg_msg_t);
    len = sizeof(gps_ubx_pkt_hdr_t);
    memcpy(buf, &pkt, len);

    /* standard NMEA messages */
    cfg.nmeaClass = 0xF0;
    cfg.nmeaid = nmea_id;
    cfg.portOn[1] = enable;

    memcpy(buf+len, &cfg, pkt.length);
    len += pkt.length;

    ubx_pkt_checknum_calc(buf+2, len-2, &buf[len], &buf[len+1]); 
    len += 2;
	
    dev = rt_device_find(RT_GPS_DEVICE_NAME);

    rt_device_write(dev, 0, buf, len);    
}


/* UBX-CFG MSG. disable GPGGA/GPGLL/GPGSV/GPVTG msg */
static void gps_cfg_msg(void)
{
    ubx_cfg_msg_std_nmea(STD_NMEA_ID_GGA, 0);
    ubx_cfg_msg_std_nmea(STD_NMEA_ID_GLL, 0);
    ubx_cfg_msg_std_nmea(STD_NMEA_ID_GSV, 0);
    ubx_cfg_msg_std_nmea(STD_NMEA_ID_VTG, 0);
}

/* UBX-CFG-PRT: set gps port baudrate to 115200 */
static void gps_cfg_prt(void)
{
   /* set baut rate = 115200 */
    uint8_t cfg_pkt[] = {
        0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0,
        0x08, 0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 0x03, 0x00, 0x03, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0xBC, 0x5E
    };
    
	rt_device_t dev ; 
    dev = rt_device_find(RT_GPS_DEVICE_NAME);

    rt_device_write(dev, 0, cfg_pkt, sizeof(cfg_pkt));
}

/* set host uart1 baudrate  */
static void gps_set_host_baudrate(int baud)
{
    rt_device_t dev;
    struct serial_configure config;

    config.baud_rate = baud;
	config.bit_order = BIT_ORDER_LSB;
	config.data_bits = DATA_BITS_8;
	config.parity    = PARITY_NONE;
	config.stop_bits = STOP_BITS_1;
	config.invert    = NRZ_NORMAL;     
    
	dev = rt_device_find(RT_GPS_DEVICE_NAME);
    rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, (void *)&config);
}


void gps_cfg_rate(uint8_t freq)
{
	rt_device_t dev; 
    uint8_t cfg_pkt[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xF4, 0x01, 
                         0x01, 0x00, 0x01, 0x00, 0x0B, 0x77};

    switch (freq)
    {
        case 1:
        {
            cfg_pkt[6] = 0xE8;
            cfg_pkt[7] = 0x03;
            cfg_pkt[12] = 0x01;
            cfg_pkt[13] = 0x39;
            break;
        }
        case 2:
        {
            cfg_pkt[6] = 0xF4;
            cfg_pkt[7] = 0x01;
            cfg_pkt[12] = 0x0B;
            cfg_pkt[13] = 0x77;
            break;
        }
        case 5:  //5Hz
        {
            cfg_pkt[6] = 0xC8;
            cfg_pkt[7] = 0x00;
            cfg_pkt[12] = 0xDE;
            cfg_pkt[13] = 0x6A;
            break;
        }
        default:
        {
            break;
        }
    }
    dev = rt_device_find(RT_GPS_DEVICE_NAME);
    rt_device_write(dev, 0, cfg_pkt, sizeof(cfg_pkt));
    
}
static void gps_read_data(rt_device_t dev)
{
	uint8_t tmp = 0 ;
    static uint8_t cfg_flag = 0;
	while(1){
		if(rt_device_read(dev, 0, &tmp, 1) == 1){

			if (tmp == '$') {
                memset(__GPSBuff.PpBuf[__GPSBuff.Pipe].Buf, 0x0, GPS_BUFF_SIZE);
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Len = 1;
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Buf[0] = '$';
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Flag = 1;	
			}
			else if (tmp == '\n') {
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Buf[__GPSBuff.PpBuf[__GPSBuff.Pipe].Len] = '\n';
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Len++;
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Len %= GPS_BUFF_SIZE;
				__GPSBuff.PpBuf[__GPSBuff.Pipe].Flag = 0;

                if((0 == cfg_flag) && (0 == memcmp(__GPSBuff.PpBuf[__GPSBuff.Pipe].Buf, "$GPTXT", 6)))
                {
                    /* got ublox GPTXT msg. config needed nmea */
                    gps_cfg_msg();
                    osal_delay(1);	
                    /* conifg ublox gps rate 5Hz */
					gps_cfg_rate(5);
					osal_delay(1);
                    /* config gps baud to 115200 */
                    gps_cfg_prt();
                    osal_delay(1);
                    gps_set_host_baudrate(BAUD_RATE_115200);
					osal_delay(1);
                    cfg_flag = 1;
                }                   

                {
                    sys_msg_t *p_msg;
                    p_msg = osal_malloc(sizeof(sys_msg_t));
                    if (p_msg) {
                        p_msg->id = VAM_MSG_GPSDATA;
                        p_msg->len = __GPSBuff.PpBuf[__GPSBuff.Pipe].Len;
                        p_msg->argv = &__GPSBuff.PpBuf[__GPSBuff.Pipe].Buf;
                        vam_add_event_queue_2(&p_cms_envar->vam, p_msg);
                    }
                }

				__GPSBuff.Pipe++;
				__GPSBuff.Pipe %= GPS_PIPE;
				get_gps = 1 ;
			}
			else {
				if (__GPSBuff.PpBuf[__GPSBuff.Pipe].Flag == 1) {
					__GPSBuff.PpBuf[__GPSBuff.Pipe].Buf[__GPSBuff.PpBuf[__GPSBuff.Pipe].Len] = tmp;
					__GPSBuff.PpBuf[__GPSBuff.Pipe].Len++;
					__GPSBuff.PpBuf[__GPSBuff.Pipe].Len %= GPS_BUFF_SIZE;
				}
                else
                {
                    /* receive messy code because of baudrate(9600) is not correct, 
                       set host uart1 baud to 115200 */
                    if(!cfg_flag)
                    {
                        gps_set_host_baudrate(BAUD_RATE_115200);
                        cfg_flag = 1;
                    }
                    
                }
			}
		}
		else{
			break ;
		}
	}
}

void rt_gps_thread_entry (void *parameter) 
{
	rt_device_t dev ;
    
	dev = rt_device_find(RT_GPS_DEVICE_NAME);
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR);
	
	while(1){
		gps_read_data(dev);
		osal_delay(1);
	}
}

//*******************************************************************************
// 函数名称    : void gps_init(void)
// 功能描述    : 初始化gps模块，例如屏蔽不需要的信息，天线断开延时
// 输入        : None
// 输出        : None
// 返回        : None
//******************************************************************************/
void gps_init(void)
{
    osal_task_t *tid;

	memset(&__GPSBuff, 0, sizeof(__GPSBuff));
	gps_recv_cb = NULL;


	tid = osal_task_create("t-gps",
                           rt_gps_thread_entry, RT_NULL,
                           RT_GPS_THREAD_STACK_SIZE, RT_GPS_THREAD_PRIORITY);
    osal_assert(tid != RT_NULL)
}

void gps_deinit(void)
{
	memset(&__GPSBuff, 0, sizeof(__GPSBuff));
	gps_recv_cb = NULL;	
}

/* shell cmd for debug */
//FINSH_FUNCTION_EXPORT(gps_cfg_rate, debug: set gps rate);

//==============================================================================
//                                   0ooo
//                          ooo0     (   ) 
//                          (   )     ) /
//                           \ (     (_/
//                            \_) 
//==============================================================================

