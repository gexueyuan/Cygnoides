/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : ili9341.c
 @brief  : This file includes the LCD driver for ILI9341 Liquid Crystal
           Display Modules
 @author : gexueyuan
 @history:
           2014-12-31    gexueyuan    Created file
           ...
******************************************************************************/
#include "ili9341.h"

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/

#define COMMAND_MODE   LCD_CtrlLinesWrite(LCD_SPI_DC_GPIO_PORT, LCD_SPI_DC_PIN, Bit_RESET)
#define DATA_MODE      LCD_CtrlLinesWrite(LCD_SPI_DC_GPIO_PORT, LCD_SPI_DC_PIN, Bit_SET) 

uint8_t orientation = 0;
/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/

static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = nCount; index != 0; index--)
  {
  }
}

void LCD_SPI_16BIT(void)
{
    SPI_InitTypeDef    SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz) 
       to verify these constraints:
          - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
          - l3gd20 SPI interface max baudrate is 10MHz for write/read
          - PCLK2 frequency is set to 90 MHz 
       */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(LCD_SPI, &SPI_InitStructure);
    
    /* Enable L3GD20_SPI  */
    SPI_Cmd(LCD_SPI, ENABLE);


}

void LCD_SPI_8BIT(void)
{
    SPI_InitTypeDef    SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz) 
       to verify these constraints:
          - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
          - l3gd20 SPI interface max baudrate is 10MHz for write/read
          - PCLK2 frequency is set to 90 MHz 
       */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(LCD_SPI, &SPI_InitStructure);
    
    /* Enable L3GD20_SPI  */
    SPI_Cmd(LCD_SPI, ENABLE);


}


/**
  * @brief  Sets or reset LCD control lines.
  * @param  GPIOx: where x can be B or D to select the GPIO peripheral.
  * @param  CtrlPins: the Control line.
  *   This parameter can be:
  *     @arg LCD_NCS_PIN: Chip Select pin
  *     @arg LCD_NWR_PIN: Read/Write Selection pin
  *     @arg LCD_RS_PIN: Register/RAM Selection pin
  * @param  BitVal: specifies the value to be written to the selected bit.
  *   This parameter can be:
  *     @arg Bit_RESET: to clear the port pin
  *     @arg Bit_SET: to set the port pin
  * @retval None
  */
lcd_inline void LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal)
{
    /* Set or Reset the control line */
    GPIO_WriteBit(GPIOx, (uint16_t)CtrlPins, (BitAction)BitVal);
}



/**
  * @brief  Configures LCD control lines in Output Push-Pull mode.
  * @note   The LCD_CS line can be configured in Open Drain mode  
  *         when VDDIO is lower than required LCD supply.
  * @param  None
  * @retval None
  */
void LCD_CtrlLinesConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOs clock*/
    RCC_AHB1PeriphClockCmd(LCD_SPI_CS_GPIO_CLK | LCD_SPI_DC_GPIO_CLK, ENABLE);

    /* Configure CS in Output Push-Pull mode */
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LCD_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

    /* Configure WRX in Output Push-Pull mode */
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_DC_PIN;
    GPIO_Init(LCD_SPI_DC_GPIO_PORT, &GPIO_InitStructure);

    /* Set chip select pin high */
    LCD_CtrlLinesWrite(LCD_SPI_CS_GPIO_PORT, LCD_SPI_CS_PIN, Bit_SET);
}




/**
  * @brief  Controls LCD Chip Select (CS) pin
  * @param  NewState CS pin state
  * @retval None
  */
lcd_inline void LCD_ChipSelect(FunctionalState NewState)
{
    if (NewState == DISABLE){
        GPIO_SetBits(LCD_SPI_CS_GPIO_PORT, LCD_SPI_CS_PIN); /* CS pin high: LCD disabled */
    }
    else{
        GPIO_ResetBits(LCD_SPI_CS_GPIO_PORT, LCD_SPI_CS_PIN); /* CS pin low: LCD enabled */
    }
}


/**
  * @brief  Writes command to select the LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
lcd_inline void LCD_WriteCommand(uint8_t LCD_Reg)
{

    /* Reset LCD control line(/CS) and Send command */
    LCD_ChipSelect(ENABLE);

    /* Reset WRX to send command */
    COMMAND_MODE;

    SPI_I2S_SendData(LCD_SPI, LCD_Reg);

    /* Wait until a data is sent(not busy), before config /CS HIGH */

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET) ;

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET);

    //LCD_ChipSelect(DISABLE);

    /* Set WRX to send data */
    DATA_MODE;
}

/**
  * @brief  Writes data to select the LCD register.
  *         This function must be used after LCD_WriteCommand() function
  * @param  value: data to write to the selected register.
  * @retval : 
  */
