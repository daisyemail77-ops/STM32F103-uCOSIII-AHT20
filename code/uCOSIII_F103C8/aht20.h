#ifndef __AHT20_H
#define __AHT20_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

uint8_t AHT20_Init(I2C_HandleTypeDef *hi2c);
uint8_t AHT20_ReadData(I2C_HandleTypeDef *hi2c, int16_t *temp_x10, uint16_t *humi_x10);
void    AHT20_DelayMs(uint32_t ms);

#endif
