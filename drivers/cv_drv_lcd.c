/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_drv_lcd.c
 @brief  : This file contains  all  lcd funtions of vanet-wifi board  
 @author : gexueyuan
 @history:
           2015-1-4    gexueyuan    Created file
           ...
******************************************************************************/
#include "cv_osal.h"

#include "ili9341.h"


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/


#if 0
int _putc(int value)
{
    if (value == '\n') {    // new line
        char_x = 0;
        char_y = char_y + font[2];
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    } else {
        character(char_x, char_y, value);
    }
    return value;
}


void character(int x, int y, int c)
{
    unsigned int hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line

    if (char_x + hor > width()) {
        char_x = 0;
        char_y = char_y + vert;
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    }
    window(char_x, char_y,hor,vert); // char box
    wr_cmd(0x2C);  // send pixel
    #ifndef TARGET_KL25Z  // 16 Bit SPI 
    SPI::format(16,3);   
    #endif                         // switch to 16 bit Mode 3
    zeichen = &font[((c -32) * offset) + 4]; // start of char bitmap
    w = zeichen[0];                          // width of actual char
     for (j=0; j<vert; j++) {  //  vert line
        for (i=0; i<hor; i++) {   //  horz line
            z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00) {
               #ifndef TARGET_KL25Z  // 16 Bit SPI 
                SPI::write(_background);
               #else
                SPI::write(_background >> 8);
                SPI::write(_background & 0xff);
                #endif
            } else {
                #ifndef TARGET_KL25Z  // 16 Bit SPI
                SPI::write(_foreground);
                #else
                SPI::write(_foreground >> 8);
                SPI::write(_foreground & 0xff);
                #endif
            }
        }
    }
    _cs = 1;
    #ifndef TARGET_KL25Z  // 16 Bit SPI
    SPI::format(8,3);
    #endif
    WindowMax();
    if ((w + 2) < hor) {                   // x offset to next char
        char_x += w + 2;
    } else char_x += hor;
}
#endif

void  rt_lcd_thread_entry(void* parameter)
{

/*
    rt_device_t dev ;
    
	dev = rt_device_find("lcd");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR);

	while(1){

	}


*/

}




void lcd_init(void)
{
   // osal_task_t  *task_lcd;


   // task_lcd = osal_task_create("t-lcd",void(* entry)(void * param),
   //                             void * param,uint32_t stk_size,uint32_t prio)





}
