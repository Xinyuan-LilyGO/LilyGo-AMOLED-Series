/**
 * @file      RM67162_AMOLED_SPI.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-05-28
 *
 */
#include "RM67162_AMOLED_SPI.h"
#include "initSequence.h"
#include <Arduino.h>

__BEGIN_DECLS

#define TAG  "rm67162"

static esp_lcd_panel_handle_t panel_handle;

static esp_err_t panel_rm67162_del(esp_lcd_panel_t *panel);
static esp_err_t panel_rm67162_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_rm67162_init(esp_lcd_panel_t *panel);
static esp_err_t panel_rm67162_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_rm67162_set_rotation(esp_lcd_panel_t *panel, uint8_t r);


static esp_err_t esp_lcd_new_panel_rm67162(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    rm67162_panel_t *rm67162 = NULL;
    uint8_t fb_bits_per_pixel = 0;

    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    rm67162 = (rm67162_panel_t *)calloc(1, sizeof(rm67162_panel_t));
    ESP_GOTO_ON_FALSE(rm67162, ESP_ERR_NO_MEM, err, TAG, "no mem for rm67162 panel");
    if (panel_dev_config->reset_gpio_num >= 0) {
        pinMode(panel_dev_config->reset_gpio_num, OUTPUT);
    }
    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565
        rm67162->colmod_cal = 0x55;
        fb_bits_per_pixel = 16;
        break;
    case 18: // RGB666
        rm67162->colmod_cal = 0x66;
        // each color component (R/G/B) should occupy the 6 high bits of a byte, which means 3 full bytes are required for a pixel
        fb_bits_per_pixel = 24;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }
    rm67162->io = io;
    rm67162->fb_bits_per_pixel = fb_bits_per_pixel;
    rm67162->reset_gpio_num = panel_dev_config->reset_gpio_num;
    rm67162->reset_level = panel_dev_config->flags.reset_active_high;
    rm67162->base.del = panel_rm67162_del;
    rm67162->base.reset = panel_rm67162_reset;
    rm67162->base.init = panel_rm67162_init;
    rm67162->base.draw_bitmap = panel_rm67162_draw_bitmap;
    rm67162->base.invert_color = NULL;
    rm67162->base.set_gap = NULL;
    rm67162->base.mirror = NULL;
    rm67162->base.swap_xy = NULL;
    *ret_panel = &(rm67162->base);
    log_d("new rm67162 panel @%p", rm67162);
    return ESP_OK;
err:
    if (rm67162) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            pinMode(panel_dev_config->reset_gpio_num, OPEN_DRAIN);
        }
        free(rm67162);
    }
    return ret;
}


static esp_err_t panel_rm67162_del(esp_lcd_panel_t *panel)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    if (rm67162->reset_gpio_num >= 0) {
        pinMode(rm67162->reset_gpio_num, OPEN_DRAIN);
    }
    log_d("del rm67162 panel @%p", rm67162);
    free(rm67162);
    return ESP_OK;
}

static esp_err_t panel_rm67162_reset(esp_lcd_panel_t *panel)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    esp_lcd_panel_io_handle_t io = rm67162->io;
    if (rm67162->reset_gpio_num >= 0) {
        digitalWrite(rm67162->reset_gpio_num, rm67162->reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(rm67162->reset_gpio_num, !rm67162->reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
    } else {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    return ESP_OK;
}

static esp_err_t panel_rm67162_init(esp_lcd_panel_t *panel)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    esp_lcd_panel_io_handle_t io = rm67162->io;
    int cmd = 0;
    while (rm67162_spi_cmd[cmd].len != 0xff) {
        esp_lcd_panel_io_tx_param(io, rm67162_spi_cmd[cmd].addr, rm67162_spi_cmd[cmd].param, rm67162_spi_cmd[cmd].len & 0x7F);
        if (rm67162_spi_cmd[cmd].len & 0x80) {
            vTaskDelay(pdMS_TO_TICKS(120));
        }
        cmd++;
    }
    rm67162->rotation = 0;

    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));
    esp_lcd_panel_io_tx_param(io, rm67162->colmod_cal, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));
    panel_rm67162_set_rotation(panel, rm67162->rotation);
    return ESP_OK;
}

static esp_err_t panel_rm67162_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    esp_lcd_panel_io_handle_t io = rm67162->io;
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    uint8_t data1[] = {lowByte(x_start >> 8), lowByte(x_start), lowByte((x_end - 1) >> 8), lowByte(x_end - 1)};
    esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, data1, 4);
    uint8_t data2[] = {lowByte(y_start >> 8), lowByte(y_start), lowByte((y_end - 1) >> 8), lowByte(y_end - 1)};
    esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, data2, 4);
    size_t write_colors_bytes = (x_end - x_start) * (y_end - y_start) * rm67162->fb_bits_per_pixel / 8;
    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, write_colors_bytes);
    return ESP_OK;
}



