# 任务二：AHT20 温湿度采集（STM32F103C8T6 / HAL / Keil AC6）

本工程按 `stm32-oled-hal` 同一块板（STM32F103C8T6）和同一套 HAL/Device 库搭建，
去掉了 OLED 和 U8g2，只保留 **AHT20 + 串口**，用于任务二单独验收。

## 1. 工程与产物
- 工程：`AHT20_Task2.uvprojx`（Keil 打开）
- 固件：`Objects/aht20_task2.hex`
- 编译：0 Error，Code 约 18.1 KB
- Device = STM32F103C8，启动文件 = startup_stm32f103xb.s，宏 = STM32F103xB, USE_HAL_DRIVER

## 2. 接线
| STM32F103C8 | AHT20 |
|---|---|
| 3.3V | VCC |
| GND | GND |
| PB6 | SCL（I2C1） |
| PB7 | SDA（I2C1） |

| STM32F103C8 | USB-TTL |
|---|---|
| PA9 (TX) | RXD |
| PA10 (RX) | TXD |
| GND | GND |

> 用的是 I2C1 **默认引脚** PB6/PB7，不需要重映射。
> 如果更想用 PB8/PB9（与 OLED 共用）或 PB10/PB11（I2C2），告诉我，改一下重新编译即可。

## 3. 烧录
1. Keil 打开 `AHT20_Task2.uvprojx`；
2. `Options for Target → Debug` 选择 ST-Link Debugger；
3. 接 SWDIO / SWCLK / GND / 3.3V，按 F8（Load）；
4. 复位。

也可以直接用 STM32CubeProgrammer 下载 `Objects/aht20_task2.hex`。

## 4. 上电现象（串口 115200-8-N-1）
```
===== AHT20 Task2 =====
I2C1: PB6=SCL, PB7=SDA, 400kHz
AHT20 init OK
Temp: 25.3 C, Humi: 48.2 %RH
Temp: 25.3 C, Humi: 48.3 %RH
...
```
- 板载 LED（PC13）每 2 秒翻转一次；
- 哈气或手捏传感器，温湿度应明显变化。

## 5. 代码结构
- `main.c`：时钟（HSE 8M→72M，失败自动退 HSI）、I2C1、USART1、LED、printf 重定向、2 秒采集循环
- `aht20.c/h`：AHT20 驱动（0xBE 校准、0xAC 触发、80ms 等待、6 字节解析、定点 ×10 输出）
- `HAL/`、`Device/`：HAL 库与 CMSIS（自带 CMSIS Core 头文件，不依赖 Keil RTE）

## 6. 排查
| 现象 | 处理 |
|---|---|
| 串口无输出 | 查 PA9→RXD 交叉、115200、共地 |
| init FAILED | 查 PB6/PB7 接线、模块 3.3V 供电、上拉 |
| read FAILED | 用地址扫描确认 0x38；确认模块是 AHT20 而不是 AHT10 |
| 数值不变 | 确认每次触发后等 80ms |