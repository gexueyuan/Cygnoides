/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : pt8211.c
 @brief  : pt8211 i2s driver
 @author : gexueyuan
 @history:
           2015-1-14    gexueyuan    Created file
           ...
******************************************************************************/
#include "pt8211.h"


/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/
/* This structure is declared global because it is handled by two different functions */
static DMA_InitTypeDef DMA_InitStructure; 
DMA_InitTypeDef AUDIO_MAL_DMA_InitStructure;

uint32_t AudioTotalSize = 0xFFFF; /* This variable holds the total size of the audio file */
uint32_t AudioRemSize   = 0xFFFF; /* This variable holds the remaining data in audio file */
uint16_t *CurrentPos;             /* This variable holds the current position of audio pointer */

__IO uint8_t OutputDev = 0;

/********test debug*************/
typedef struct
{
	uint8_t ucFmtIdx;			
	uint8_t ucVolume;			
	uint8_t ucMicGain;			
	int16_t *pAudio;				
	uint32_t uiCursor;				
	uint8_t ucStatus;			
} REC_T;



REC_T g_tRec;

extern void release_semadpcm(void);
//g_tRec.ucVolume = 39;

/*
g_tRec.ucVolume = 39;       
g_tRec.ucMicGain = 24;         

//ucRefresh = 1;

g_tRec.ucFmtIdx = 1;        
g_tRec.pAudio = AUDIO_SAMPLE;
*/

/*****************************/
/*----------------------------------------------------------------------------*/

/*-----------------------------------
                   MAL (Media Access Layer) functions 
                                    ------------------------------------------*/
/* Peripherals configuration functions */
static void     Audio_MAL_Init(void);
static void     Audio_MAL_DeInit(void);
static void     Audio_MAL_Play(uint32_t Addr, uint32_t Size);
static void     Audio_MAL_PauseResume(uint32_t Cmd, uint32_t Addr);
static void     Audio_MAL_Stop(void);


/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
void sound_en(uint8_t option)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin =  I2S_EN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd  = option;//GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

}


/**
  * @brief  Initializes the Audio Codec audio interface (I2S)
  * @note   This function assumes that the I2S input clock (through PLL_R in 
  *         Devices RevA/Z and through dedicated PLLI2S_R in Devices RevB/Y)
  *         is already configured and ready to be used.    
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral. 
  * @retval None
  */
static void I2S_AudioInterface_Init(uint32_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;

  /* I2S peripheral configuration */
  SPI_I2S_DeInit(I2S);

  /* Enable the I2S peripheral clock */
  RCC_APB1PeriphClockCmd(I2S_CLK, ENABLE);

  
   /* Enable PLLI2S */
   RCC->CR |= ((uint32_t)RCC_CR_PLLI2SON);
  
     /* Wait till PLLI2S is ready */
     while((RCC->CR & RCC_CR_PLLI2SRDY) == 0)
    {
     }


  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
#ifdef CODEC_MCLK_ENABLED
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
#elif defined(CODEC_MCLK_DISABLED)
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
#else
#error "No selection for the MCLK output has been defined !"
#endif /* CODEC_MCLK_ENABLED */

  /* Initialize the I2S peripheral with the structure above */
  I2S_Init(I2S, &I2S_InitStructure);

  //SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
      
 // SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);


  I2S_Cmd(I2S,ENABLE);

  /* The I2S peripheral will be enabled only in the EVAL_AUDIO_Play() function 
       or by user functions if DMA mode not enabled */  
}




/**
  * @brief  Restores the Audio Codec audio interface to its default state.
  * @param  None
  * @retval None
  */
static void I2S_AudioInterface_DeInit(void)
{
  /* Disable the I2S peripheral (in case it hasn't already been disabled) */
  I2S_Cmd(I2S, DISABLE);
  
  /* Deinitialize the I2S peripheral */
  SPI_I2S_DeInit(I2S);
  
  /* Disable the I2S peripheral clock */
  RCC_APB1PeriphClockCmd(I2S_CLK, DISABLE); 
}

/**
  * @brief Initializes IOs used by the Audio Codec (on the control and audio 
  *        interfaces).
  * @param  None
  * @retval None
  */
