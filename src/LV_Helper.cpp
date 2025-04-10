/**
 * @file      LV_Helper.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-04-20
 * @note      Adapt to lvgl 8 version
 */
#include <Arduino.h>
#include "LV_Helper.h"


#if LVGL_VERSION_MAJOR == 8

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t  indev_drv;
static lv_indev_t  *mouse_indev = NULL;
static lv_indev_t  *kb_indev = NULL;
static lv_indev_drv_t indev_mouse;
static lv_indev_drv_t indev_keypad;
static struct InputParams params_copy;

/* Display flushing */
static void disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );
    static_cast<LilyGo_Display *>(disp_drv->user_data)->pushColors(area->x1, area->y1, w, h, (uint16_t *)color_p);
    lv_disp_flush_ready( disp_drv );
}

static void disp_flushDMA( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );
    static_cast<LilyGo_Display *>(disp_drv->user_data)->setAddrWindow(area->x1, area->y1, area->x2, area->y2);
    static_cast<LilyGo_Display *>(disp_drv->user_data)->pushColorsDMA((uint16_t *)color_p, w * h);

    lv_disp_flush_ready( disp_drv );
}

/*Read the touchpad*/
static void touchpad_read( lv_indev_drv_t *indev_driver, lv_indev_data_t *data )
{
    static int16_t x, y;
    uint8_t touched =   static_cast<LilyGo_Display *>(indev_driver->user_data)->getPoint(&x, &y, 1);
    if ( touched ) {
        data->point.x = x;
        data->point.y = y;
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
void lv_log_print_g_cb(const char *buf)
{
    Serial.println(buf);
    Serial.flush();
}
#endif

static void mouse_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
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

static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
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


static void lv_rounder_cb(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    // make sure all coordinates are even
    if (area->x1 & 1)
        area->x1--;
    if (!(area->x2 & 1))
        area->x2++;
    if (area->y1 & 1)
        area->y1--;
    if (!(area->y2 & 1))
        area->y2++;
}

void beginLvglHelperDMA(LilyGo_Display &board, bool debug) {
    lv_init();

#if LV_USE_LOG
    if (debug) {
        lv_log_register_print_cb(lv_log_print_g_cb);
    }
#endif

    size_t lv_buffer_size = (board.width() * board.height() / 10) * sizeof(lv_color_t);

    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(lv_buffer_size, MALLOC_CAP_DMA);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(lv_buffer_size, MALLOC_CAP_DMA);

    assert (buf1 && buf2);

    if (!esp_ptr_dma_capable(buf1) || !esp_ptr_dma_capable(buf2)) {
        Serial.println("Error: Buffers are not DMA-capable!");
    }

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, board.width() * board.height() / 10);

    /*Initialize the display*/
    lv_disp_drv_init( &disp_drv );
    /* display resolution */
    disp_drv.hor_res = board.width();
    disp_drv.ver_res = board.height();
    disp_drv.flush_cb = disp_flushDMA;
    disp_drv.draw_buf = &draw_buf;
    bool full_refresh = board.needFullRefresh();
    disp_drv.full_refresh = full_refresh;
    disp_drv.user_data = &board;
    if (!full_refresh) {
        disp_drv.rounder_cb = lv_rounder_cb;
    }
    lv_disp_drv_register( &disp_drv );

    if (board.hasTouch()) {
        lv_indev_drv_init( &indev_drv );
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touchpad_read;
        indev_drv.user_data = &board;
        lv_indev_drv_register( &indev_drv );
    }

    lv_group_set_default(lv_group_create());
}

void beginLvglHelper(LilyGo_Display &board, bool debug)
{

    lv_init();

#if LV_USE_LOG
    if (debug) {
        lv_log_register_print_cb(lv_log_print_g_cb);
    }
#endif

    size_t lv_buffer_size = board.width() * board.height() * sizeof(lv_color_t);
    buf = (lv_color_t *)ps_malloc(lv_buffer_size);
    assert(buf);

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, board.width() * board.height());

    /*Initialize the display*/
    lv_disp_drv_init( &disp_drv );
    /* display resolution */
    disp_drv.hor_res = board.width();
    disp_drv.ver_res = board.height();
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    bool full_refresh = board.needFullRefresh();
    disp_drv.full_refresh = full_refresh;
    disp_drv.user_data = &board;
    if (!full_refresh) {
        disp_drv.rounder_cb = lv_rounder_cb;
    }
    lv_disp_drv_register( &disp_drv );

    if (board.hasTouch()) {
        lv_indev_drv_init( &indev_drv );
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touchpad_read;
        indev_drv.user_data = &board;
        lv_indev_drv_register( &indev_drv );
    }

    lv_group_set_default(lv_group_create());
}

void beginLvglInputDevice(struct InputParams prams)
{
    memcpy(&params_copy, &prams, sizeof(struct InputParams));

    if (!mouse_indev) {
        /*Register a mouse input device*/
        lv_indev_drv_init( &indev_mouse );
        indev_mouse.type = LV_INDEV_TYPE_POINTER;
        indev_mouse.read_cb = mouse_read;
        mouse_indev = lv_indev_drv_register( &indev_mouse );
    }

    lv_obj_t *cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(cursor, params_copy.icon);
    lv_indev_set_cursor(mouse_indev, cursor);

    /*Register a keypad input device*/
    if (!kb_indev) {
        lv_indev_drv_init(&indev_keypad);
        indev_keypad.type = LV_INDEV_TYPE_KEYPAD;
        indev_keypad.read_cb = keypad_read;
        kb_indev = lv_indev_drv_register(&indev_keypad);
        lv_indev_set_group(kb_indev, lv_group_get_default());
    }
}

#endif
