/**
 * @file      RM67162_AMOLED_SPI.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-05-29
 * 
 */

#pragma once


#include <sys/cdefs.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_commands.h>
#include <esp_check.h>
#include <hal/spi_types.h>
#include <driver/spi_common.h>


#ifndef LCD_CMD_MADCTL
#define LCD_CMD_MADCTL       0x36     // Memory data access control
#endif
#ifndef LCD_CMD_CASET
#define LCD_CMD_CASET        0x2A // Set column address
#endif

#ifndef LCD_CMD_RASET
#define LCD_CMD_RASET        0x2B // Set row address
#endif

#ifndef LCD_CMD_RAMWR
#define LCD_CMD_RAMWR        0x2C // Write frame memory
#endif


#ifndef LCD_CMD_SLPIN
#define LCD_CMD_SLPIN        0x10 // Go into sleep mode (DC/DC, oscillator, scanning stopped, but memory keeps content)
#endif

#ifndef LCD_CMD_BRIGHTNESS
#define LCD_CMD_BRIGHTNESS  0x51
#endif

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint8_t fb_bits_per_pixel;
    uint8_t colmod_cal; // save surrent value of LCD_CMD_COLMOD register
    uint8_t rotation;
    uint16_t width;
    uint16_t height;
} rm67162_panel_t;



esp_lcd_panel_handle_t *panel_rm67162_init_spi_bus(spi_host_device_t  host_id, int mosi, int miso, int sck, int dc, int cs, int rst, uint32_t speed_clk_mhz);
esp_err_t panel_rm67162_write_command(esp_lcd_panel_t *panel, uint32_t cmd, uint8_t *pdat, uint32_t length);
esp_err_t panel_rm67162_tx_colors(esp_lcd_panel_t *panel, const void *color_data, size_t colors_size);


















