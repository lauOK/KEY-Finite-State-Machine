/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-14     Tibbers       the first version
 */
#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f4xx_hal.h"

#define KEY_PORT    GPIOE
#define KEY_1       GPIO_PIN_3
#define KEY_0       GPIO_PIN_4

void Key_Init(void);
void Task_KeyScan(void);

#endif
