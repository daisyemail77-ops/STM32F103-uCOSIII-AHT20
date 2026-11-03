/**
 * aht20.c - AHT20 温湿度传感器驱动（HAL I2C，阻塞轮询）
 *   7 位地址 0x38：写 0x70 / 读 0x71
 *   初始化：上电 >40ms，读状态，bit3 未置位则发 0xBE 0x08 0x00
 *   测量：0xAC 0x33 0x00 → 等 80ms → 读 6 字节
 *   湿度 = raw_h / 2^20 * 100%，温度 = raw_t / 2^20 * 200 - 50
 */
#include "aht20.h"

#define AHT20_ADDR      0x38
#define AHT20_ADDR_W    (AHT20_ADDR << 1)
#define AHT20_ADDR_R    ((AHT20_ADDR << 1) | 1)

#define AHT20_CMD_INIT  0xBE
#define AHT20_CMD_TRIG  0xAC
#define AHT20_WAIT_MS   80U
#define AHT20_TIMEOUT   50U
#define AHT20_TRIES     3U

/* 弱定义：裸机用 HAL_Delay，RTOS 里由应用覆盖为 OSTimeDly */
__weak void AHT20_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

static uint8_t aht20_read_status(I2C_HandleTypeDef *hi2c, uint8_t *status)
{
    if (HAL_I2C_Master_Receive(hi2c, AHT20_ADDR_R, status, 1, AHT20_TIMEOUT) != HAL_OK) {
        return 1;
    }
    return 0;
}

static uint8_t aht20_send_init(I2C_HandleTypeDef *hi2c)
{
    uint8_t cmd[3] = { AHT20_CMD_INIT, 0x08, 0x00 };

    if (HAL_I2C_Master_Transmit(hi2c, AHT20_ADDR_W, cmd, 3, AHT20_TIMEOUT) != HAL_OK) {
        return 1;
    }
    AHT20_DelayMs(10);
    return 0;
}

uint8_t AHT20_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t status = 0;
    uint8_t i;

    AHT20_DelayMs(40);

    for (i = 0; i < AHT20_TRIES; i++) {
        if (aht20_read_status(hi2c, &status) == 0) {
            if ((status & 0x18) == 0x18) {
                return 0;
            }
        }
        if (aht20_send_init(hi2c) != 0) {
            return 1;
        }
    }

    if (aht20_read_status(hi2c, &status) != 0) {
        return 2;
    }
    if ((status & 0x80) != 0) {
        return 3;
    }
    if ((status & 0x08) == 0) {
        return 4;
    }
    return 0;
}

uint8_t AHT20_ReadData(I2C_HandleTypeDef *hi2c, int16_t *temp_x10, uint16_t *humi_x10)
{
    uint8_t  cmd[3]  = { AHT20_CMD_TRIG, 0x33, 0x00 };
    uint8_t  data[6] = { 0 };
    uint32_t raw_h;
    uint32_t raw_t;
    uint8_t  i;

    if (HAL_I2C_Master_Transmit(hi2c, AHT20_ADDR_W, cmd, 3, AHT20_TIMEOUT) != HAL_OK) {
        return 1;
    }
    AHT20_DelayMs(AHT20_WAIT_MS);

    for (i = 0; i < AHT20_TRIES; i++) {
        if (HAL_I2C_Master_Receive(hi2c, AHT20_ADDR_R, data, 6, AHT20_TIMEOUT) != HAL_OK) {
            return 2;
        }
        if ((data[0] & 0x80) == 0) {
            break;
        }
        AHT20_DelayMs(10);
        if (i == (AHT20_TRIES - 1u)) {
            return 3;
        }
    }

    raw_h = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((uint32_t)data[3] >> 4);
    raw_t = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];

    *humi_x10 = (uint16_t)(((raw_h * 1000UL) + 524288UL) / 1048576UL);
    *temp_x10 = (int16_t)(((raw_t * 2000UL) + 524288UL) / 1048576UL) - 500;

    if (*humi_x10 > 1000u) {
        *humi_x10 = 1000u;
    }
    return 0;
}