static void I2S_GPIO_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable I2S and I2C GPIO clocks */
  RCC_AHB1PeriphClockCmd(I2S_GPIO_CLOCK, ENABLE);

  /* I2S pins configuration: WS, SCK and SD pins -----------------------------*/

  /*CK*/
  GPIO_InitStructure.GPIO_Pin = I2S_SCK_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2S_CK_GPIO, &GPIO_InitStructure);  

  /*WS*/
  GPIO_InitStructure.GPIO_Pin = I2S_WS_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2S_WS_GPIO, &GPIO_InitStructure);
  
  /*SD*/  
  GPIO_InitStructure.GPIO_Pin =  I2S_SD_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2S_SD_GPIO, &GPIO_InitStructure);  
  /* Connect pins to I2S peripheral  */
  GPIO_PinAFConfig(I2S_CK_GPIO, I2S_SCK_PINSRC, I2S_GPIO_AF);  
  GPIO_PinAFConfig(I2S_WS_GPIO, I2S_WS_PINSRC, I2S_GPIO_AF);
  GPIO_PinAFConfig(I2S_SD_GPIO, I2S_SD_PINSRC, I2S_GPIO_AF);


/* I2S pins configuration: MCK pin */
    GPIO_InitStructure.GPIO_Pin = I2S_MCK_PIN; 
#if 0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(I2S_MCK_GPIO, &GPIO_InitStructure);   
    /* Connect pins to I2S peripheral  */
    GPIO_PinAFConfig(I2S_MCK_GPIO, I2S_MCK_PINSRC, I2S_GPIO_AF); 
    /* CODEC_MCLK_ENABLED */   

#else
    /* Sound en enable*/
    GPIO_InitStructure.GPIO_Pin =  I2S_EN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
}

/**
  * @brief  Restores the IOs used by the Audio Codec interface to their default state.
  * @param  None
  * @retval None
  */
static void I2S_GPIO_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Deinitialize all the GPIOs used by the driver (EXCEPT the I2C IOs since 
     they are used by the IOExpander as well) */
  GPIO_InitStructure.GPIO_Pin = I2S_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(I2S_CK_GPIO, &GPIO_InitStructure);    

  GPIO_InitStructure.GPIO_Pin = I2S_WS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(I2S_WS_GPIO, &GPIO_InitStructure);


  GPIO_InitStructure.GPIO_Pin = I2S_SD_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(I2S_SD_GPIO, &GPIO_InitStructure);

  /* Disconnect pins from I2S peripheral  */
  GPIO_PinAFConfig(I2S_CK_GPIO, I2S_WS_PIN, 0x00);  
  GPIO_PinAFConfig(I2S_WS_GPIO, I2S_SCK_PIN, 0x00);
  GPIO_PinAFConfig(I2S_SD_GPIO, I2S_SD_PIN, 0x00);  
  
#ifdef CODEC_MCLK_ENABLED
  /* I2S pins deinitialization: MCK pin */
  GPIO_InitStructure.GPIO_Pin = I2S_MCK_PIN; 
  GPIO_Init(I2S_MCK_GPIO, &GPIO_InitStructure);   
  /* Disconnect pins from I2S peripheral  */
  GPIO_PinAFConfig(I2S_MCK_GPIO, I2S_MCK_PINSRC, I2S_GPIO_AF); 
#endif /* CODEC_MCLK_ENABLED */    
}


static void _I2S_Init(uint32_t AudioFreq)
{ 

  /* Configure the Codec related IOs */
  I2S_GPIO_Init();   

  /* Configure the I2S peripheral */
  I2S_AudioInterface_Init(AudioFreq);  
  
}

/**
  * @brief  Restore the audio codec state to default state and free all used 
  *         resources.
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
static void I2S_DeInit(void)
{

  /* Deinitialize all use GPIOs */
  I2S_GPIO_DeInit();
  
  /* Deinitialize the Codec audio interface (I2S) */
  I2S_AudioInterface_DeInit(); 
  
}


