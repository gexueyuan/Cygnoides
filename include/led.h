#ifndef __LED_H
#define __LED_H 

#include "board.h"

typedef enum 
{
  LED4 = 0,
  LED3 = 1,
  LED5 = 2,
  LED6 = 3,
}Led_TypeDef;

#ifdef HARDWARE_DVB_F401Disco
#define LEDn  4
#elif defined HARDWARE_MODULE_V1
#define LEDn  3
#define LED_RED  LED4
#define LED_BLUE  LED5
#define LED_GREEN  LED3
#elif defined HARDWARE_MODULE_V2
#define LEDn  3
#define LED_RED  LED4
#define LED_BLUE  LED3
#define LED_GREEN  LED5



#endif


#ifdef HARDWARE_DVB_F401Disco
#define LED4_PIN                         GPIO_Pin_12
#define LED4_GPIO_PORT                   GPIOD
#define LED4_GPIO_CLK                    RCC_AHB1Periph_GPIOD  
  
#define LED3_PIN                         GPIO_Pin_13
#define LED3_GPIO_PORT                   GPIOD
#define LED3_GPIO_CLK                    RCC_AHB1Periph_GPIOD  
  
#define LED5_PIN                         GPIO_Pin_14
#define LED5_GPIO_PORT                   GPIOD
#define LED5_GPIO_CLK                    RCC_AHB1Periph_GPIOD  
  
#define LED6_PIN                         GPIO_Pin_15
#define LED6_GPIO_PORT                   GPIOD
#define LED6_GPIO_CLK                    RCC_AHB1Periph_GPIOD
#elif defined(HARDWARE_MODULE_V1)||defined(HARDWARE_MODULE_V2)
#define LED3_PIN                         GPIO_Pin_2
#define LED3_GPIO_PORT                   GPIOC
#define LED3_GPIO_CLK                    RCC_AHB1Periph_GPIOC  
  
#define LED4_PIN                         GPIO_Pin_1
#define LED4_GPIO_PORT                   GPIOC
#define LED4_GPIO_CLK                    RCC_AHB1Periph_GPIOC  
  
#define LED5_PIN                         GPIO_Pin_0
#define LED5_GPIO_PORT                   GPIOC
#define LED5_GPIO_CLK                    RCC_AHB1Periph_GPIOC  
#endif


void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDBlink(Led_TypeDef Led);

#endif
