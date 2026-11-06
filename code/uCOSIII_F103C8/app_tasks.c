#include "app_tasks.h"
#include "os.h"
#include "bsp.h"
#include "aht20.h"
#include <stdio.h>

/* 串口助手（XCOM 等）默认 GBK，这里直接输出 GBK 字节，避免中文乱码。
 * 对应文字：欢迎来到RTOS多任务环境！ */
#define HELLO_CN  "\xBB\xB6\xD3\xAD\xC0\xB4\xB5\xBD" "RTOS" "\xB6\xE0\xC8\xCE\xCE\xF1\xBB\xB7\xBE\xB3\xA3\xA1"

#define TASK1_PRIO      5u
#define TASK2_PRIO      6u
#define TASK3_PRIO      7u
#define TASK_STK_SIZE   512u

static OS_TCB  Task1TCB;
static OS_TCB  Task2TCB;
static OS_TCB  Task3TCB;

static CPU_STK Task1Stk[TASK_STK_SIZE];
static CPU_STK Task2Stk[TASK_STK_SIZE];
static CPU_STK Task3Stk[TASK_STK_SIZE];

static void AppTaskLed1(void *p_arg);
static void AppTaskLed2(void *p_arg);
#if (APP_TASK3_USE_AHT20 > 0)
static void AppTaskAHT20(void *p_arg);
#else
static void AppTaskHello(void *p_arg);
#endif

/* uC/OS-III V3.04 及以后 OSTaskCreate 比 V3.03 多两个参数 */
#if (defined(OS_VERSION) && (OS_VERSION >= 30004u))
#define OSTASK_CREATE(tcb, name, fn, prio, stk)                                \
    OSTaskCreate((tcb), (name), (fn), (void *)0, (prio), (stk),                \
                 (CPU_STK_SIZE)(TASK_STK_SIZE / 10u), (CPU_STK_SIZE)TASK_STK_SIZE, \
                 (OS_MSG_QTY)0, (OS_TICK)0, (void *)0,                         \
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), &err)
#else
#define OSTASK_CREATE(tcb, name, fn, prio, stk)                                \
    OSTaskCreate((tcb), (name), (fn), (void *)0, (prio), (stk),                \
                 (CPU_STK_SIZE)(TASK_STK_SIZE / 10u), (CPU_STK_SIZE)TASK_STK_SIZE, \
                 (void *)0, (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), &err)
#endif

void AppTaskCreate(void)
{
    OS_ERR err = OS_ERR_NONE;

#if (APP_TASK3_USE_AHT20 > 0)
    OSTASK_CREATE(&Task3TCB, (CPU_CHAR *)"Task3_AHT20", AppTaskAHT20, TASK3_PRIO, &Task3Stk[0]);
#else
    OSTASK_CREATE(&Task3TCB, (CPU_CHAR *)"Task3_Hello", AppTaskHello, TASK3_PRIO, &Task3Stk[0]);
#endif
    OSTASK_CREATE(&Task1TCB, (CPU_CHAR *)"Task1_LED1",  AppTaskLed1,  TASK1_PRIO, &Task1Stk[0]);
    OSTASK_CREATE(&Task2TCB, (CPU_CHAR *)"Task2_LED2",  AppTaskLed2,  TASK2_PRIO, &Task2Stk[0]);

    if (err != OS_ERR_NONE) {
        printf("task create error, code=%d\r\n", (int)err);
    }
}

/* 任务1：每 1s 翻转 LED1（PC13） */
static void AppTaskLed1(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;
    while (1) {
        BSP_LED_Toggle(0u);
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

/* 任务2：每 3s 翻转 LED2（PB0） */
static void AppTaskLed2(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;
    while (1) {
        BSP_LED_Toggle(1u);
        OSTimeDlyHMSM(0u, 0u, 3u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

#if (APP_TASK3_USE_AHT20 > 0)

/* 任务3（任务二 3）：每 2s 采集 AHT20 并通过串口输出 */
static void AppTaskAHT20(void *p_arg)
{
    OS_ERR   err;
    int16_t  temp10 = 0;
    uint16_t humi10 = 0;
    uint8_t  ret;

    (void)p_arg;

    ret = AHT20_Init(&hi2c1);
    if (ret == 0u) {
        printf("AHT20 init OK\r\n");
    } else {
        printf("AHT20 init FAILED, code=%u\r\n", ret);
    }

    while (1) {
        ret = AHT20_ReadData(&hi2c1, &temp10, &humi10);
        if (ret == 0u) {
            printf("Temp: %d.%d C, Humi: %d.%d %%RH\r\n",
                   (int)(temp10 / 10),
                   (int)((temp10 % 10 < 0) ? -(temp10 % 10) : (temp10 % 10)),
                   (int)(humi10 / 10),
                   (int)(humi10 % 10));
        } else {
            printf("AHT20 read FAILED, code=%u\r\n", ret);
        }
        OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

/* 把 AHT20 驱动里的 HAL_Delay 换成 OS 延时，等待 80ms 时让出 CPU */
void AHT20_DelayMs(uint32_t ms)
{
    OS_ERR err;

    if (OSRunning == OS_STATE_OS_RUNNING) {
        OSTimeDlyHMSM(0u, 0u, 0u, ms, OS_OPT_TIME_HMSM_STRICT, &err);
    } else {
        HAL_Delay(ms);
    }
}

#else

/* 任务3（任务一）：每 2s 通过串口发送 hello uc/OS! */
static void AppTaskHello(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;
    while (1) {
        printf("hello uc/OS! " HELLO_CN "\r\n");
        OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

#endif
