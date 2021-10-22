 /*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-14     Tibbers       the first version
 */
#include <drv_key.h>
#include <drv_led.h>

#define KEY_LONG_DOWN_DELAY     50

void Key_Init(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = KEY_0 | KEY_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStruct);
}

typedef enum _kEY_STATUS_LIST
{
    KEY_NULL = 0x00,    //无事件
    KEY_SURE = 0x01,    //确认状态
    KEY_DOWN = 0x02,    //按下
    KEY_LONG = 0x03,    //长按
    KEY_UP   = 0x04,    //弹起
}KEY_STATUS_LIST;

typedef enum _KEY_LIST
{
    KEY0 = 0,
    KEY1 = 1,
    KEY_NUM = 2,
}KEY_LIST;

typedef struct _KEY_COMPONENTS
{
    uint8_t KEY_SHIELD;       //按键屏蔽0:屏蔽，1:不屏蔽
    uint8_t KEY_COUNT;        //按键长按计数
    uint8_t KEY_LEVEL;        //虚拟当前IO电平，按下1，抬起0
    uint8_t KEY_DOWN_LEVEL;   //按下时IO实际的电平
    uint8_t KEY_STATUS;       //按键状态
    uint8_t KEY_EVENT;        //按键事件
    uint8_t (*READ_PIN)(void);//读IO电平函数
}KEY_COMPONENTS;

static uint8_t KEY0_ReadPin(void)
{
    return HAL_GPIO_ReadPin(KEY_PORT, KEY_0);
}

static uint8_t KEY1_ReadPin(void)
{
    return HAL_GPIO_ReadPin(KEY_PORT, KEY_1);
}

KEY_COMPONENTS Key_Buf[KEY_NUM] =
{
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY0_ReadPin},
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY1_ReadPin},
};

static void Get_Key_Value(void)
{
    uint8_t i;
    for (i = 0; i < KEY_NUM; ++i)
    {
        if (Key_Buf[i].KEY_SHIELD == 0)
            continue;
        if(Key_Buf[i].READ_PIN() == Key_Buf[i].KEY_DOWN_LEVEL)
            Key_Buf[i].KEY_LEVEL = 1;
        else
            Key_Buf[i].KEY_LEVEL = 0;
    }
}

void ReadKeyStatus(void)
{
    uint8_t i;
    Get_Key_Value();
    for(i = 0; i<KEY_NUM; i++)
    {
        switch(Key_Buf[i].KEY_STATUS)
        {
            //状态0:没有按键按下
            case KEY_NULL:
                if(Key_Buf[i].KEY_LEVEL == 1)//按键按下
                {
                    Key_Buf[i].KEY_STATUS = KEY_SURE;   //转入状态1
                    Key_Buf[i].KEY_EVENT = KEY_NULL;    //空事件
                }
                else
                {
                    Key_Buf[i].KEY_EVENT = KEY_NULL;
                }
                break;
            //状态1:按键状态确认
            case KEY_SURE:
                if(Key_Buf[i].KEY_LEVEL == 1)//按键按下
                {
                    Key_Buf[i].KEY_STATUS = KEY_DOWN;   //转入状态2
                    Key_Buf[i].KEY_EVENT = KEY_DOWN;    //按下事件
                    Key_Buf[i].KEY_COUNT = 0;           //计数器清零
                }
                else
                {
                    Key_Buf[i].KEY_STATUS = KEY_NULL;   //转入状态0
                    Key_Buf[i].KEY_EVENT = KEY_NULL;
                }
                break;
            //状态2:按键按下
            case KEY_DOWN:
                if(Key_Buf[i].KEY_LEVEL != 1)//按键释放
                {
                    Key_Buf[i].KEY_STATUS = KEY_NULL;   //转入状态0
                    Key_Buf[i].KEY_EVENT = KEY_UP;      //抬起事件
                }
                else if((Key_Buf[i].KEY_LEVEL == 1) && (++Key_Buf[i].KEY_COUNT >= KEY_LONG_DOWN_DELAY)) //超过KEY_LONG_DOWN_DELAY没有释放
                {
                    Key_Buf[i].KEY_STATUS = KEY_LONG;   //转入状态3
                    Key_Buf[i].KEY_EVENT = KEY_LONG;    //长按事件
                    Key_Buf[i].KEY_COUNT = 0;           //计数器清零
                }
                else
                {
                    Key_Buf[i].KEY_EVENT = KEY_NULL;    //空事件
                }
                break;
            //状态3:按键长按
            case KEY_LONG:
                if(Key_Buf[i].KEY_LEVEL != 1)//按键释放
                {
                    Key_Buf[i].KEY_STATUS = KEY_NULL;   //转入状态0
                    Key_Buf[i].KEY_EVENT = KEY_UP;      //抬起事件
                }
                else if((Key_Buf[i].KEY_LEVEL == 1) && (++Key_Buf[i].KEY_COUNT >= KEY_LONG_DOWN_DELAY)) //超过KEY_LONG_DOWN_DELAY没有释放
                {
                    Key_Buf[i].KEY_EVENT = KEY_LONG;    //长按事件
                    Key_Buf[i].KEY_COUNT = 0;           //计数器清零
                }
                else
                {
                Key_Buf[i].KEY_EVENT = KEY_NULL;        //空事件
                }
                break;
            //状态4:弹起
            case KEY_UP:
                if(Key_Buf[i].KEY_LEVEL != 1)//按键释放
                {
                    Key_Buf[i].KEY_STATUS = KEY_NULL;   //转入状态0
                    Key_Buf[i].KEY_EVENT = KEY_NULL;    //空事件
                }
                else
                {
                    Key_Buf[i].KEY_STATUS = KEY_SURE;   //按键确认状态
                    Key_Buf[i].KEY_EVENT = KEY_NULL;    //空事件
                }
        }
    }
}

void Task_KeyScan(void)
{
    ReadKeyStatus();
    if(Key_Buf[KEY0].KEY_EVENT == KEY_DOWN)
    {
        LOG_D("KEY0 Down");
    }
    else if(Key_Buf[KEY0].KEY_EVENT == KEY_LONG)
    {
        LOG_D("KEY0 Long Down");
    }
    else if(Key_Buf[KEY0].KEY_EVENT == KEY_UP)
    {
        LOG_D("KEY0 UP");
    }

    if(Key_Buf[KEY1].KEY_EVENT == KEY_DOWN)
    {
        LOG_D("KEY1 Down");
    }
    else if(Key_Buf[KEY1].KEY_EVENT == KEY_LONG)
    {
        LOG_D("KEY1 Long Down");
    }
    else if(Key_Buf[KEY1].KEY_EVENT == KEY_UP)
    {
        LOG_D("KEY1 UP");
    }
}