lcd_inline void LCD_WriteData(uint16_t value)
{

    /* Reset LCD control line(/CS) and Send command */
   // LCD_ChipSelect(ENABLE);

   // DATA_MODE;


    SPI_I2S_SendData(LCD_SPI, value);

    /* Wait until a data is sent(not busy), before config /CS HIGH */

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET) ;

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET);

    //LCD_ChipSelect(DISABLE);
}

/**
  * @brief  Read data from select the LCD register.
  *         This function must be used after LCD_SetXY() function
  * @param  value: None.
  * @retval: data from the specify position
  */
lcd_inline char  LCD_ReadByte(unsigned char cmd)
{
    char temp;

    LCD_WriteCommand(cmd);
    
    COMMAND_MODE;

    /* Reset LCD control line(/CS) and Send data */  
    LCD_ChipSelect(ENABLE);
    
    temp = SPI_I2S_ReceiveData(LCD_SPI);//dump
    
    /* Wait until a data is recieve(not busy), before config /CS HIGH */

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) != RESET) ;

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET);


    temp = SPI_I2S_ReceiveData(LCD_SPI);
    
    /* Wait until a data is recieve(not busy), before config /CS HIGH */

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) != RESET) ;

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET);

    LCD_ChipSelect(DISABLE);

    return temp;
}



lcd_inline void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WriteCommand(LCD_COLUMN_ADDR);
    LCD_WriteData(x1>>8);
    LCD_WriteData(x1&0x00ff);
    LCD_WriteData(x2>>8);
    LCD_WriteData(x2&0x00ff);
    LCD_WriteCommand(LCD_PAGE_ADDR);
    LCD_WriteData(y1>>8);
    LCD_WriteData(y1&0x00ff);
    LCD_WriteData(y2>>8);
    LCD_WriteData(y2&0x00ff);
}

void LCD_PutPixel(uint16_t x,uint16_t y,uint16_t color)
{

    LCD_WriteCommand(LCD_COLUMN_ADDR);
    LCD_WriteData(x>>8);
    LCD_WriteData(x);
    LCD_ChipSelect(DISABLE);
    
    LCD_WriteCommand(LCD_PAGE_ADDR);
    LCD_WriteData(y>>8);
    LCD_WriteData(y);
    LCD_ChipSelect(DISABLE);

    LCD_WriteCommand(LCD_GRAM);

    LCD_SPI_16BIT();

    LCD_WriteData(color);

    LCD_SPI_8BIT();
    
    LCD_ChipSelect(DISABLE);

 
}


lcd_inline void LCD_Set_Cursor(u16 x,u16 y)
{
    LCD_SetWindow(x, y, LCD_WIDTH-1, LCD_HEIGHT-1);
}



int LCD_Get_Width()
{
    if (orientation == 0 || orientation == 2) return LCD_WIDTH;
    else return LCD_HEIGHT;
}


int LCD_Get_Height()
{
    if (orientation == 0 || orientation == 2) return LCD_HEIGHT;
    else return LCD_WIDTH;
}

void WindowMax (void)
{
    LCD_SetWindow(0, 0, LCD_Get_Width()-1,  LCD_Get_Height()-1);
}

void LCD_Set_Orientation(unsigned int o)
{
    orientation = o;
    LCD_WriteCommand(LCD_MAC);                     // MEMORY_ACCESS_CONTROL
    switch (orientation) {
        case 0:
            LCD_WriteData(0x48);
            break;
        case 1:
            LCD_WriteData(0x28);
            break;
        case 2:
            LCD_WriteData(0x88);
            break;
        case 3:
            LCD_WriteData(0xE8);
            break;
    }
    WindowMax();
} 





static void LCD_Clear(uint16_t color)
{
#if 0
    uint8_t  color_H,color_L;
    uint16_t i,j; 

    color_H = color >> 8;
    color_L = 0xFF&color;
    
    //WindowMax();
    LCD_SetWindow(0,0,200,200);
    LCD_WriteCommand(LCD_GRAM);          /* Prepare to write GRAM */
      for(i=0;i<LCD_HEIGHT/4;i++){
	  for (j=0;j<LCD_WIDTH;j++){
        
        	 LCD_WriteData(color_H);
			 LCD_WriteData(color_L);	
			//LCD_PutPixel(j,i,color);

	  }

	}
#endif
fillrect(0,0,LCD_Get_Width()-1,LCD_Get_Height()-1,color);
}