static esp_err_t panel_rm67162_set_rotation(esp_lcd_panel_t *panel, uint8_t r)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    esp_lcd_panel_io_handle_t io = rm67162->io;
    uint8_t write_data = 0x00;
    switch (r) {
    case 1:
        write_data = RM67162_MADCTL_RGB;
        rm67162->height = RM67162_HEIGHT;
        rm67162->width = RM67162_WIDTH;
        break;
    case 2:
        write_data =  RM67162_MADCTL_MV | RM67162_MADCTL_MY | RM67162_MADCTL_RGB;
        rm67162->height = RM67162_WIDTH;
        rm67162->width = RM67162_HEIGHT;
        break;
    case 3:
        write_data = RM67162_MADCTL_MX | RM67162_MADCTL_MY | RM67162_MADCTL_RGB;
        rm67162->height = RM67162_HEIGHT;
        rm67162->width = RM67162_WIDTH;
        break;
    default: // case 0:
        write_data = RM67162_MADCTL_MX | RM67162_MADCTL_MV | RM67162_MADCTL_RGB;
        rm67162->height = RM67162_WIDTH;
        rm67162->width = RM67162_HEIGHT;
        break;
    }
    rm67162->rotation = r;
    // log_i("set_rotation:%d write reg :0x%X , data : 0x%X Width:%d Height:%d", r, LCD_CMD_MADCTL, write_data, rm67162->width, rm67162->height);
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &write_data, 1);
    return ESP_OK;
}
__END_DECLS



esp_err_t panel_rm67162_tx_colors(esp_lcd_panel_t *panel, const void *color_data, size_t colors_size)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    esp_lcd_panel_io_handle_t io = rm67162->io;
    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, colors_size);
    return ESP_OK;
}

esp_err_t panel_rm67162_write_command(esp_lcd_panel_t *panel, uint32_t cmd, uint8_t *pdat, uint32_t length)
{
    rm67162_panel_t *rm67162 = __containerof(panel, rm67162_panel_t, base);
    esp_lcd_panel_io_handle_t io = rm67162->io;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, cmd, pdat, length), TAG, "io tx command failed");
    return ESP_OK;
}

esp_lcd_panel_handle_t *panel_rm67162_init_spi_bus(spi_host_device_t  host_id, int mosi, int miso, int sck, int dc, int cs, int rst, uint32_t speed_clk_mhz)
{
    spi_bus_config_t buscfg = {0};
    buscfg.mosi_io_num = mosi;
    buscfg.miso_io_num = miso;
    buscfg.sclk_io_num = sck;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.data4_io_num = 0;
    buscfg.data5_io_num = 0;
    buscfg.data6_io_num = 0;
    buscfg.data7_io_num = 0;
    buscfg.max_transfer_sz = RM67162_WIDTH * 80 * sizeof(uint16_t); 
    buscfg.flags = 0x00;
    buscfg.intr_flags = 0x00;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
    buscfg.isr_cpu_id = INTR_CPU_ID_AUTO;
#endif

    ESP_ERROR_CHECK(spi_bus_initialize(host_id, &buscfg, SPI_DMA_CH_AUTO));

    log_i( "Install panel IO");

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {0};
    io_config.cs_gpio_num = cs;
    io_config.dc_gpio_num = dc;
    io_config.spi_mode = 0;
    io_config.pclk_hz = speed_clk_mhz;
    io_config.trans_queue_depth = 30;
    io_config.on_color_trans_done = NULL;
    io_config.user_ctx = NULL;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.flags.dc_low_on_data = 0;
    io_config.flags.octal_mode = 0;
    io_config.flags.lsb_first = 0;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
    io_config.flags.quad_mode = 0;
    io_config.flags.sio_mode = 0;
    io_config.flags.cs_high_active = 0;
#else
    io_config.flags.dc_as_cmd_phase = 0;
#endif

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t )host_id, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {0};
    panel_config.reset_gpio_num = rst;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    panel_config.color_space = ESP_LCD_COLOR_SPACE_RGB;
#else
    panel_config.color_space = LCD_RGB_ELEMENT_ORDER_RGB;
#endif
    panel_config.flags.reset_active_high = 0;
    panel_config.vendor_config = NULL;
    panel_config.bits_per_pixel = 16;

    log_i( "Install RM67162 panel driver");

    ESP_ERROR_CHECK(esp_lcd_new_panel_rm67162(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    return &panel_handle;
}