#ifdef USE_I2SNVIC
static void I2S_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = I2S_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
#endif
static void I2S_Mode_Config(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq, uint16_t _usMode)
{
	I2S_InitTypeDef I2S_InitStructure; 

	if ((_usMode == I2S_Mode_SlaveTx) && (_usMode == I2S_Mode_SlaveRx))
	{
		return;
	}

	RCC_APB1PeriphClockCmd(I2S_CLK, ENABLE);
	
	SPI_I2S_DeInit(I2S); 
		
	if (_usMode == I2S_Mode_MasterTx)
	{
		I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;			/* ????I2S1∟℅¯?㏒那? */
		I2S_InitStructure.I2S_Standard = _usStandard;			/* ?車?迆㊣那℅? */
		I2S_InitStructure.I2S_DataFormat = _usWordLen;			/* 那y?Y??那?㏒?16bit */
		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;	/* ?¯那㊣?車?㏒那? */
		I2S_InitStructure.I2S_AudioFreq = _usAudioFreq;			/* 辰??米2谷?迄?米?那 */
		I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  			
		I2S_Init(I2S, &I2S_InitStructure);
	}
	else if (_usMode == I2S_Mode_MasterRx)
	{
		I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;			/* ????I2S1∟℅¯?㏒那? */
		I2S_InitStructure.I2S_Standard = _usStandard;			/* ?車?迆㊣那℅? */
		I2S_InitStructure.I2S_DataFormat = _usWordLen;			/* 那y?Y??那?㏒?16bit */
		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;	/* ?¯那㊣?車?㏒那? */
		I2S_InitStructure.I2S_AudioFreq = _usAudioFreq;			/* 辰??米2谷?迄?米?那 */
		I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  			
		I2S_Init(I2S, &I2S_InitStructure);
	}

	/* ???1I2S2 TXE?D??(﹞⊿?赤?o3?????)㏒?D豕辰a那㊣?迄∩辰?a */ 
	SPI_I2S_ITConfig(I2S, SPI_I2S_IT_TXE, DISABLE);
		
	/* ???1I2S2 RXNE?D??(?車那?2???)㏒?D豕辰a那㊣?迄∩辰?a */ 
	SPI_I2S_ITConfig(I2S, SPI_I2S_IT_RXNE, DISABLE);
	
	/* 那1?邦 SPI2/I2S2 赤a谷豕 */
	I2S_Cmd(I2S, ENABLE);
}

void I2S_StartPlay(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq)
{

	I2S_Mode_Config(_usStandard, I2S_DataFormat_16b, _usAudioFreq, I2S_Mode_MasterTx);

	SPI_I2S_ITConfig(I2S, SPI_I2S_IT_TXE, ENABLE);		/* 那1?邦﹞⊿?赤?D?? */
}


/*
*********************************************************************************************************
*	o‘ 那y ??: I2S_CODEC_DataTransfer
*	1|?邦?米?¯: I2S那y?Y∩?那?o‘那y, ㊣?SPI2 (I2S)?D??﹞t??3足D辰米¯車??㏒??辰?o赤﹞?辰??迄?迆∩?o‘那y∩|角赤?㏒
*	D?    2?㏒o?T
*	﹞米 ?? ?米: ?T
*********************************************************************************************************
*/
void I2S_CODEC_DataTransfer(void)
{
	uint16_t usData;

#if 1

    {
		usData = g_tRec.pAudio[g_tRec.uiCursor++];		
//		if (SPI_I2S_GetFlagStatus(SPI2, I2S_FLAG_CHSIDE) != SET)	
//		{		
			if (g_tRec.uiCursor >= sizeof(g_tRec.pAudio) / 2)
			{
				g_tRec.uiCursor = sizeof(g_tRec.pAudio)/ 2;
				SPI_I2S_ITConfig(I2S, SPI_I2S_IT_RXNE, DISABLE);
				SPI_I2S_ITConfig(I2S, SPI_I2S_IT_TXE, DISABLE);			
				//bsp_PutKey(KEY_DOWN_JOY_OK);	/* ?㏒?a赤㏒?1?邦～∩?? */
			}		
//		}
		SPI_I2S_SendData(I2S, usData);	

	}
/*
	else 	
	{
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
        
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
	}	
	*/
#else	
	if (g_tRec.ucStatus == STA_RECORDING)	/* ??辰?℅∩足? */
	{	
		usData = SPI_I2S_ReceiveData(SPI2);
		g_tRec.pAudio[g_tRec.uiCursor] = usData;
		if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
		{		
			g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
			/* ???1I2S2 RXNE?D??(?車那?2???)㏒?D豕辰a那㊣?迄∩辰?a */ 
			SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
			bsp_PutKey(KEY_DOWN_JOY_OK);	/* ?㏒?a赤㏒?1?邦～∩?? */
		}
	}
	else 	/* ﹞?辰?℅∩足? */
	{
		usData = g_tRec.pAudio[g_tRec.uiCursor];
		SPI_I2S_SendData(SPI2, usData);	
		if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
		{
			g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
			/* ???1I2S2 TXE?D??(﹞⊿?赤?o3?????)㏒?D豕辰a那㊣?迄∩辰?a */ 
			SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);			
			bsp_PutKey(KEY_DOWN_JOY_OK);	/* ?㏒?a赤㏒?1?邦～∩?? */
		}
	}