#if 0
static void Lcd_Data_Test(void)
{
    unsigned short temp1;
    unsigned short temp2;

    LCD_Set_Cursor(0,0);
    LCD_WriteCommand(0x2c);          /* Prepare to write GRAM */
    LCD_WriteData(0x55);
    LCD_WriteData(0x66);

    LCD_Set_Cursor(1,0);
    LCD_WriteCommand(0x2c);          /* Prepare to write GRAM */
    LCD_WriteData(0xbb);
    LCD_WriteData(0xbb);
    /* read */
    LCD_Set_Cursor(0,0);

    LCD_WriteCommand(0x2e);
    temp1 = LCD_ReadData();
    temp2 = LCD_ReadData();

    if( (temp1 == 0x5566) && (temp2 == 0xAAbb) )
    {
        rt_kprintf(" lcd data bus test pass!");
    }
    else
    {
        rt_kprintf(" lcd data bus test error: %04X %04X",temp1,temp2);
    }
}

#endif
void lcd_read_Id(void)
{
/*
    uint8_t temp1,temp2,temp3;
    
    LCD_WriteCommand(0xda);

    LCD_CtrlLinesWrite(LCD_SPI_DC_GPIO_PORT, LCD_SPI_DC_PIN, Bit_SET);
    temp1 = SPI_I2S_ReceiveData(LCD_SPI);
    delay(200);
    temp2 = SPI_I2S_ReceiveData(LCD_SPI);
    delay(200);
    temp3 = SPI_I2S_ReceiveData(LCD_SPI);

  

    rt_kprintf("temp = %d  %d  %d\n\n",temp1,temp2,temp3);
*/
  rt_kprintf("id = %d\n\n",LCD_ReadByte(0xda));

}

//FINSH_FUNCTION_EXPORT(Lcd_Data_Test,"test lcd data\n");


/**
  * @brief  Configures the LCD_SPI interface.
  * @param  None
  * @retval None
  */
void LCD_SPIConfig(void)
{
    SPI_InitTypeDef    SPI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;

    /*Enable LCD control line*/
    RCC_AHB1PeriphClockCmd(LCD_SPI_CS_GPIO_CLK  | LCD_SPI_DC_GPIO_CLK | LCD_SPI_RESET_GPIO_CLK, ENABLE);


    /* Configure LCD_SPI CS pin */
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(LCD_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

    /* Configure LCD_SPI DC  pin */    
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_DC_PIN;
    GPIO_Init(LCD_SPI_DC_GPIO_PORT, &GPIO_InitStructure);


    /* Configure LCD_SPI DC  pin */    
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_RESET_PIN;
    GPIO_Init(LCD_SPI_RESET_GPIO_PORT, &GPIO_InitStructure);

    /* Enable LCD_SPI_SCK_GPIO_CLK, LCD_SPI_MISO_GPIO_CLK and LCD_SPI_MOSI_GPIO_CLK clock */
    RCC_AHB1PeriphClockCmd(LCD_SPI_SCK_GPIO_CLK | LCD_SPI_MISO_GPIO_CLK | LCD_SPI_MOSI_GPIO_CLK, ENABLE);

    /* Enable LCD_SPI and SYSCFG clock  */
    RCC_APB2PeriphClockCmd(LCD_SPI_CLK, ENABLE);

    /* Configure LCD_SPI SCK pin */
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /* Configure LCD_SPI MISO pin */
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
    GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    /* Configure LCD_SPI MOSI pin */
    GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
    GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /* Connect SPI SCK */
    GPIO_PinAFConfig(LCD_SPI_SCK_GPIO_PORT, LCD_SPI_SCK_SOURCE, LCD_SPI_SCK_AF);

    /* Connect SPI MISO */
    GPIO_PinAFConfig(LCD_SPI_MISO_GPIO_PORT, LCD_SPI_MISO_SOURCE, LCD_SPI_MISO_AF);

    /* Connect SPI MOSI */
    GPIO_PinAFConfig(LCD_SPI_MOSI_GPIO_PORT, LCD_SPI_MOSI_SOURCE, LCD_SPI_MOSI_AF);

    SPI_I2S_DeInit(LCD_SPI);

    /* SPI configuration -------------------------------------------------------*/
    /* If the SPI peripheral is already enabled, don't reconfigure it */
    if ((LCD_SPI->CR1 & SPI_CR1_SPE) == 0){    
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
        /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz) 
           to verify these constraints:
              - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
              - l3gd20 SPI interface max baudrate is 10MHz for write/read
              - PCLK2 frequency is set to 90 MHz 
           */
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial = 7;
        SPI_Init(LCD_SPI, &SPI_InitStructure);

        /* Enable L3GD20_SPI  */
        SPI_Cmd(LCD_SPI, ENABLE);

    }
}


