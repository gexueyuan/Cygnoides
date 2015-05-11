
#include "led.h"
#include <stm32f4xx.h>

void STM_EVAL_LEDInit(Led_TypeDef Led);

GPIO_TypeDef*  GPIO_PORT[LEDn] = {LED0_GPIO_PORT, LED1_GPIO_PORT, LED2_GPIO_PORT,
                               };
const uint16_t GPIO_PIN[LEDn] = {LED0_PIN, LED1_PIN, LED2_PIN,
                                 };
const uint32_t GPIO_CLK[LEDn] = {LED0_GPIO_CLK, LED1_GPIO_CLK, LED2_GPIO_CLK,
                                 };
/* --- DCKCFGR Register ---*/
/* Alias word address of TIMPRE bit */
#define RCC_OFFSET                (RCC_BASE - PERIPH_BASE)
#define DCKCFGR_OFFSET            (RCC_OFFSET + 0x8C)
#define TIMPRE_BitNumber          0x18
#define DCKCFGR_TIMPRE_BB         (PERIPH_BB_BASE + (DCKCFGR_OFFSET * 32) + (TIMPRE_BitNumber * 4))

/** @defgroup RCC_TIM_PRescaler_Selection
  * @{
  */
#define RCC_TIMPrescDesactivated             ((uint8_t)0x00)
#define RCC_TIMPrescActivated                ((uint8_t)0x01)

#define IS_RCC_TIMCLK_PRESCALER(VALUE) (((VALUE) == RCC_TIMPrescDesactivated) || ((VALUE) == RCC_TIMPrescActivated))

/**
  * @brief  Configures the Timers clocks prescalers selection.
  * 
  * @note   This function can be used only for STM32F42xxx/43xxx and STM32F401xx devices. 
  *   
  * @param  RCC_TIMCLKPrescaler : specifies the Timers clocks prescalers selection
  *         This parameter can be one of the following values:
  *            @arg RCC_TIMPrescDesactivated: The Timers kernels clocks prescaler is 
  *                 equal to HPRE if PPREx is corresponding to division by 1 or 2, 
  *                 else it is equal to [(HPRE * PPREx) / 2] if PPREx is corresponding to 
  *                 division by 4 or more.
  *                   
  *            @arg RCC_TIMPrescActivated: The Timers kernels clocks prescaler is 
  *                 equal to HPRE if PPREx is corresponding to division by 1, 2 or 4, 
  *                 else it is equal to [(HPRE * PPREx) / 4] if PPREx is corresponding 
  *                 to division by 8 or more.
  * @retval None
  */
void RCC_TIMCLKPresConfig(uint32_t RCC_TIMCLKPrescaler)
{
  /* Check the parameters */
  assert_param(IS_RCC_TIMCLK_PRESCALER(RCC_TIMCLKPrescaler));

  *(__IO uint32_t *) DCKCFGR_TIMPRE_BB = RCC_TIMCLKPrescaler;
  
}


void STM_EVAL_LEDInit(Led_TypeDef Led)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(GPIO_CLK[Led], ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
}

void STM_EVAL_LEDOn(Led_TypeDef Led)
{
  GPIO_PORT[Led]->BSRRL = GPIO_PIN[Led];
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED4
  *     @arg LED3
  *     @arg LED5
  *     @arg LED6 
  * @retval None
  */
void STM_EVAL_LEDOff(Led_TypeDef Led)
{
  GPIO_PORT[Led]->BSRRH = GPIO_PIN[Led];  
}


/**
  * @brief  Switch selected LED's status.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED4
  *     @arg LED3
  *     @arg LED5
  *     @arg LED6 
  * @retval None
  */
void STM_EVAL_LEDBlink(Led_TypeDef Led)
{
  GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];  
}


/**
  * @brief  Configure the TIM1 Ouput Channels.
  * @param  None
  * @retval None
  */
void TIM1_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM1 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  /* GPIOA clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /* GPIOA Configuration: TIM1 CH2 (PA9)*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 

  /* Connect TIM4 pins to AF2 */  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1); 
}

void LED_PWM_INIT(uint16_t CCR_Val)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    uint16_t PrescalerValue = 0;


    TIM1_Config();
    
    RCC_TIMCLKPresConfig(RCC_TIMPrescActivated);

    /* Compute the prescaler value */
    PrescalerValue = (uint16_t) (SystemCoreClock / 21000000) - 1;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 699;
    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = CCR_Val;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    
    /* TIM4 enable counter */
    TIM_Cmd(TIM1, ENABLE);


}