#endif
}


void SPI3_IRQHandler(void)
{
	if ( (SPI_I2S_GetITStatus(I2S, SPI_I2S_IT_TXE) == SET)
		|| (SPI_I2S_GetITStatus(I2S, SPI_I2S_IT_RXNE) == SET))
	{	
		I2S_CODEC_DataTransfer();
	}
}

/*========================

                Audio MAL Interface Control Functions

  ==============================*/

/**
  * @brief  Initializes and prepares the Media to perform audio data transfer 
  *         from Media to the I2S peripheral.
  * @param  None
  * @retval None
  */
static void Audio_MAL_Init(void)  
{   
#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
  NVIC_InitTypeDef NVIC_InitStructure;
#endif
  
  /* Enable the DMA clock */
  RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, ENABLE); 
  
  /* Configure the DMA Stream */
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_MAL_DMA_STREAM);
  /* Set the parameters to be configured */
  DMA_InitStructure.DMA_Channel = AUDIO_MAL_DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = I2S_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize =  DMA_PeripheralDataSize_HalfWord;//AUDIO_MAL_DMA_PERIPH_DATA_SIZE;//
  DMA_InitStructure.DMA_MemoryDataSize =   DMA_MemoryDataSize_Byte;//AUDIO_MAL_DMA_MEM_DATA_SIZE; 
#ifdef AUDIO_MAL_MODE_NORMAL
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
#else
  #error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);  

  /* Enable the selected DMA interrupts (selected in "stm32_eval_audio_codec.h" defines) */
#ifdef AUDIO_MAL_DMA_IT_TC_EN
  DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TC_EN */
#ifdef AUDIO_MAL_DMA_IT_HT_EN
  DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_HT_EN */
#ifdef AUDIO_MAL_DMA_IT_TE_EN
  DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TE_EN */
  
  /* Enable the I2S DMA request */
  SPI_I2S_DMACmd(I2S, SPI_I2S_DMAReq_Tx, ENABLE);
  
#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
  /* I2S DMA IRQ Channel configuration */
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif 
}

/**
  * @brief  Restore default state of the used Media.
  * @param  None
  * @retval None
  */
static void Audio_MAL_DeInit(void)  
{   
#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
  NVIC_InitTypeDef NVIC_InitStructure;  
  
  /* Deinitialize the NVIC interrupt for the I2S DMA Stream */
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);  
#endif 
  
  /* Disable the DMA stream before the deinit */
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  
  /* Dinitialize the DMA Stream */
  DMA_DeInit(AUDIO_MAL_DMA_STREAM);
  
  /* 
     The DMA clock is not disabled, since it can be used by other streams 
                                                                          */ 
}

/**
  * @brief  Starts playing audio stream from the audio Media.
  * @param  None
  * @retval None
  */
static void Audio_MAL_Play(uint32_t Addr, uint32_t Size)
{  
    #if 1
  Audio_MAL_Init();
  
  I2S_Cmd(I2S,ENABLE);
  /* Configure the buffer address and size */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Addr;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)Size;
  /* Configure the DMA Stream with the new parameters */
  DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);
  
  /* Enable the I2S DMA Stream*/
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
  #endif
   // rt_kprintf("DMA play!! \n");
#if 0
  I2S_Cmd(I2S,ENABLE);

  for(i = 0;i < 100;i++)
    {
        SPI_I2S_SendData(I2S,22);

       // Addr += 4;
    /* Wait until a data is sent(not busy), before config /CS HIGH */

    //while(SPI_I2S_GetFlagStatus(I2S, SPI_I2S_FLAG_TXE) == RESET) ;

    //while(SPI_I2S_GetFlagStatus(I2S, SPI_I2S_FLAG_BSY) != RESET);
    }
#endif  


#if 0
g_tRec.uiCursor = 0;
g_tRec.pAudio = (int16_t *)Addr;

