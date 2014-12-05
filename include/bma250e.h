/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : bma250e.c
 @brief  : bma250e chip spi driver. ref l3gd20.h
 @author : wanglei
 @history:
           2014-8-11    wanglei       Created file
           ...
******************************************************************************/



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F401_DISCOVERY_BMA250E_H
#define __STM32F401_DISCOVERY_BMA250E_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
 #include "stm32f4xx.h"


#define BMA250E_Sensitivity_2g    (float)0.038344f        /* m/s2 */
#define BMA250E_Sensitivity_4g    (float)0.076688f        
#define BMA250E_Sensitivity_8g    (float)0.153376f         


/* BMA250E struct */
typedef struct
{
  uint8_t Power_Mode;                         /* Power-down/Sleep/Normal Mode */
  uint8_t Output_DataRate;                    /* OUT data rate */
  uint8_t Axes_Enable;                        /* Axes enable */
  uint8_t Band_Width;                         /* Bandwidth selection */
  uint8_t BlockData_Update;                   /* Block Data Update */
  uint8_t Endianness;                         /* Endian Data selection */
  uint8_t Full_Scale;                         /* Full Scale selection */
}BMA250E_InitTypeDef;

/* BMA250E High Pass Filter struct */
typedef struct
{
  uint8_t HighPassFilter_Mode_Selection;      /* Internal filter mode */
  uint8_t HighPassFilter_CutOff_Frequency;    /* High pass filter cut-off frequency */
}BMA250E_FilterConfigTypeDef;

/* BMA250E Interrupt struct */
typedef struct
{
  uint8_t Latch_Request;                      /* Latch interrupt request into CLICK_SRC register */
  uint8_t Interrupt_Axes;                     /* X, Y, Z Axes Interrupts */ 
  uint8_t Interrupt_ActiveEdge;               /*  Interrupt Active edge */
}BMA250E_InterruptConfigTypeDef;  

  
/* Read/Write command */
#define READWRITE_CMD              ((uint8_t)0x80) 
/* Multiple byte read/write command */ 
#define MULTIPLEBYTE_CMD           ((uint8_t)0x40)
/* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define DUMMY_BYTE                 ((uint8_t)0xff)

/**
  * @brief  BMA250E SPI Interface pins
  */
#define BMA250E_SPI                       SPI2
#define BMA250E_SPI_CLK                   RCC_APB1Periph_SPI2

#define BMA250E_SPI_SCK_PIN               GPIO_Pin_13                 /* PB.13 */
#define BMA250E_SPI_SCK_GPIO_PORT         GPIOB                       /* GPIOB */
#define BMA250E_SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOB
#define BMA250E_SPI_SCK_SOURCE            GPIO_PinSource13
#define BMA250E_SPI_SCK_AF                GPIO_AF_SPI2

#define BMA250E_SPI_MISO_PIN              GPIO_Pin_14                 /* PB.14 */
#define BMA250E_SPI_MISO_GPIO_PORT        GPIOB                       /* GPIOB */
#define BMA250E_SPI_MISO_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define BMA250E_SPI_MISO_SOURCE           GPIO_PinSource14
#define BMA250E_SPI_MISO_AF               GPIO_AF_SPI2

#define BMA250E_SPI_MOSI_PIN              GPIO_Pin_15                 /* PB.15 */
#define BMA250E_SPI_MOSI_GPIO_PORT        GPIOB                       /* GPIOB */
#define BMA250E_SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define BMA250E_SPI_MOSI_SOURCE           GPIO_PinSource15
#define BMA250E_SPI_MOSI_AF               GPIO_AF_SPI2

#define BMA250E_SPI_CS_PIN                GPIO_Pin_12                 /* PB.12 */
#define BMA250E_SPI_CS_GPIO_PORT          GPIOB                       /* GPIOB */
#define BMA250E_SPI_CS_GPIO_CLK           RCC_AHB1Periph_GPIOB

#define BMA250E_SPI_INT1_PIN              GPIO_Pin_1                  /* PB.01 */
#define BMA250E_SPI_INT1_GPIO_PORT        GPIOB                       /* GPIOC */
#define BMA250E_SPI_INT1_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define BMA250E_SPI_INT1_EXTI_LINE        EXTI_Line3
#define BMA250E_SPI_INT1_EXTI_PORT_SOURCE EXTI_PortSourceGPIOB
#define BMA250E_SPI_INT1_EXTI_PIN_SOURCE  EXTI_PinSource1
#define BMA250E_SPI_INT1_EXTI_IRQn        EXTI1_IRQn 

