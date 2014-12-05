/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : bma250e.c
 @brief  : bma250e chip spi driver
 @author : wanglei
 @history:
           2014-8-11    wanglei       Created file
           ...
******************************************************************************/

#include "bma250e.h"
#include "gsensor.h"

#ifdef GSENSOR_BMA250E

/** @defgroup STM32F401_DISCOVERY_BMA250E_Private_Variables
  * @{
  */
__IO uint32_t  BMA250ETimeout = BMA250E_FLAG_TIMEOUT;
/**
  * @}
  */

/** @defgroup STM32F401_DISCOVERY_BMA250E_Private_FunctionPrototypes
  * @{
  */
static uint8_t BMA250E_SendByte(uint8_t byte);

/**
  * @brief  Set BMA250E Initialization.
  * @param  BMA250E_InitStruct: pointer to a BMA250E_InitTypeDef structure
  *         that contains the configuration setting for the BMA250E.
  * @retval None
  */
void BMA250E_Init(BMA250E_InitTypeDef *BMA250E_InitStruct)
{
  uint8_t ctrl1 = 0x00, ctrl4 = 0x00;

  /* Configure the low level interface ---------------------------------------*/
  BMA250E_LowLevel_Init();

  /* Configure MEMS: data rate, power mode, full scale and axes */
  ctrl1 |= (uint8_t) (BMA250E_InitStruct->Power_Mode | BMA250E_InitStruct->Output_DataRate | \
                    BMA250E_InitStruct->Axes_Enable | BMA250E_InitStruct->Band_Width);

  ctrl4 |= (uint8_t) (BMA250E_InitStruct->BlockData_Update | BMA250E_InitStruct->Endianness | \
                    BMA250E_InitStruct->Full_Scale);
#if 0
  /* Write value to MEMS CTRL_REG1 regsister */
  BMA250E_Write(&ctrl1, BMA250E_CTRL_REG1_ADDR, 1);

  /* Write value to MEMS CTRL_REG4 regsister */
  BMA250E_Write(&ctrl4, BMA250E_CTRL_REG4_ADDR, 1);
#endif
}

/**
  * @brief  Reboot memory content of BMA250E
  * @param  None
  * @retval None
  */
void BMA250E_RebootCmd(void)
{
  uint8_t tmpreg;

  /* Read CTRL_REG5 register */
  BMA250E_Read(&tmpreg, BMA250E_CTRL_REG5_ADDR, 1);

  /* Enable or Disable the reboot memory */
  tmpreg |= BMA250E_BOOT_REBOOTMEMORY;

  /* Write value to MEMS CTRL_REG5 regsister */
 // BMA250E_Write(&tmpreg, BMA250E_CTRL_REG5_ADDR, 1);
}

/**
  * @brief Set BMA250E Interrupt configuration
  * @param  BMA250E_InterruptConfig_TypeDef: pointer to a BMA250E_InterruptConfig_TypeDef
  *         structure that contains the configuration setting for the BMA250E Interrupt.
  * @retval None
  */
void BMA250E_INT1InterruptConfig(BMA250E_InterruptConfigTypeDef *BMA250E_IntConfigStruct)
{
#if 0
  uint8_t ctrl_cfr = 0x00, ctrl3 = 0x00;

  /* Read INT1_CFG register */
  BMA250E_Read(&ctrl_cfr, BMA250E_INT1_CFG_ADDR, 1);

  /* Read CTRL_REG3 register */
  BMA250E_Read(&ctrl3, BMA250E_CTRL_REG3_ADDR, 1);

  ctrl_cfr &= 0x80;

  ctrl3 &= 0xDF;

  /* Configure latch Interrupt request and axe interrupts */
  ctrl_cfr |= (uint8_t)(BMA250E_IntConfigStruct->Latch_Request| \
                   BMA250E_IntConfigStruct->Interrupt_Axes);

  ctrl3 |= (uint8_t)(BMA250E_IntConfigStruct->Interrupt_ActiveEdge);

  /* Write value to MEMS INT1_CFG register */
  BMA250E_Write(&ctrl_cfr, BMA250E_INT1_CFG_ADDR, 1);

  /* Write value to MEMS CTRL_REG3 register */
  BMA250E_Write(&ctrl3, BMA250E_CTRL_REG3_ADDR, 1);
  #endif
}