I2S_StartPlay(I2S_Standard_LSB,I2S_DataFormat_16b,I2S_AudioFreq_8k);

#endif
  /* If the I2S peripheral is still not enabled, enable it */
  if ((I2S->I2SCFGR & ENABLE) == 0)
  {
    I2S_Cmd(I2S, ENABLE);
  }   
}

/**
  * @brief  Pauses or Resumes the audio stream playing from the Media.
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *              from 0) to resume. 
  * @param  Addr: Address from/at which the audio stream should resume/pause.
  * @retval None
  */
static void Audio_MAL_PauseResume(uint32_t Cmd, uint32_t Addr)
{
  /* Pause the audio file playing */
  if (Cmd == AUDIO_PAUSE)
  {   
    /* Disable the I2S DMA request */
    SPI_I2S_DMACmd(I2S, SPI_I2S_DMAReq_Tx, DISABLE);

    /* Pause the I2S DMA Stream 
        Note. For the STM32F4xx devices, the DMA implements a pause feature, 
              by disabling the stream, all configuration is preserved and data 
              transfer is paused till the next enable of the stream.*/
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  }
  else /* AUDIO_RESUME */
  {
    /* Enable the I2S DMA request */
    SPI_I2S_DMACmd(I2S, SPI_I2S_DMAReq_Tx, ENABLE);
  
    /* Resume the I2S DMA Stream 
        Note. For the STM32F4xx devices, the DMA implements a pause feature, 
              by disabling the stream, all configuration is preserved and data 
              transfer is paused till the next enable of the stream.*/
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
    
    /* If the I2S peripheral is still not enabled, enable it */
    if ((I2S->I2SCFGR & ENABLE) == 0)
    {
      I2S_Cmd(I2S, ENABLE);
    }    
  }  
}

/**
  * @brief  Stops audio stream playing on the used Media.
  * @param  None
  * @retval None
  */
static void Audio_MAL_Stop(void)
{   
  /* Stop the Transfer on the I2S side: Stop and disable the DMA stream */
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);

  /* Clear all the DMA flags for the next transfer */
  DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC |AUDIO_MAL_DMA_FLAG_HT | \
                                  AUDIO_MAL_DMA_FLAG_FE | AUDIO_MAL_DMA_FLAG_TE);
  
  /*  
           The I2S DMA requests are not disabled here.
                                                            */
  
  /* In all modes, disable the I2S peripheral */
  I2S_Cmd(I2S, DISABLE);
}


/**
  * @brief  This function handles main Media layer interrupt. 
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void Audio_MAL_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

#ifndef AUDIO_MAL_MODE_NORMAL
	uint16_t *pAddr = (uint16_t *)CurrentPos;
	uint32_t Size = AudioRemSize;
#endif /* AUDIO_MAL_MODE_NORMAL */

	//rt_kprintf("come in IRQ\n");
#ifdef AUDIO_MAL_DMA_IT_TC_EN
	/* Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC) != RESET)
	{     
#ifdef AUDIO_MAL_MODE_NORMAL
	/* Check if the end of file has been reached */
	if (AudioRemSize > 0)
	{      
	  /* Wait the DMA Stream to be effectively disabled */
	  while (DMA_GetCmdStatus(AUDIO_MAL_DMA_STREAM) != DISABLE)
	  {}
	  
	  /* Clear the Interrupt flag */
	  DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);  
	  
	  /* Re-Configure the buffer address and size */
	  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) CurrentPos;
	  DMA_InitStructure.DMA_BufferSize = (uint32_t) (DMA_MAX(AudioRemSize));
	  
	  /* Configure the DMA Stream with the new parameters */
	  DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);
	  
	  /* Enable the I2S DMA Stream*/
	  DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);    
	  
	  /* Update the current pointer position */
	  CurrentPos += DMA_MAX(AudioRemSize);        
	  
	  /* Update the remaining number of data to be played */
	  AudioRemSize -= DMA_MAX(AudioRemSize);    
	}
	else
	{
	  /* Disable the I2S DMA Stream*/
	  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);   
	  
	  /* Clear the Interrupt flag */
	  DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);  

	  
	  DMA_ClearITPendingBit(AUDIO_MAL_DMA_STREAM, DMA_IT_TCIF4);

	  release_semadpcm();
	//      rt_kprintf("clear flag\n");
	}

