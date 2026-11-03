# STM32F103 + uC/OS-III + AHT20 实验

嵌入式 RTOS 课程实验：uC/OS-III 移植到 STM32F103C8T6，三任务调度 + AHT20 温湿度采集。

## 目录结构

```
.
├─ code/
│  ├─ uCOSIII_F103C8/     任务一 / 任务二 3)：uC/OS-III 三任务工程
│  │  ├─ uCOSIII_F103C8.uvprojx
│  │  ├─ main.c  app_tasks.c/h  bsp.c/h  aht20.c/h
│  │  ├─ HAL/  Device/  Middlewares/
│  │  └─ hex/  uCOSIII_Task1_hello.hex、uCOSIII_Task2_3_AHT20.hex
│  └─ aht20-i2c-hal/      任务二 2)：裸机 AHT20 + 串口
├─ docs/
│  ├─ 实验报告.md / .pdf
│  ├─ 博客.md
│  └─ AI对话记录.md
└─ README.md
```

## 硬件

STM32F103C8T6 最小系统板 + AHT20 + ST-Link V2 + USB-TTL。

| STM32 | AHT20 | | STM32 | USB-TTL |
|---|---|---|---|---|
| 3.3V | VCC | | PA9 | RXD |
| GND | GND | | PA10 | TXD |
| PB6 | SCL | | GND | GND |
| PB7 | SDA | | | |

LED1 = PC13（板载），LED2 = PB0（外接 LED + 限流电阻）。

## 编译与烧录

1. Keil MDK 5.37 + ARM Compiler 6 打开 `uCOSIII_F103C8.uvprojx`；
2. `Options → Debug` 选 ST-Link Debugger；
3. F7 编译，F8 下载。

也可以直接用 `hex/` 里的固件通过 STM32CubeProgrammer 下载。

## 两个任务版本

`app_tasks.h`：

```c
#define APP_TASK3_USE_AHT20   0   /* 0: 任务一，任务3 打印 hello
                                     1: 任务二 3)，任务3 采集 AHT20 */
```

- `0` → 任务一（1 s LED、3 s LED、2 s 串口 hello）
- `1` → 任务二 3)（第 3 个任务改为每 2 s 采集温湿度）

两个现成 hex 已放在 `hex/` 下，可直接烧录对比。

## 串口

USART1，115200-8-N-1，UTF-8 编码。

任务一输出：

```
hello uc/OS! 欢迎来到RTOS多任务环境！
```

任务二 3) 输出：

```
Temp: 25.3 C, Humi: 48.2 %RH
```

## 说明

- uC/OS-III V3.08.02（Apache-2.0），uC-CPU、uC-LIB 同版本；
- 移植层为 `ARM-Cortex-M/ARMv7-M/ARM`，已针对 AC6 与无 FPU 的 Cortex-M3 做修改（见实验报告 3.3 节）；
- 编译占用：hello 版 27 KB，AHT20 版 32 KB，均在 C8 的 64 KB 以内。