/**
  * @brief  Enable or disable INT1 interrupt
  * @param  InterruptState: State of INT1 Interrupt
  *      This parameter can be:
  *        @arg BMA250E_INT1INTERRUPT_DISABLE
  *        @arg BMA250E_INT1INTERRUPT_ENABLE
  * @retval None
  */
void BMA250E_INT1InterruptCmd(uint8_t InterruptState)
{
  uint8_t tmpreg;

  /* Read CTRL_REG3 register */
  BMA250E_Read(&tmpreg, BMA250E_CTRL_REG3_ADDR, 1);

  tmpreg &= 0x7F;
  tmpreg |= InterruptState;

  /* Write value to MEMS CTRL_REG3 regsister */
  //BMA250E_Write(&tmpreg, BMA250E_CTRL_REG3_ADDR, 1);
}

/**
  * @brief  Enable or disable INT2 interrupt
  * @param  InterruptState: State of INT1 Interrupt
  *      This parameter can be:
  *        @arg BMA250E_INT2INTERRUPT_DISABLE
  *        @arg BMA250E_INT2INTERRUPT_ENABLE
  * @retval None
  */
void BMA250E_INT2InterruptCmd(uint8_t InterruptState)
{
  uint8_t tmpreg;

  /* Read CTRL_REG3 register */
  BMA250E_Read(&tmpreg, BMA250E_CTRL_REG3_ADDR, 1);

  tmpreg &= 0xF7;
  tmpreg |= InterruptState;

  /* Write value to MEMS CTRL_REG3 regsister */
  //BMA250E_Write(&tmpreg, BMA250E_CTRL_REG3_ADDR, 1);
}


/**
  * @brief  Writes one byte to the BMA250E.
  * @param  pBuffer : pointer to the buffer  containing the data to be written to the BMA250E.
  * @param  WriteAddr : BMA250E's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void BMA250E_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
  /* Configure the MS bit:
       - When 0, the address will remain unchanged in multiple read/write commands.
       - When 1, the address will be auto incremented in multiple read/write commands.
  */
  if(NumByteToWrite > 0x01)
  {
    WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
  }
#if 1 
  /* Set chip select Low at the start of the transmission */
  BMA250E_CS_LOW();

  /* Send the Address of the indexed register */
  BMA250E_SendByte(WriteAddr);
  /* Send the data that will be written into the device (MSB First) */
  while(NumByteToWrite >= 0x01)
  {
    BMA250E_SendByte(*pBuffer);
    NumByteToWrite--;
    pBuffer++;
  }

  /* Set chip select High at the end of the transmission */
  BMA250E_CS_HIGH();
#else
	u16 temp ;
	u16 receive ;
	temp = (WriteAddr<<8)|(*pBuffer);

    gsensor_sendata(temp);
#endif
}

/**
  * @brief  Reads a block of data from the BMA250E.
  * @param  pBuffer : pointer to the buffer that receives the data read from the BMA250E.
  * @param  ReadAddr : BMA250E's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the BMA250E.
  * @retval None
  */
void BMA250E_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
  if(NumByteToRead > 0x01)
  {
    ReadAddr |= (uint8_t)(READWRITE_CMD | MULTIPLEBYTE_CMD);
  }
  else
  {
    ReadAddr |= (uint8_t)READWRITE_CMD;
  }
#if 1 
  /* Set chip select Low at the start of the transmission */
  BMA250E_CS_LOW();

  /* Send the Address of the indexed register */
  BMA250E_SendByte(ReadAddr);

  /* Receive the data that will be read from the device (MSB First) */
  while(NumByteToRead > 0x00)
  {
    /* Send dummy byte (0x00) to generate the SPI clock to BMA250E (Slave device) */
    *pBuffer = BMA250E_SendByte(0x00);
    NumByteToRead--;
    pBuffer++;
  }

  /* Set chip select High at the end of the transmission */
  BMA250E_CS_HIGH();