/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOn(void)
{
  LCD_WriteCommand(LCD_DISPLAY_ON);
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOff(void)
{
    /* Display Off */
    LCD_WriteCommand(LCD_DISPLAY_OFF);
}


/**
  * @brief  Configure the LCD controller (Power On sequence as described in ILI9341 Datasheet)
  * @param  None
  * @retval None
  */
void LCD_PowerOn(void)
{

/*
    LCD_CtrlLinesWrite(LCD_SPI_RESET_GPIO_PORT,LCD_SPI_RESET_PIN,Bit_SET);
    delay(20000);	
    LCD_CtrlLinesWrite(LCD_SPI_RESET_GPIO_PORT,LCD_SPI_RESET_PIN,Bit_RESET);
    delay(20000);	
    LCD_CtrlLinesWrite(LCD_SPI_RESET_GPIO_PORT,LCD_SPI_RESET_PIN,Bit_SET);
	LCD_ChipSelect(ENABLE); 
    delay(20000);		
	LCD_ChipSelect(DISABLE); 
	*/
    LCD_CtrlLinesWrite(LCD_SPI_CS_GPIO_PORT,LCD_SPI_CS_PIN,Bit_SET);
    LCD_CtrlLinesWrite(LCD_SPI_DC_GPIO_PORT,LCD_SPI_DC_PIN,Bit_SET);
    LCD_CtrlLinesWrite(LCD_SPI_RESET_GPIO_PORT,LCD_SPI_RESET_PIN,Bit_RESET);  
    delay(200);		
    LCD_CtrlLinesWrite(LCD_SPI_RESET_GPIO_PORT,LCD_SPI_RESET_PIN,Bit_SET);
    delay(200);	
	LCD_ChipSelect(DISABLE); 
    delay(20000);	   
	LCD_ChipSelect(ENABLE); 
    LCD_WriteCommand(0x01); 
    delay(200);	
    LCD_WriteCommand(0x28); 

#if 0
    LCD_WriteCommand(0xCA);
    LCD_WriteData(0xC3);
    LCD_WriteData(0x08);
    LCD_WriteData(0x50);
    LCD_WriteCommand(LCD_POWERB);
    LCD_WriteData(0x00);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x30);
    LCD_WriteCommand(LCD_POWER_SEQ);
    LCD_WriteData(0x64);
    LCD_WriteData(0x03);
    LCD_WriteData(0x12);
    LCD_WriteData(0x81);
    LCD_WriteCommand(LCD_DTCA);
    LCD_WriteData(0x85);
    LCD_WriteData(0x00);
    LCD_WriteData(0x78);
    LCD_WriteCommand(LCD_POWERA);
    LCD_WriteData(0x39);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x34);
    LCD_WriteData(0x02);
    LCD_WriteCommand(LCD_PRC);
    LCD_WriteData(0x20);
    LCD_WriteCommand(LCD_DTCB);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteCommand(LCD_FRC);
    LCD_WriteData(0x00);
    LCD_WriteData(0x1B);
    LCD_WriteCommand(LCD_DFC);
    LCD_WriteData(0x0A);
    LCD_WriteData(0xA2);
    LCD_WriteCommand(LCD_POWER1);
    LCD_WriteData(0x10);
    LCD_WriteCommand(LCD_POWER2);
    LCD_WriteData(0x10);
    LCD_WriteCommand(LCD_VCOM1);
    LCD_WriteData(0x45);
    LCD_WriteData(0x15);
    LCD_WriteCommand(LCD_VCOM2);
    LCD_WriteData(0x90);
    LCD_WriteCommand(LCD_MAC);
    LCD_WriteData(0xC8);
    LCD_WriteCommand(LCD_3GAMMA_EN);
    LCD_WriteData(0x00);
    LCD_WriteCommand(LCD_RGB_INTERFACE);
    LCD_WriteData(0xC2);
    LCD_WriteCommand(LCD_DFC);
    LCD_WriteData(0x0A);
    LCD_WriteData(0xA7);
    LCD_WriteData(0x27);
    LCD_WriteData(0x04);

    /* colomn address set */
    LCD_WriteCommand(LCD_COLUMN_ADDR);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0xEF);
    /* Page Address Set */
    LCD_WriteCommand(LCD_PAGE_ADDR);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x01);
    LCD_WriteData(0x3F);
    LCD_WriteCommand(LCD_INTERFACE);
    LCD_WriteData(0x01);
    LCD_WriteData(0x00);
    LCD_WriteData(0x06);

    LCD_WriteCommand(LCD_GRAM);
    delay(200);

    LCD_WriteCommand(LCD_GAMMA);
    LCD_WriteData(0x01);

    LCD_WriteCommand(LCD_PGAMMA);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x29);
    LCD_WriteData(0x24);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x09);
    LCD_WriteData(0x4E);
    LCD_WriteData(0x78);
    LCD_WriteData(0x3C);
    LCD_WriteData(0x09);
    LCD_WriteData(0x13);
    LCD_WriteData(0x05);
    LCD_WriteData(0x17);
    LCD_WriteData(0x11);
    LCD_WriteData(0x00);
    LCD_WriteCommand(LCD_NGAMMA);
    LCD_WriteData(0x00);
    LCD_WriteData(0x16);
    LCD_WriteData(0x1B);
    LCD_WriteData(0x04);
    LCD_WriteData(0x11);
    LCD_WriteData(0x07);
    LCD_WriteData(0x31);
    LCD_WriteData(0x33);
    LCD_WriteData(0x42);
    LCD_WriteData(0x05);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0A);
    LCD_WriteData(0x28);
    LCD_WriteData(0x2F);
    LCD_WriteData(0x0F);

    LCD_WriteCommand(LCD_SLEEP_OUT);
    delay(200);
    LCD_WriteCommand(LCD_DISPLAY_ON);
    /* GRAM start writing */
    LCD_WriteCommand(LCD_GRAM);
