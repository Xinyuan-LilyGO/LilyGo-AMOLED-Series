/**
 * @file      InputParams.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-11-26
 *
 */


#pragma once

#include <freertos/queue.h>

struct InputData {
    char id;        // 'm' = mouse ,'k' = keyboard
    char key;
    bool left;
    bool right;
    int x;
    int y;
};

struct InputParams {
    QueueHandle_t queue;
    const void *icon;
};