#else
	u16 temp ;
	u16 receive ;
	temp = (ReadAddr<<8)|(*pBuffer);
	receive = gsensor_sendata(temp);

    *pBuffer =receive;
#endif

}
/**
  * @brief  Initializes the low level interface used to drive the BMA250E
  * @param  None
  * @retval None
  */
void BMA250E_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable the SPI periph */
  RCC_APB1PeriphClockCmd(BMA250E_SPI_CLK, ENABLE);

  /* Enable SCK, MOSI and MISO GPIO clocks */
  RCC_AHB1PeriphClockCmd(BMA250E_SPI_SCK_GPIO_CLK | BMA250E_SPI_MISO_GPIO_CLK | BMA250E_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable CS  GPIO clock */
  RCC_AHB1PeriphClockCmd(BMA250E_SPI_CS_GPIO_CLK, ENABLE);

  /* Enable INT1 GPIO clock */
  RCC_AHB1PeriphClockCmd(BMA250E_SPI_INT1_GPIO_CLK, ENABLE);

  /* Enable INT2 GPIO clock */
  RCC_AHB1PeriphClockCmd(BMA250E_SPI_INT2_GPIO_CLK, ENABLE);

  GPIO_PinAFConfig(BMA250E_SPI_SCK_GPIO_PORT, BMA250E_SPI_SCK_SOURCE, BMA250E_SPI_SCK_AF);
  GPIO_PinAFConfig(BMA250E_SPI_MISO_GPIO_PORT, BMA250E_SPI_MISO_SOURCE, BMA250E_SPI_MISO_AF);
  GPIO_PinAFConfig(BMA250E_SPI_MOSI_GPIO_PORT, BMA250E_SPI_MOSI_SOURCE, BMA250E_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN; //GPIO_PuPd_NOPULL;//
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = BMA250E_SPI_SCK_PIN;
  GPIO_Init(BMA250E_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  BMA250E_SPI_MOSI_PIN;
  GPIO_Init(BMA250E_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //wanglei add
  GPIO_InitStructure.GPIO_Pin = BMA250E_SPI_MISO_PIN;
  GPIO_Init(BMA250E_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(BMA250E_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(BMA250E_SPI, &SPI_InitStructure);

  /* Enable SPI2  */
  SPI_Cmd(BMA250E_SPI, ENABLE);

  /* Configure GPIO PIN for Lis Chip select */
  GPIO_InitStructure.GPIO_Pin = BMA250E_SPI_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(BMA250E_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

  /* Deselect : Chip Select high */
  GPIO_SetBits(BMA250E_SPI_CS_GPIO_PORT, BMA250E_SPI_CS_PIN);

  /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStructure.GPIO_Pin = BMA250E_SPI_INT1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BMA250E_SPI_INT1_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = BMA250E_SPI_INT2_PIN;
  GPIO_Init(BMA250E_SPI_INT2_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
static uint8_t BMA250E_SendByte(uint8_t byte)
{

   
  /* Loop while DR register in not empty */
  BMA250ETimeout = BMA250E_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(BMA250E_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
    if((BMA250ETimeout--) == 0) return BMA250E_TIMEOUT_UserCallback();
#endif
  }

  /* Send a Byte through the SPI peripheral */
  SPI_I2S_SendData(BMA250E_SPI, (uint16_t)byte);

  /* Wait to receive a Byte */
  BMA250ETimeout = BMA250E_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(BMA250E_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
    if((BMA250ETimeout--) == 0) return BMA250E_TIMEOUT_UserCallback();
#endif
  }

  /* Return the Byte read from the SPI bus */
  return (uint8_t)SPI_I2S_ReceiveData(BMA250E_SPI);

}

#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t BMA250E_TIMEOUT_UserCallback(void)
{
  /* Block communication and all processes */
  while (1)
  {
  }
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

#endif /* #ifdef GSENSOR_BMA250E */
