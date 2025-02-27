/**
 * @file      LV_Helper_v9.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2025-02-27
 * @note      Adapt to lvgl 9 version
 */
#include <Arduino.h>
#include "LV_Helper.h"

#if LVGL_VERSION_MAJOR == 9

static lv_display_t *disp_drv;
static lv_draw_buf_t draw_buf;
static lv_indev_t *indev_drv;
static lv_indev_t *indev_mouse;
static lv_indev_t *indev_keypad;

static lv_color16_t *buf  = NULL;
static lv_color16_t *buf1  = NULL;

static lv_indev_t  *mouse_indev = NULL;
static lv_indev_t  *kb_indev = NULL;
static struct InputParams params_copy;

static void disp_flush( lv_display_t *disp_drv, const lv_area_t *area, uint8_t *color_p)
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );
    auto *plane = (LilyGo_Display *)lv_display_get_user_data(disp_drv);
    lv_draw_sw_rgb565_swap(color_p, w * h * 2);
    plane->pushColors(area->x1, area->y1, w, h, (uint16_t *)color_p);
    lv_display_flush_ready( disp_drv );
}

/*Read the touchpad*/
static void touchpad_read( lv_indev_t *indev, lv_indev_data_t *data )
{
    static int16_t x, y;
    auto *plane = (LilyGo_Display *)lv_indev_get_user_data(indev);
    uint8_t touched = plane->getPoint(&x, &y, 1);
    if ( touched ) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
        return;
    }
    data->state = LV_INDEV_STATE_REL;
}

static uint32_t lv_tick_get_cb(void)
{
    return millis();
}

static void mouse_read( lv_indev_t *indev, lv_indev_data_t *data )
{
    static int16_t last_x;
    static int16_t last_y;
    struct InputData msg;
    const lv_img_dsc_t *cur = (const lv_img_dsc_t *)params_copy.icon;
    uint16_t _maxX = lv_disp_get_hor_res(NULL) - cur->header.w;
    uint16_t _maxY = lv_disp_get_ver_res(NULL) - cur->header.h;

    if (xQueueReceive(params_copy.queue, &msg, pdTICKS_TO_MS(30)) == pdPASS) {
        if (msg.id == 'm') {
            last_x = constrain(msg.x, 0, _maxX);
            last_y = constrain(msg.y, 0, _maxY);
            data->state = (msg.left || msg.right) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        }
    }
    data->point.x = last_x;
    data->point.y = last_y;
}

static void keypad_read( lv_indev_t *indev, lv_indev_data_t *data )
{
    static uint32_t last_key = 0;
    uint32_t act_key ;
    struct InputData msg;
    if (xQueueReceive(params_copy.queue, &msg, pdTICKS_TO_MS(30)) == pdPASS) {
        if (msg.id == 'k') {
            last_key =  msg.key;
            data->key = msg.key;
            data->state = LV_INDEV_STATE_PR;
            return;
        }
    }
    data->state = LV_INDEV_STATE_REL;
    data->key = last_key;
}

static void lv_rounder_cb(lv_event_t *e)
{
    lv_area_t *area = (lv_area_t *)lv_event_get_param(e);
    if (area->x1 & 1)
        area->x1--;
    if (!(area->x2 & 1))
        area->x2++;
    if (area->y1 & 1)
        area->y1--;
    if (!(area->y2 & 1))
        area->y2++;
}

void beginLvglHelper(LilyGo_Display &board, bool debug)
{

    lv_init();

#if LV_USE_LOG
    if (debug) {
        lv_log_register_print_cb(lv_log_print_g_cb);
    }
#endif

    size_t lv_buffer_size = board.width() * board.height() * sizeof(lv_color16_t);

    buf = (lv_color16_t *)ps_malloc(lv_buffer_size);
    assert(buf);

    buf1 = (lv_color16_t *)ps_malloc(lv_buffer_size);
    assert(buf1);

    disp_drv = lv_display_create(board.width(), board.height());

    if (board.needFullRefresh()) {
        lv_display_set_buffers(disp_drv, buf, buf1, lv_buffer_size, LV_DISPLAY_RENDER_MODE_FULL);
    } else {
        lv_display_set_buffers(disp_drv, buf, buf1, lv_buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_display_add_event_cb(disp_drv, lv_rounder_cb, LV_EVENT_INVALIDATE_AREA, NULL);
    }

    lv_display_set_color_format(disp_drv, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp_drv, disp_flush);
    lv_display_set_user_data(disp_drv, &board);

    if (board.hasTouch()) {
        indev_drv = lv_indev_create();
        lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev_drv, touchpad_read);
        lv_indev_set_user_data(indev_drv, &board);
        lv_indev_enable(indev_drv, true);
        lv_indev_set_display(indev_drv, disp_drv);
    }

    lv_tick_set_cb(lv_tick_get_cb);

    lv_group_set_default(lv_group_create());
}

void beginLvglInputDevice(struct InputParams prams)
{
    memcpy(&params_copy, &prams, sizeof(struct InputParams));

    if (!mouse_indev) {
        /*Register a mouse input device*/
        indev_mouse = lv_indev_create();
        lv_indev_set_type(indev_mouse, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev_mouse, mouse_read);
        lv_indev_enable(indev_mouse, true);
        lv_indev_set_display(indev_mouse, disp_drv);
    }

    lv_obj_t *cursor = lv_image_create(lv_scr_act());
    lv_image_set_src(cursor, params_copy.icon);
    lv_indev_set_cursor(mouse_indev, cursor);

    /*Register a keypad input device*/
    if (!kb_indev) {
        indev_keypad = lv_indev_create();
        lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(indev_keypad, keypad_read);
        lv_indev_enable(indev_keypad, true);
        lv_indev_set_display(indev_keypad, disp_drv);
        lv_indev_set_group(kb_indev, lv_group_get_default());
    }
}

#endif