#endif

#if 0
     LCD_WriteCommand(0xCB);  
     LCD_WriteData(0x39); 
     LCD_WriteData(0x2C); 
     LCD_WriteData(0x00); 
     LCD_WriteData(0x34); 
     LCD_WriteData(0x02); 
 
     LCD_WriteCommand(0xCF);  
     LCD_WriteData(0x00); 
     LCD_WriteData(0XC1); 
     LCD_WriteData(0X30); 
 
     LCD_WriteCommand(0xE8);  
     LCD_WriteData(0x85); 
     LCD_WriteData(0x00); 
     LCD_WriteData(0x78); 
 
     LCD_WriteCommand(0xEA);  
     LCD_WriteData(0x00); 
     LCD_WriteData(0x00); 
 
     LCD_WriteCommand(0xED);  
     LCD_WriteData(0x64); 
     LCD_WriteData(0x03); 
     LCD_WriteData(0X12); 
     LCD_WriteData(0X81); 
 
     LCD_WriteCommand(0xF7);  
     LCD_WriteData(0x20); 
 
     LCD_WriteCommand(0xC0);    //Power control 
     LCD_WriteData(0x23);   //VRH[5:0] 
 
     LCD_WriteCommand(0xC1);    //Power control 
     LCD_WriteData(0x10);   //SAP[2:0];BT[3:0] 
 
     LCD_WriteCommand(0xC5);    //VCM control 
     LCD_WriteData(0x3e); //对比度调节
     LCD_WriteData(0x28); 
 
     LCD_WriteCommand(0xC7);    //VCM control2 
     LCD_WriteData(0x86);  //--
 
     LCD_WriteCommand(0x36);    // Memory Access Control 
     LCD_WriteData(0x48); //C8       //48 68竖屏//28 E8 横屏
 
     LCD_WriteCommand(0x3A);    
     LCD_WriteData(0x55); 
 
     LCD_WriteCommand(0xB1);    
     LCD_WriteData(0x00);  
     LCD_WriteData(0x18); 
 
     LCD_WriteCommand(0xB6);    // Display Function Control 
     LCD_WriteData(0x08); 
     LCD_WriteData(0x82);
     LCD_WriteData(0x27);  
 
     LCD_WriteCommand(0xF2);    // 3Gamma Function Disable 
     LCD_WriteData(0x00); 
 
     LCD_WriteCommand(0x26);    //Gamma curve selected 
     LCD_WriteData(0x01); 
 
     LCD_WriteCommand(0xE0);    //Set Gamma 
     LCD_WriteData(0x0F); 
     LCD_WriteData(0x31); 
     LCD_WriteData(0x2B); 
     LCD_WriteData(0x0C); 
     LCD_WriteData(0x0E); 
     LCD_WriteData(0x08); 
     LCD_WriteData(0x4E); 
     LCD_WriteData(0xF1); 
     LCD_WriteData(0x37); 
     LCD_WriteData(0x07); 
     LCD_WriteData(0x10); 
     LCD_WriteData(0x03); 
     LCD_WriteData(0x0E); 
     LCD_WriteData(0x09); 
     LCD_WriteData(0x00); 
 
     LCD_WriteCommand(0XE1);    //Set Gamma 
     LCD_WriteData(0x00); 
     LCD_WriteData(0x0E); 
     LCD_WriteData(0x14); 
     LCD_WriteData(0x03); 
     LCD_WriteData(0x11); 
     LCD_WriteData(0x07); 
     LCD_WriteData(0x31); 
     LCD_WriteData(0xC1); 
     LCD_WriteData(0x48); 
     LCD_WriteData(0x08); 
     LCD_WriteData(0x0F); 
     LCD_WriteData(0x0C); 
     LCD_WriteData(0x31); 
     LCD_WriteData(0x36); 
     LCD_WriteData(0x0F); 
 
     LCD_WriteCommand(0x11);    //Exit Sleep 
     delay(120); 
             
     LCD_WriteCommand(0x29);    //Display on 
     LCD_WriteCommand(0x2c); 
