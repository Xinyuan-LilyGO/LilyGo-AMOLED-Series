/**
 * @file      initSequence.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-05-29
 *
 */
#pragma once

#include <stdint.h>

typedef struct {
    uint32_t addr;
    uint8_t param[5];
    uint32_t len;
} lcd_cmd_t;

#define AMOLED_DEFAULT_BRIGHTNESS               175

#define SH8501_INIT_SEQUENCE_LENGHT             407
extern const lcd_cmd_t sh8501_cmd[SH8501_INIT_SEQUENCE_LENGHT];
#define SH8501_WIDTH                            194
#define SH8501_HEIGHT                           368


#define RM67162_INIT_SEQUENCE_LENGHT             6
extern const lcd_cmd_t rm67162_cmd[RM67162_INIT_SEQUENCE_LENGHT];
#define RM67162_WIDTH                            240
#define RM67162_HEIGHT                           536













