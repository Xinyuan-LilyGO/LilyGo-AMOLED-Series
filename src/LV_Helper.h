/**
 * @file      LV_Helper.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-04-20
 *
 */

#pragma once
#include <lvgl.h>
#include "LilyGo_Display.h"
#include "InputParams.h"


void beginLvglHelper(LilyGo_Display &board, bool debug = false);
void beginLvglHelperDMA(LilyGo_Display &board, bool debug = false);
void beginLvglInputDevice(struct InputParams prams);


