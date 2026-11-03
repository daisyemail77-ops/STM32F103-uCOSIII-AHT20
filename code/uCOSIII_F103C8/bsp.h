#ifndef __BSP_H
#define __BSP_H

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef  hi2c1;

/* LED1: 板载 PC13（低电平点亮）  LED2: 外接 PB0（低电平点亮） */
#define LED1_PORT   GPIOC
#define LED1_PIN    GPIO_PIN_13
#define LED2_PORT   GPIOB
#define LED2_PIN    GPIO_PIN_0

void BSP_Init(void);
void BSP_LED_Toggle(uint8_t led);

#endif