#define BMA250E_SPI_INT2_PIN              GPIO_Pin_2                  /* PB.02 */
#define BMA250E_SPI_INT2_GPIO_PORT        GPIOB                       /* GPIOB */
#define BMA250E_SPI_INT2_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define BMA250E_SPI_INT2_EXTI_LINE        EXTI_Line2
#define BMA250E_SPI_INT2_EXTI_PORT_SOURCE EXTI_PortSourceGPIOB
#define BMA250E_SPI_INT2_EXTI_PIN_SOURCE  EXTI_PinSource2
#define BMA250E_SPI_INT2_EXTI_IRQn        EXTI2_IRQn 


/******************************************************************************/
/*************************** START REGISTER MAPPING  **************************/
/******************************************************************************/
#define BMA250E_WHO_AM_I_ADDR          0x00  /* device identification register */
#define BMA250E_OUT_X_L_ADDR           0x02  /* Output Register X */
#define BMA250E_OUT_X_H_ADDR           0x03  /* Output Register X */
#define BMA250E_OUT_Y_L_ADDR           0x04  /* Output Register Y */
#define BMA250E_OUT_Y_H_ADDR           0x05  /* Output Register Y */
#define BMA250E_OUT_Z_L_ADDR           0x06  /* Output Register Z */
#define BMA250E_OUT_Z_H_ADDR           0x07  /* Output Register Z */ 

#define BMA250E_PMU_RANGE              0x0F
#define BMA250E_PMU_BW                 0x10
#define BMA250E_PMU_LPW                0x11
#define BMA250E_LOW_POWER              0x12
#define BMA250E_ACC_HBW                0x13

#define BMA250E_BGW_SPI3_WDT           0x34  /* spi 4 wire mode */
#define BMA250E_OFC_CTRL               0x36  /* enable x,y,z  */


#define BMA250E_CTRL_REG1_ADDR         0x01
#define BMA250E_CTRL_REG2_ADDR         0x02
#define BMA250E_CTRL_REG3_ADDR         0x03
#define BMA250E_CTRL_REG4_ADDR         0x04
#define BMA250E_CTRL_REG5_ADDR         0x05

/******************************************************************************/
/**************************** END REGISTER MAPPING  ***************************/
/******************************************************************************/

#define BMA250E_FLAG_TIMEOUT     ((uint32_t)0x1000)

#define I_AM_BMA250E		    ((uint8_t)0xD4)

/** @defgroup Power_Mode_selection 
  * @{
  */
#define BMA250E_MODE_POWERDOWN       ((uint8_t)0x00)
#define BMA250E_MODE_ACTIVE          ((uint8_t)0x08)
/**
  * @}
  */

/** @defgroup OutPut_DataRate_Selection 
  * @{
  */
#define BMA250E_OUTPUT_DATARATE_1    ((uint8_t)0x00)
#define BMA250E_OUTPUT_DATARATE_2    ((uint8_t)0x40)
#define BMA250E_OUTPUT_DATARATE_3    ((uint8_t)0x80)
#define BMA250E_OUTPUT_DATARATE_4    ((uint8_t)0xC0)
/**
  * @}
  */

/** @defgroup Axes_Selection 
  * @{
  */
#define BMA250E_X_ENABLE            ((uint8_t)0x02)
#define BMA250E_Y_ENABLE            ((uint8_t)0x01)
#define BMA250E_Z_ENABLE            ((uint8_t)0x04)
#define BMA250E_AXES_ENABLE         ((uint8_t)0x07)
#define BMA250E_AXES_DISABLE        ((uint8_t)0x00)
/**
  * @}
  */

/** @defgroup BandWidth_Selection 
  * @{
  */
#define BMA250E_BANDWIDTH_1         ((uint8_t)0x00)
#define BMA250E_BANDWIDTH_2         ((uint8_t)0x10)
#define BMA250E_BANDWIDTH_3         ((uint8_t)0x20)
#define BMA250E_BANDWIDTH_4         ((uint8_t)0x30)
/**
  * @}
  */

/** @defgroup Full_Scale_Selection 
  * @{
  */
#define BMA250E_FULLSCALE_2g               ((uint8_t)0x03)
#define BMA250E_FULLSCALE_4g               ((uint8_t)0x05)
#define BMA250E_FULLSCALE_8g               ((uint8_t)0x08) 
#define BMA250E_FULLSCALE_16g              ((uint8_t)0x0C) 
/**
  * @}
  */
  
/** @defgroup Block_Data_Update 
  * @{
  */  
#define BMA250E_BlockDataUpdate_Continous   ((uint8_t)0x00)
#define BMA250E_BlockDataUpdate_Single      ((uint8_t)0x80)
/**
  * @}
  */
  
/** @defgroup Endian_Data_selection
  * @{
  */  
#define BMA250E_BLE_LSB                     ((uint8_t)0x00)
#define BMA250E_BLE_MSB	                   ((uint8_t)0x40)
/**
  * @}
  */
  
/** @defgroup High_Pass_Filter_status 
  * @{
  */   
