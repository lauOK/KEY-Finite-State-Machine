/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-19     Tibbers       the first version
 */

#include "drv_key.h"

int Task_Key(void)
{
    Key_Init();
  
    while(1)
    {
        Task_KeyScan();
        mdelay(20);
    }
    return 0;
}