#endif

#if 1


/* Start Initial Sequence ----------------------------------------------------*/
 LCD_WriteCommand(0xCF);                     
 LCD_WriteData(0x00);
 LCD_WriteData(0x83);
 LCD_WriteData(0x30);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xED);                     
 LCD_WriteData(0x64);
 LCD_WriteData(0x03);
 LCD_WriteData(0x12);
 LCD_WriteData(0x81);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xE8);                     
 LCD_WriteData(0x85);
 LCD_WriteData(0x01);
 LCD_WriteData(0x79);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xCB);                     
 LCD_WriteData(0x39);
 LCD_WriteData(0x2C);
 LCD_WriteData(0x00);
 LCD_WriteData(0x34);
 LCD_WriteData(0x02);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xF7);                     
 LCD_WriteData(0x20);
 LCD_ChipSelect(DISABLE);
       
 LCD_WriteCommand(0xEA);                     
 LCD_WriteData(0x00);
 LCD_WriteData(0x00);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xC0);                     // POWER_CONTROL_1
 LCD_WriteData(0x26);
 LCD_ChipSelect(DISABLE);

 LCD_WriteCommand(0xC1);                     // POWER_CONTROL_2
 LCD_WriteData(0x11);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xC5);                     // VCOM_CONTROL_1
 LCD_WriteData(0x35);
 LCD_WriteData(0x3E);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0xC7);                     // VCOM_CONTROL_2
 LCD_WriteData(0xBE);
 LCD_ChipSelect(DISABLE);
 
 LCD_WriteCommand(0x36);                     // MEMORY_ACCESS_CONTROL
 LCD_WriteData(0x48);
 LCD_ChipSelect(DISABLE);


/*
 LCD_WriteCommand(0x33);                     
 LCD_WriteData(0x00);
 LCD_WriteData(0x00);
 LCD_WriteData(0x01);
 LCD_WriteData(0x40);
 LCD_WriteData(0x00);
 LCD_WriteData(0x00);
 LCD_ChipSelect(DISABLE);


 LCD_WriteCommand(0x37);                     
 LCD_WriteData(0x00);
 LCD_WriteData(0x00);
 LCD_ChipSelect(DISABLE);
*/
 
 LCD_WriteCommand(0x3A);                     // COLMOD_PIXEL_FORMAT_SET
 LCD_WriteData(0x55);
 LCD_ChipSelect(DISABLE);// 16 bit pixel 
 
 LCD_WriteCommand(0xB1);                     // Frame Rate
 LCD_WriteData(0x00);
 LCD_WriteData(0x1B);               
 LCD_ChipSelect(DISABLE);

 
 LCD_WriteCommand(0xF2);                     // Gamma Function Disable
 LCD_WriteData(0x08);
 LCD_ChipSelect(DISABLE);

 
 LCD_WriteCommand(0x26);                     
 LCD_WriteData(0x01);                 // gamma set for curve 01/2/04/08
 LCD_ChipSelect(DISABLE);

 
 LCD_WriteCommand(0xE0);                     // positive gamma correction
 LCD_WriteData(0x1F); 
 LCD_WriteData(0x1A); 
 LCD_WriteData(0x18); 
 LCD_WriteData(0x0A); 
 LCD_WriteData(0x0F); 
 LCD_WriteData(0x06); 
 LCD_WriteData(0x45); 
 LCD_WriteData(0x87); 
 LCD_WriteData(0x32); 
 LCD_WriteData(0x0A); 
 LCD_WriteData(0x07); 
 LCD_WriteData(0x02); 
 LCD_WriteData(0x07);
 LCD_WriteData(0x05); 
 LCD_WriteData(0x00);
 LCD_ChipSelect(DISABLE);

 
 LCD_WriteCommand(0xE1);                     // negativ gamma correction
 LCD_WriteData(0x00); 
 LCD_WriteData(0x25); 
 LCD_WriteData(0x27); 
 LCD_WriteData(0x05); 
 LCD_WriteData(0x10); 
 LCD_WriteData(0x09); 
 LCD_WriteData(0x3A); 
 LCD_WriteData(0x78); 
 LCD_WriteData(0x4D); 
 LCD_WriteData(0x05); 
 LCD_WriteData(0x18); 
 LCD_WriteData(0x0D); 
 LCD_WriteData(0x38);
 LCD_WriteData(0x3A); 
 LCD_WriteData(0x1F);
 LCD_ChipSelect(DISABLE);

 
 WindowMax ();
 
 //LCD_WriteCommand(0x34);                     // tearing effect off
 //_cs = 1;
 
 //LCD_WriteCommand(0x35);                     // tearing effect on
 //_cs = 1;
  
 LCD_WriteCommand(0xB7);                       // entry mode
 LCD_WriteData(0x07);
 LCD_ChipSelect(DISABLE);


 
 LCD_WriteCommand(0xB6);                       // display function control
 LCD_WriteData(0x0A);
 LCD_WriteData(0x82);
 LCD_WriteData(0x27);
 LCD_WriteData(0x00);
 LCD_ChipSelect(DISABLE);


 
 LCD_WriteCommand(0x11);                     // sleep out
 
 delay(2000);
 
 //LCD_WriteCommand(0x29);                     // display on

 //delay(2000);
 LCD_DisplayOff();
 delay(2000);
 LCD_Clear(LCD_COLOR_BLACK);
 LCD_Clear(LCD_COLOR_WHITE);
 LCD_DisplayOn();