#elif defined(AUDIO_MAL_MODE_CIRCULAR)
	/* Manage the remaining file size and new address offset: This function 
	   should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
	EVAL_AUDIO_TransferComplete_CallBack(pAddr, Size);    

	/* Clear the Interrupt flag */
	DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);
#endif /* AUDIO_MAL_MODE_NORMAL */  
	}
#endif /* AUDIO_MAL_DMA_IT_TC_EN */

#ifdef AUDIO_MAL_DMA_IT_HT_EN  
	/* Half Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_HT) != RESET)
	{
	/* Manage the remaining file size and new address offset: This function 
	   should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
	EVAL_AUDIO_HalfTransfer_CallBack((uint32_t)pAddr, Size);    

	/* Clear the Interrupt flag */
	DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_HT);    
	}
#endif /* AUDIO_MAL_DMA_IT_HT_EN */

#ifdef AUDIO_MAL_DMA_IT_TE_EN  
	/* FIFO Error interrupt */
	if ((DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TE) != RESET) || \
	 (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_FE) != RESET) || \
	 (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_DME) != RESET))

	{
	/* Manage the error generated on DMA FIFO: This function 
	   should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
	EVAL_AUDIO_Error_CallBack((uint32_t*)&pAddr);    

	/* Clear the Interrupt flag */
	DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TE | AUDIO_MAL_DMA_FLAG_FE | \
	                                    AUDIO_MAL_DMA_FLAG_DME);
	}  
#endif /* AUDIO_MAL_DMA_IT_TE_EN */
	/* leave interrupt */
	rt_interrupt_leave();

	//rt_kprintf("come out IRQ\n");

}


/**
  * @brief  Configure the audio peripherals.
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
void Pt8211_AUDIO_Init(uint32_t AudioFreq)
{    

    /* DeInitialize I2S */  
    _I2S_Init(AudioFreq);
    /* I2S data transfer preparation:
    Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
   Audio_MAL_Init();
  // I2S_NVIC_Config();
}

/**
  * @brief  Deinitializes all the resources used by the codec (those initialized
  *         by EVAL_AUDIO_Init() function) EXCEPT the I2C resources since they are 
  *         used by the IOExpander as well (and eventually other modules). 
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void  Pt8211_AUDIO_DeInit(void)
{ 
  /* DeInitialize the Media layer */
  Audio_MAL_DeInit();
  
  /* DeInitialize Codec */  
  I2S_DeInit();  
  
}

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data BYTES.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Pt8211_AUDIO_Play(uint16_t* pBuffer, uint32_t Size)
{

  GPIO_ResetBits(GPIOC, GPIO_Pin_6); /* Enable the amplifier */

  /* Set the total number of data to be played (count in half-word) */
  AudioTotalSize = Size/2;

  
  /* Update the Media layer and enable it for play */  
  //Audio_MAL_Play((uint32_t)pBuffer, (uint32_t)(DMA_MAX(AudioTotalSize / 2)));
  
  Audio_MAL_Play((uint32_t)pBuffer, (uint32_t)AudioTotalSize);
  
  /* Update the remaining number of data to be played */
  AudioRemSize = (Size/2) - DMA_MAX(AudioTotalSize);
  
  /* Update the current audio pointer position */
  CurrentPos = pBuffer + DMA_MAX(AudioTotalSize);
  
  return 0;
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the I2S 
  *         peripheral is disabled. 
  * 
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *          this function should be called for resume (use of EVAL_AUDIO_Play() 
  *          function for resume could lead to unexpected behavior).
  * 
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
void  Pt8211_AUDIO_PauseResume(uint32_t Cmd)
{    

    /* Call the Media layer pause/resume function */
    Audio_MAL_PauseResume(Cmd, 0);

}

/**
  * @brief  Stops audio playing and Power down the Audio Codec. 
  * @param  Option: could be one of the following parameters 
  *           - CODEC_PDWN_SW: for software power off (by writing registers). 
  *                            Then no need to reconfigure the Codec after power on.
  *           - CODEC_PDWN_HW: completely shut down the codec (physically). 
  *                            Then need to reconfigure the Codec after power on.  
  * @retval 0 if correct communication, else wrong communication
  */
void  Pt8211_AUDIO_Stop(uint32_t Option)
{

    /* Call Media layer Stop function */
    Audio_MAL_Stop();
    
    /* Update the remaining data number */
    AudioRemSize = AudioTotalSize;    
    

}

