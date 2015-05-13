/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : pt8211.h
 @brief  : pt8211 i2s driver function 
 @author : gexueyuan
 @history:
           2015-1-14    gexueyuan    Created file
           ...
******************************************************************************/
#ifndef	__PT8211_H__
#define	__PT8211_H__

#include "stm32f4xx.h"
#include "rtthread.h"
#include "board.h"
/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
    
/* Uncomment defines below to select standard for audio communication between 
  Codec and I2S peripheral */
//#define I2S_STANDARD_PHILLIPS
/* #define I2S_STANDARD_MSB */
// #define I2S_STANDARD_LSB



/* Uncomment the defines below to select if the Master clock mode should be 
  enabled or not */
//#define CODEC_MCLK_ENABLED
#define CODEC_MCLK_DISABLED




/*------------------------------------
             CONFIGURATION: Audio Codec Driver Configuration parameters
                                      ----------------------------------------*/
/* Audio Transfer mode (DMA, Interrupt or Polling) */
#define AUDIO_MAL_MODE_NORMAL         /* Uncomment this line to enable the audio 
                                         Transfer using DMA */
/* #define AUDIO_MAL_MODE_CIRCULAR */ /* Uncomment this line to enable the audio 
                                         Transfer using DMA */

/* For the DMA modes select the interrupt that will be used */
#define AUDIO_MAL_DMA_IT_TC_EN   /* Uncomment this line to enable DMA Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_HT_EN */  /* Uncomment this line to enable DMA Half Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_TE_EN */  /* Uncomment this line to enable DMA Transfer Error interrupt */

/* Select the interrupt preemption priority and subpriority for the DMA interrupt */
#define EVAL_AUDIO_IRQ_PREPRIO           0   /* Select the preemption priority level(0 is the highest) */
#define EVAL_AUDIO_IRQ_SUBRIO            0   /* Select the sub-priority level (0 is the highest) */


/*-----------------------------------
                    Hardware Configuration defines parameters
                                     -----------------------------------------*/
/* I2S peripheral configuration defines */
#define I2S                      SPI3
#define I2S_CLK                  RCC_APB1Periph_SPI3
#define I2S_ADDRESS              0x40003C0C
#define I2S_GPIO_AF              GPIO_AF_SPI3
#define I2S_IRQ                  SPI3_IRQn
#define I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC)

#define I2S_WS_PIN               GPIO_Pin_4
#define I2S_SCK_PIN              GPIO_Pin_10
#define I2S_SD_PIN               GPIO_Pin_12
#define I2S_MCK_PIN              GPIO_Pin_6

#define I2S_WS_PINSRC            GPIO_PinSource4
#define I2S_SCK_PINSRC           GPIO_PinSource10
#define I2S_SD_PINSRC            GPIO_PinSource12
#define I2S_MCK_PINSRC           GPIO_PinSource6

#define I2S_WS_GPIO              GPIOA
#define I2S_CK_GPIO              GPIOC
#define I2S_SD_GPIO              GPIOC   
#define I2S_MCK_GPIO             GPIOC


#define I2S_EN_GPIO              GPIOC
#define I2S_EN_PIN               GPIO_Pin_3       

/* I2S DMA Stream definitions */
#define AUDIO_MAL_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_MAL_DMA_STREAM           DMA1_Stream5
#define AUDIO_MAL_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_MAL_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_MAL_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define AUDIO_MAL_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define AUDIO_MAL_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define AUDIO_MAL_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define AUDIO_MAL_DMA_FLAG_DME         DMA_FLAG_DMEIF5
#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord
#define DMA_MAX_SZE                    0xFFFF

#define Audio_MAL_IRQHandler           DMA1_Stream5_IRQHandler

 
#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1
 
 /* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2
 
 /* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0
 /*----------------------------------------------------------------------------*/



#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)



void Pt8211_AUDIO_Init(uint32_t AudioFreq);
void  Pt8211_AUDIO_DeInit(void);

uint32_t Pt8211_AUDIO_Play(uint16_t* pBuffer, uint32_t Size);
void  Pt8211_AUDIO_PauseResume(uint32_t Cmd);

void  Pt8211_AUDIO_Stop(uint32_t Option);



/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
#endif
