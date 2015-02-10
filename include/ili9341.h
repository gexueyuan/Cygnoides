/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : ili9341.h
 @brief  : This file contains all the functions prototypes for the 
  *          ili9341.c driver
 @author : gexueyuan
 @history:
           2014-12-31    gexueyuan    Created file
           ...
******************************************************************************/
#ifndef	__ili9341_H__
#define	__ili9341_H__

#include "stm32f4xx.h"
#include "rtthread.h"
#include "board.h"

#define lcd_inline 					static __inline
//#define LCD_SPI_8b

/** 
  * @brief  LCD SPI Interface pins 
  */ 

#define LCD_SPI_SCK_PIN               GPIO_Pin_5                     /* PF.07 */
#define LCD_SPI_SCK_GPIO_PORT         GPIOA                          /* GPIOF */
#define LCD_SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOA  
#define LCD_SPI_SCK_SOURCE            GPIO_PinSource5
#define LCD_SPI_SCK_AF                GPIO_AF_SPI1

#define LCD_SPI_MISO_PIN              GPIO_Pin_6                     /* PF.08 */
#define LCD_SPI_MISO_GPIO_PORT        GPIOA                          /* GPIOF */
#define LCD_SPI_MISO_GPIO_CLK         RCC_AHB1Periph_GPIOA  
#define LCD_SPI_MISO_SOURCE           GPIO_PinSource6
#define LCD_SPI_MISO_AF               GPIO_AF_SPI1

#define LCD_SPI_MOSI_PIN              GPIO_Pin_7                     /* PF.09 */
#define LCD_SPI_MOSI_GPIO_PORT        GPIOA                          /* GPIOF */
#define LCD_SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOA  
#define LCD_SPI_MOSI_SOURCE           GPIO_PinSource7
#define LCD_SPI_MOSI_AF               GPIO_AF_SPI1

#define LCD_SPI                       SPI1
#define LCD_SPI_CLK                   RCC_APB2Periph_SPI1 

/** 
  * @brief  LCD Reset pin  
  */ 

#define LCD_SPI_RESET_PIN                GPIO_Pin_6  
#define LCD_SPI_RESET_GPIO_PORT          GPIOB      
#define LCD_SPI_RESET_GPIO_CLK           RCC_AHB1Periph_GPIOB  

/** 
  * @brief  LCD Control pin  
  */ 
#ifdef USE_I2S3
#define LCD_SPI_CS_PIN                GPIO_Pin_10  
#define LCD_SPI_CS_GPIO_PORT          GPIOB      
#define LCD_SPI_CS_GPIO_CLK           RCC_AHB1Periph_GPIOB
#else
#define LCD_SPI_CS_PIN                GPIO_Pin_4  
#define LCD_SPI_CS_GPIO_PORT          GPIOA      
#define LCD_SPI_CS_GPIO_CLK           RCC_AHB1Periph_GPIOA  
#endif

/** 
  * @brief  LCD Command/data pin  
  */
#define LCD_SPI_DC_PIN                GPIO_Pin_7  
#define LCD_SPI_DC_GPIO_PORT          GPIOB      
#define LCD_SPI_DC_GPIO_CLK           RCC_AHB1Periph_GPIOB  




 //¶¨ÒåLCDµÄ³ß´ç	
#define LCD_WIDTH         240
#define LCD_HEIGHT        320



/** 
  * @brief  LCD Registers  
  */ 
#define LCD_SLEEP_OUT            0x11   /* Sleep out register */
#define LCD_GAMMA                0x26   /* Gamma register */
#define LCD_DISPLAY_OFF          0x28   /* Display off register */
#define LCD_DISPLAY_ON           0x29   /* Display on register */
#define LCD_COLUMN_ADDR          0x2A   /* Colomn address register */ 
#define LCD_PAGE_ADDR            0x2B   /* Page address register */ 
#define LCD_GRAM                 0x2C   /* GRAM register */   
#define LCD_MAC                  0x36   /* Memory Access Control register*/
#define LCD_PIXEL_FORMAT         0x3A   /* Pixel Format register */
#define LCD_WDB                  0x51   /* Write Brightness Display register */
#define LCD_WCD                  0x53   /* Write Control Display register*/
#define LCD_RGB_INTERFACE        0xB0   /* RGB Interface Signal Control */
#define LCD_FRC                  0xB1   /* Frame Rate Control register */
#define LCD_BPC                  0xB5   /* Blanking Porch Control register*/
#define LCD_DFC                  0xB6   /* Display Function Control register*/
#define LCD_POWER1               0xC0   /* Power Control 1 register */
#define LCD_POWER2               0xC1   /* Power Control 2 register */
#define LCD_VCOM1                0xC5   /* VCOM Control 1 register */
#define LCD_VCOM2                0xC7   /* VCOM Control 2 register */
#define LCD_POWERA               0xCB   /* Power control A register */
#define LCD_POWERB               0xCF   /* Power control B register */
#define LCD_PGAMMA               0xE0   /* Positive Gamma Correction register*/
#define LCD_NGAMMA               0xE1   /* Negative Gamma Correction register*/
#define LCD_DTCA                 0xE8   /* Driver timing control A */
#define LCD_DTCB                 0xEA   /* Driver timing control B */
#define LCD_POWER_SEQ            0xED   /* Power on sequence register */
#define LCD_3GAMMA_EN            0xF2   /* 3 Gamma enable register */
#define LCD_INTERFACE            0xF6   /* Interface control register */
#define LCD_PRC                  0xF7   /* Pump ratio control register */




/** 
  * @brief  LCD color  
  */ 
#define LCD_COLOR_WHITE          0xFFFF
#define LCD_COLOR_BLACK          0x0000
#define LCD_COLOR_GREY           0xF7DE
#define LCD_COLOR_BLUE           0x001F
#define LCD_COLOR_BLUE2          0x051F
#define LCD_COLOR_RED            0xF800
#define LCD_COLOR_MAGENTA        0xF81F
#define LCD_COLOR_GREEN          0x07E0
#define LCD_COLOR_CYAN           0x7FFF
#define LCD_COLOR_YELLOW         0xFFE0


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
void LCD_SPI_Init(void);

void rt_hw_lcd_set_pixel(const char* c, int x, int y);

void hline(int x0, int x1, int y, int color);
void vline(int x, int y0, int y1, int color);
void fillrect(int x0, int y0, int x1, int y1, int color);


void LCD_PutPixel(uint16_t x,uint16_t y,uint16_t color);

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
#define RGB(r,g,b)  (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue

#endif
