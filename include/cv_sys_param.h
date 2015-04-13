/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_sys_param.h
 @brief  : head file of vanet's param 
 @author : gexueyuan
 @history:
           2015-2-4    gexueyuan    Created file
           ...
******************************************************************************/

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
typedef struct _flash_param{
    char *key;
    char *value;
}flash_param, *flash_param_t;


typedef struct {

	cfg_param_t cus_param;
	cfg_param_t hw_param;
	cfg_param_t mt_param;
	cfg_param_t ct_param;

}vanet_param;


#define PARAM_FLAG_ADDR     ((uint32_t)0x80E0000)

#define PARAM_MODE_ADDR     ((uint32_t)0x80E0010)

#define PARAM_ADDR    		((uint32_t)0x80E0020)

#define name_to_str(name)  (#name)




static const flash_param param_data[] = {

    {"ID","0"},
    {"vam.bsm_hops","0"},
    {"vam.bsm_boardcast_mode","0"},
    {"vam.bsm_boardcast_saftyfactor","1"},
    {"vam.bsm_pause_mode","0"},




};