#endif

 }

/******************绘图函数****************/

void circle(int x0, int y0, int r, int color)
{

    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        LCD_PutPixel(x0-x, y0+y,color);
        LCD_PutPixel(x0+x, y0+y,color);
        LCD_PutPixel(x0+x, y0-y,color);
        LCD_PutPixel(x0-x, y0-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);

}

void fillcircle(int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        vline(x0-x, y0-y, y0+y, color);
        vline(x0+x, y0-y, y0+y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}






void line(int x0, int y0, int x1, int y1, int color)
{
    //WindowMax();
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    if (dx == 0) {        /* vertical line */
        if (y1 > y0) vline(x0,y0,y1,color);
        else vline(x0,y1,y0,color);
        return;
    }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    if (dy == 0) {        /* horizontal line */
        if (x1 > x0) hline(x0,x1,y0,color);
        else  hline(x1,x0,y0,color);
        return;
    }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            LCD_PutPixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        LCD_PutPixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            LCD_PutPixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        LCD_PutPixel(x0, y0, color);
    }
    return;
}


void rect(int x0, int y0, int x1, int y1, int color)
{

    if (x1 > x0) hline(x0,x1,y0,color);
    else  hline(x1,x0,y0,color);

    if (y1 > y0) vline(x0,y0,y1,color);
    else vline(x0,y1,y0,color);

    if (x1 > x0) hline(x0,x1,y1,color);
    else  hline(x1,x0,y1,color);

    if (y1 > y0) vline(x1,y0,y1,color);
    else vline(x1,y1,y0,color);

    return;
}



void hline(int x0, int x1, int y, int color)
{
    int w;   
    int j;
    w = x1 - x0 + 1;
    LCD_SetWindow(x0,y,x1,y);
    LCD_WriteCommand(LCD_GRAM);  // send pixel
#ifdef LCD_SPI_8b
    for (j=0; j<w; j++) {
        LCD_WriteData(color >> 8);
        LCD_WriteData(color & 0xff);
    } 
#else
        LCD_SPI_16BIT();
        for (j=0; j<w; j++) {
            LCD_WriteData(color);    
        } 
        LCD_SPI_8BIT();
#endif 
    LCD_ChipSelect(DISABLE);
    WindowMax();
    return;
}

void vline(int x, int y0, int y1, int color)
{
    int h;  
    int j;
    h = y1-y0+1;

    LCD_SetWindow(x,y0,x,y1);
    LCD_WriteCommand(LCD_GRAM);  // send pixel
#if LCD_SPI_8b
    for (j=0; j<h; j++) {
        LCD_WriteData(color >> 8);
        LCD_WriteData(color & 0xff);
    } 
#else
    LCD_SPI_16BIT();
    for (j=0; j<h; j++) {
        LCD_WriteData(color);
    } 
    LCD_SPI_8BIT();
#endif    
    LCD_ChipSelect(DISABLE);
    WindowMax();
    return;
}



void fillrect(int x0, int y0, int x1, int y1, int color)
{
    int p = 0;
    int h = y1 - y0 + 1;
    int w = x1 - x0 + 1;
    int pixel = h * w;
    LCD_SetWindow(x0,y0,x1,y1);
    LCD_WriteCommand(LCD_GRAM);  // send pixel 
    #if LCD_SPI_8b  // 8 Bit SPI
    for (p=0; p<pixel; p++) {
        LCD_WriteData(color >> 8);
        LCD_WriteData(color & 0xff);
    }
   #else
    LCD_SPI_16BIT();                            // switch to 16 bit Mode 3
    for (p=0; p<pixel; p++) {
        LCD_WriteData(color);
    }
    LCD_SPI_8BIT();                            // switch to 16 bit Mode 3
    #endif
    LCD_ChipSelect(DISABLE);
    WindowMax();
    return;
}



