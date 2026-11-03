# 与 AI 大模型对话记录（实验辅助）

> 说明：以下按时间顺序整理本次实验过程中与 AI 的对话。用户消息保留原意，AI 回复做了要点摘录（完整代码与文件见工程目录）。图片类附件未包含在本文档中。

**使用模型：** DeepSeek Harness（deepseek-flash）
**时间范围：** 2026-10-28 ～ 2026-11-03
**实验：** STM32F103 + uC/OS-III + AHT20

---

## 第 1 轮（2026-10-28）

**用户：** 任务一（uC/OS 移植、三任务）和任务二（I2C + AHT20）这个实验咋做呀？

**AI：** 给出整体路线：先裸机跑通 AHT20（任务二 2），再移植 uC/OS 完成任务一，最后把任务 3 换成采集（任务二 3）。说明所需软硬件（Keil、CubeMX、ST-Link、USB-TTL、AHT20）、接线（I2C1 PB6/PB7、USART1 PA9/PA10）、AHT20 命令（0xBE 初始化、0xAC 触发、80 ms、6 字节解析）和常见坑（上拉、地址 0x38<<1、MicroLIB、编码）。

## 第 2 轮

**用户：** 要先做任务二。

**AI：** 给出任务二裸机版的完整步骤：CubeMX 配置（HSE 8 MHz→72 MHz、I2C1 100 kHz、USART1 115200）、驱动文件（bsp.c/h、aht20.c/h）、main.c 三个 USER CODE 区只加十几行、串口现象、调试顺序（串口→地址扫描→初始化→测量）、软/硬件 I2C 对比答案、AHT20 手册要点。

## 第 3 轮

**用户：** 直接帮我弄 Keil 有关工程部分，弄好我来烧录。

**AI：** 检查本机环境，发现 Keil 5.37、STM32F1xx_DFP 2.4.0 已安装，于是直接用正点原子 MiniSTM32 V4（STM32F103RCT6）模板搭工程并编译出 hex，同时说明如果板子是 C8T6 需要改器件和启动文件。

## 第 4 轮

**用户：** 按 `D:\Project\STM32Project\stm32-oled-hal` 这块板子配置，在 `D:\Project\STM32Project` 下新建文件夹。

**AI：** 读取该工程，确认板子为 STM32F103C8T6，AHT20 原接 I2C2（PB10/PB11）、OLED 接 I2C1 重映射（PB8/PB9）、串口 USART1。新建 `aht20-i2c-hal` 工程，复用其 HAL/Device/CMSIS 与 AHT20 驱动，编译通过（Code 18.1 KB），并给出烧录说明。

## 第 5 轮

**用户：** 可以更换引脚。

**AI：** 把 AHT20 改到 I2C1 默认引脚 PB6/PB7（无需重映射），重新编译并同步到 `D:\Project\STM32Project\aht20-i2c-hal`。

## 第 6 轮

**用户：** 烧录后串口一直输出 `===== AH===== AH...`，啥意思？

**AI：** 判断为程序在打印标题时卡死：替换 main.c 时漏掉了原工程的 `SysTick_Handler`（`HAL_IncTick`），启动文件里的弱定义是 `B .` 死循环，1 ms 后 SysTick 进入死循环，正好停在输出 8 个字符处。补回 `SysTick_Handler` 并重新编译，hex 更新；同时给出供电、复位、看门狗的排查顺序。

## 第 7 轮

**用户：** 任务二 2) 和 3) 有啥区别？

**AI：** 2) 是裸机大循环 + `HAL_Delay(2000)`；3) 是把同一段采集代码放进 uC/OS-III 的任务 3，用 `OSTimeDlyHMSM` 让出 CPU，等待期间跑 LED 任务。给出对照表、代码差异、`HAL_Delay` 在 RTOS 中失效的原因（SysTick 被内核接管）、实验现象差别和报告写法。

## 第 8 轮

**用户：** 目前代码实现了哪几问？

**AI：** 说明现状：只有任务二 2) 编译通过、可烧录；任务一和任务二 3) 只有代码骨架，未移植 uC/OS；任务二 1) 只有文字答案；报告、GitHub、zip 未做。

## 第 9 轮（2026-11-03）

**用户：** 给出完整任务要求（含任务二 4 示波器观察），要求完成没做的部分，并写报告、博客、提交 GitHub。

**AI：** 从 GitHub 克隆 uC/OS-III 3.08.02、uC-CPU、uC-LIB；在 C8 工程上新增 8 个分组完成移植；解决 `CPU_CFG_NVIC_PRIO_BITS` 未定义、FPU 汇编、`__isb` 未定义、`os_cpu_c.c` 头文件路径等问题；编译出两个固件（hello 版 27 KB、AHT20 版 32 KB）；生成实验报告、博客、README、AI 对话记录并打包。

**关键命令与配置（AI 给出）：**

```c
/* cpu_cfg.h */
#define CPU_CFG_ENDIAN_TYPE     CPU_ENDIAN_TYPE_LITTLE
#define CPU_CFG_NVIC_PRIO_BITS  4u
#define CPU_CFG_KA_IPL_BOUNDARY 4u
```

```c
/* 中断对接 */
void SysTick_Handler(void) { HAL_IncTick(); OS_CPU_SysTickHandler(); }
void PendSV_Handler(void)  { OS_CPU_PendSVHandler(); }
```

```c
/* 任务切换开关：0 = 任务一 hello，1 = 任务二 3) AHT20 */
#define APP_TASK3_USE_AHT20   0
```

---

## 结论

本次对话完成了：环境确认、Keil 工程搭建、AHT20 驱动复用与引脚调整、SysTick 卡死问题定位、uC/OS-III 移植、两个版本固件编译、报告/博客/仓库文档撰写。所有可执行文件与源码均保存在 `D:\Project\STM32Project\` 下。