#define BMA250E_HIGHPASSFILTER_DISABLE      ((uint8_t)0x00)
#define BMA250E_HIGHPASSFILTER_ENABLE	     ((uint8_t)0x10)
/**
  * @}
  */

/** @defgroup INT1_Interrupt_status 
  * @{
  */   
#define BMA250E_INT1INTERRUPT_DISABLE       ((uint8_t)0x00)
#define BMA250E_INT1INTERRUPT_ENABLE	   ((uint8_t)0x80)
/**
  * @}
  */

/** @defgroup INT2_Interrupt_status 
  * @{
  */   
#define BMA250E_INT2INTERRUPT_DISABLE       ((uint8_t)0x00)
#define BMA250E_INT2INTERRUPT_ENABLE	   ((uint8_t)0x08)
/**
  * @}
  */

/** @defgroup INT1_Interrupt_ActiveEdge 
  * @{
  */   
#define BMA250E_INT1INTERRUPT_LOW_EDGE      ((uint8_t)0x20)
#define BMA250E_INT1INTERRUPT_HIGH_EDGE     ((uint8_t)0x00)
/**
  * @}
  */
  
/** @defgroup Boot_Mode_selection 
  * @{
  */
#define BMA250E_BOOT_NORMALMODE             ((uint8_t)0x00)
#define BMA250E_BOOT_REBOOTMEMORY           ((uint8_t)0x80)
/**
  * @}
  */  
 
/** @defgroup High_Pass_Filter_Mode 
  * @{
  */   
#define BMA250E_HPM_NORMAL_MODE_RES         ((uint8_t)0x00)
#define BMA250E_HPM_REF_SIGNAL              ((uint8_t)0x10)
#define BMA250E_HPM_NORMAL_MODE             ((uint8_t)0x20)
#define BMA250E_HPM_AUTORESET_INT           ((uint8_t)0x30)
/**
  * @}
  */

/** @defgroup High_Pass_CUT OFF_Frequency 
  * @{
  */   
#define BMA250E_HPFCF_0              0x00
#define BMA250E_HPFCF_1              0x01
#define BMA250E_HPFCF_2              0x02
#define BMA250E_HPFCF_3              0x03
#define BMA250E_HPFCF_4              0x04
#define BMA250E_HPFCF_5              0x05
#define BMA250E_HPFCF_6              0x06
#define BMA250E_HPFCF_7              0x07
#define BMA250E_HPFCF_8              0x08
#define BMA250E_HPFCF_9              0x09
/**
  * @}
  */


/** @defgroup STM32F401_DISCOVERY_BMA250E_Exported_Macros
  * @{
  */
#define BMA250E_CS_LOW()       GPIO_ResetBits(BMA250E_SPI_CS_GPIO_PORT, BMA250E_SPI_CS_PIN)
#define BMA250E_CS_HIGH()      GPIO_SetBits(BMA250E_SPI_CS_GPIO_PORT, BMA250E_SPI_CS_PIN)
/**
  * @}
  */
 
/** @defgroup STM32F401_DISCOVERY_BMA250E_Exported_Functions
  * @{
  */
/* Sensor Configuration Functions */ 
void BMA250E_Init(BMA250E_InitTypeDef *BMA250E_InitStruct);
void BMA250E_RebootCmd(void);

/*INT1 Interrupt Configuration Functions */
void BMA250E_INT1InterruptCmd(uint8_t InterruptState);
void BMA250E_INT2InterruptCmd(uint8_t InterruptState);
void BMA250E_INT1InterruptConfig(BMA250E_InterruptConfigTypeDef *BMA250E_IntConfigStruct);
uint8_t BMA250E_GetDataStatus(void);

/* High Pass Filter Configuration Functions */
void BMA250E_FilterConfig(BMA250E_FilterConfigTypeDef *BMA250E_FilterStruct);
void BMA250E_FilterCmd(uint8_t HighPassFilterState);
void BMA250E_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);
void BMA250E_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);

void BMA250E_LowLevel_Init(void) ;
/* USER Callbacks: This is function for which prototype only is declared in
   MEMS accelerometre driver and that should be implemented into user applicaiton. */  
/* LSM303DLHC_TIMEOUT_UserCallback() function is called whenever a timeout condition 
   occure during communication (waiting transmit data register empty flag(TXE)
   or waiting receive data register is not empty flag (RXNE)).
   You can use the default timeout callback implementation by uncommenting the 
   define USE_DEFAULT_TIMEOUT_CALLBACK in stm32f401_discovery_LSM303DLHC.h file.
   Typically the user implementation of this callback should reset MEMS peripheral
   and re-initialize communication or in worst case reset all the application. */
uint32_t BMA250E_TIMEOUT_UserCallback(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F401_DISCOVERY_BMA250E_H */