/***************************************/

void LCD_SPI_Init(void)
{

    /* Configure the LCD Control pins ------------------------------------------*/
    LCD_CtrlLinesConfig();
    LCD_ChipSelect(ENABLE);
    LCD_ChipSelect(DISABLE);

    /* Configure the LCD_SPI interface -----------------------------------------*/
    LCD_SPIConfig(); 

    /* Power on the LCD --------------------------------------------------------*/
    LCD_PowerOn();

    //Lcd_Data_Test();

    
    //fillrect(0,0,239,319,LCD_COLOR_WHITE);
  //  fillrect(0,0,239,319,LCD_COLOR_WHITE);

    circle(40,40,30, LCD_COLOR_RED);
    fillcircle(120,160,50,LCD_COLOR_GREEN);
    circle(200,240,30, LCD_COLOR_BLACK);

    rect(180,50,200,70,LCD_COLOR_MAGENTA);

    vline(120,0,320,LCD_COLOR_BLUE);
    hline(0,239,160,LCD_COLOR_BLUE);
   // lcd_read_Id();

    //rt_hw_lcd_set_pixel(const char * c,int x,int y);
}



/*  设置像素点 颜色,X,Y */
void rt_hw_lcd_set_pixel(const char* c, int x, int y)
{

    uint16_t p;
    p =  *(uint16_t *)c;

    LCD_Set_Cursor(x, y);
    LCD_WriteCommand(LCD_GRAM);
    LCD_WriteData(p);

}

/* 获取像素点颜色 */
void rt_hw_lcd_get_pixel(char* c, int x, int y)
{
    LCD_Set_Cursor(x, y);
    //LCD_WriteCommand(0x2e);
    *(uint16_t*)c = LCD_ReadByte(0x2e);

}


/* 画水平线 */
void rt_hw_lcd_draw_hline(const char* c, int x1, int x2, int y)
{

    uint16_t p;

    p = *(uint16_t *)c;

    LCD_Set_Cursor(x1, y);
    LCD_WriteCommand(LCD_GRAM);
    while (x1 < x2)
    {
        LCD_WriteData(p);
        x1++;
    }


}

/* 垂直线 */
void rt_hw_lcd_draw_vline(const char* c, int x, int y1, int y2)
{

    uint16_t p;

    p = *(uint16_t *)c;

    LCD_Set_Cursor(x, y1);
    LCD_WriteCommand(LCD_GRAM);
    while (y1 < y2)
    {
        LCD_WriteData(p);
        y1++;
    }

}

/* ?? */
void rt_hw_lcd_draw_blit_line(const char* c, int x, int y, unsigned long size)
{
    uint16_t *ptr;
    ptr = (uint16_t*)c;

    LCD_Set_Cursor(x, y);
    LCD_WriteCommand(0x2c);
    while (size)
    {
        LCD_WriteData(*ptr ++);
        size--;
    }
}


/**********************************************/







/********RT-Thread  driver******************************/

struct rt_device _lcd_device;	 //设备框架结构




struct rt_device_graphic_ops lcd_ili_ops =
{
    rt_hw_lcd_set_pixel,
    rt_hw_lcd_get_pixel,
    rt_hw_lcd_draw_hline,
    rt_hw_lcd_draw_vline,
    rt_hw_lcd_draw_blit_line
};


static rt_err_t lcd_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case RTGRAPHIC_CTRL_GET_INFO:
    {
        struct rt_device_graphic_info *info;

        info = (struct rt_device_graphic_info*) args;
        RT_ASSERT(info != RT_NULL);

        info->bits_per_pixel = 16;
        info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565P;
        info->framebuffer = RT_NULL;

        info->width = LCD_WIDTH;
        info->height = LCD_HEIGHT;

    }
    break;

    case RTGRAPHIC_CTRL_RECT_UPDATE:
        /* nothong to be done */
        break;

    default:
        break;
    }

    return RT_EOK;
}

/* 需直接调用的 用于硬件初始化和注册设备 */
int ili9341_init(void)
{
    /* register lcd device */
    _lcd_device.type  = RT_Device_Class_Graphic;
    _lcd_device.init  = lcd_init;
    _lcd_device.open  = lcd_open;
    _lcd_device.close = lcd_close;
    _lcd_device.control = lcd_control;
    _lcd_device.read  = RT_NULL;
    _lcd_device.write = RT_NULL;

    _lcd_device.user_data = &lcd_ili_ops;

    LCD_SPI_Init();

    /* register graphic device driver */
    rt_device_register(&_lcd_device, "lcd",
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
    return 0;
}

INIT_DEVICE_EXPORT(ili9341_init);

