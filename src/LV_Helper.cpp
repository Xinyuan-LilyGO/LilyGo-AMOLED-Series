/**
 * @file      LV_Helper.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-04-20
 *
 */
#include "LilyGo_AMOLED.h"
#include <lvgl.h>


/* Display flushing */
#if LV_VERSION_CHECK(9,0,0)
static void disp_flush( lv_disp_t *disp, const lv_area_t *area, lv_color_t *color_p )
#else
static void disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
#endif
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );
    amoled.pushColors(area->x1, area->y1, w, h, (uint16_t *)color_p);
    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
#if LV_VERSION_CHECK(9,0,0)
static void touchpad_read( lv_indev_t *indev_driver, lv_indev_data_t *data )
#else
static void touchpad_read( lv_indev_drv_t *indev_driver, lv_indev_data_t *data )
#endif
{
    static int16_t x, y;
    uint8_t touched = amoled.getPoint(&x, &y);
    if ( touched ) {
        data->point.x = y;
        // data->point.y = amoled.width() - x;
        data->point.y = amoled.width() - x;
        data->state = LV_INDEV_STATE_PR;
        return;

    }
    data->state = LV_INDEV_STATE_REL;
}

#ifndef BOARD_HAS_PSRAM
#error "Please turn on PSRAM to OPI !"
#else
static lv_color_t *buf = NULL;
#endif

#if LV_USE_LOG
#if LV_VERSION_CHECK(9,0,0)
void lv_log_print_g_cb(lv_log_level_t level, const char *buf)
#else
void lv_log_print_g_cb(const char *buf)
#endif
{
    Serial.println(buf);
    Serial.flush();
}
#endif

void beginLvglHelper(bool debug)
{
    lv_init();

#if LV_USE_LOG
    if (debug) {
        lv_log_register_print_cb(lv_log_print_g_cb);
    }
#endif

    size_t lv_buffer_size = amoled.width() * amoled.height() * sizeof(lv_color_t);
    buf = (lv_color_t *)ps_malloc(lv_buffer_size);
    assert(buf);

#if LV_VERSION_CHECK(9,0,0)
    lv_disp_t *disp =  lv_disp_create(amoled.width(), amoled.height());

#ifdef BOARD_HAS_PSRAM
    lv_disp_set_draw_buffers(disp, buf, buf1, amoled.width() * amoled.height(), LV_DISP_RENDER_MODE_PARTIAL);
#else
    lv_disp_set_draw_buffers(disp, buf, NULL, sizeof(buf), LV_DISP_RENDER_MODE_PARTIAL);
#endif
    lv_disp_set_res(disp, amoled.width(), amoled.height());
    lv_disp_set_physical_res(disp, amoled.width(), amoled.height());
    lv_disp_set_flush_cb(disp, disp_flush);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_read_cb(indev, touchpad_read);
    lv_indev_set_disp(indev, disp);
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
#else


    static lv_disp_draw_buf_t draw_buf;
    static lv_disp_drv_t disp_drv;
    static lv_indev_drv_t  indev_drv;

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, lv_buffer_size);

    /*Initialize the display*/
    lv_disp_drv_init( &disp_drv );
    /* display resolution */
    disp_drv.hor_res = amoled.height();
    disp_drv.ver_res = amoled.width();

    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register( &disp_drv );

    if (amoled.getBoarsdConfigure()) {
        if (amoled.getBoarsdConfigure()->hasTouch) {
            lv_indev_drv_init( &indev_drv );
            indev_drv.type = LV_INDEV_TYPE_POINTER;
            indev_drv.read_cb = touchpad_read;
            lv_indev_drv_register( &indev_drv );
        }
    }

#endif
}