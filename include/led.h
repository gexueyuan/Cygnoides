#ifndef __LED_H
#define __LED_H 

#include "board.h"

typedef enum 
{
  LED0 = 0,
  LED1 = 1,
  LED2 = 2,
  LED3 = 3,
}Led_TypeDef;


#define LEDn  3
#define LED_RED     LED0
#define LED_GREEN   LED1
#define LED_BLUE    LED2



#define LED0_PIN                         GPIO_Pin_13
#define LED0_GPIO_PORT                   GPIOC
#define LED0_GPIO_CLK                    RCC_AHB1Periph_GPIOC  

#ifdef USE_I2S3
#define LED1_PIN                         GPIO_Pin_3
#define LED1_GPIO_PORT                   GPIOC
#define LED1_GPIO_CLK                    RCC_AHB1Periph_GPIOC 
#else
#define LED1_PIN                         GPIO_Pin_12
#define LED1_GPIO_PORT                   GPIOC
#define LED1_GPIO_CLK                    RCC_AHB1Periph_GPIOC  
#endif  
#define LED2_PIN                         GPIO_Pin_11
#define LED2_GPIO_PORT                   GPIOC
#define LED2_GPIO_CLK                    RCC_AHB1Periph_GPIOC


void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDBlink(Led_TypeDef Led);

void led_init(void);
void led_on(Led_TypeDef led);
void led_off(Led_TypeDef led);
void led_blink(Led_TypeDef led);


#endif
