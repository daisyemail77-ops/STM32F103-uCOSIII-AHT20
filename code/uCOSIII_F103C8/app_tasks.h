#ifndef __APP_TASKS_H
#define __APP_TASKS_H

/* 0: 任务3 每 2s 打印 hello uc/OS!（任务一要求）
 * 1: 任务3 每 2s 采集 AHT20 并通过串口输出（任务二 3) 要求）
 */
#ifndef APP_TASK3_USE_AHT20
#define APP_TASK3_USE_AHT20   0
#endif

void AppTaskCreate(void);

#endif
