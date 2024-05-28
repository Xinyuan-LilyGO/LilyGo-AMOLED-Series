/**
 * @file      TouchPaint.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-05-28
 *
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;

void initPaint();

struct {
    /**
     * root
     *   -> toolbar
     *     -> color
     *     -> brush
     *   -> Canvas
     *   -> Colorwheel
    */
    lv_obj_t *root;
    lv_obj_t *toolbarDiv;
    lv_obj_t *brushColorButton;
    lv_obj_t *brushButton;
    lv_obj_t *refreshButton;
    lv_obj_t *brushColorwheel;
    lv_obj_t *brushWidthSlider;
    lv_obj_t *canvas;
    lv_draw_line_dsc_t brush;
} ui;


void setup(void)
{
    Serial.begin(115200);

    bool rslt = false;

    // Begin LilyGo  1.47 Inch AMOLED board class
    //rslt = amoled.beginAMOLED_147();

    // Begin LilyGo  1.91 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_191();

    // Begin LilyGo  2.41 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_241();

    // Automatically determine the access device
    rslt = amoled.begin();

    if (!rslt) {
        while (1) {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }

    // Register lvgl helper
    beginLvglHelper(amoled);

    // Painting touch board
    initPaint();
}

void loop()
{
    lv_task_handler();
    delay(5);
}




void onCanvasEvent(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    static lv_coord_t last_x, last_y = -32768;

    if (code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_indev_get_act();
        if (indev == NULL) return;

        lv_point_t point;
        lv_indev_get_point(indev, &point);
        lv_point_t points[2];
        if ((last_x == -32768) || (last_y == -32768)) {
            last_x = point.x;
            last_y = point.y;
        } else {
            points[0].x = last_x;
            points[0].y = last_y;
            points[1].x = point.x;
            points[1].y = point.y;
            last_x = point.x;
            last_y = point.y;
            lv_canvas_draw_line(obj, points, 2, &ui.brush);
        }
    } else if (code == LV_EVENT_RELEASED) {
        last_x = -32768;
        last_y = -32768;
    }

}


void onBrushColorLabelEvent(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(ui.brushWidthSlider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui.brushColorwheel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_event_cb(ui.canvas, onCanvasEvent);
    }
}


void onBrushLabelEvent(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(ui.brushColorwheel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui.brushWidthSlider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_event_cb(ui.canvas, onCanvasEvent);
    }
}


void onBrushWidthSliderEvent(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target(e);
    if (code == LV_EVENT_DEFOCUSED) {
        lv_obj_add_flag(ui.brushWidthSlider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_event_cb(ui.canvas, onCanvasEvent, LV_EVENT_ALL, NULL);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        ui.brush.width = (lv_coord_t)lv_slider_get_value(slider);
    }
}



void onBrushColorwheelEvent(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *colorwheel = lv_event_get_target(e);
    if (code == LV_EVENT_DEFOCUSED) {
        lv_obj_add_flag(ui.brushColorwheel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_event_cb(ui.canvas, onCanvasEvent, LV_EVENT_ALL, NULL);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        ui.brush.color = lv_colorwheel_get_rgb(colorwheel);
    }
}


void onRefreshLabelEvent(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {

        lv_obj_add_flag(ui.brushWidthSlider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui.brushColorwheel, LV_OBJ_FLAG_HIDDEN);

        lv_canvas_fill_bg(
            ui.canvas,
            lv_palette_lighten(LV_PALETTE_GREY, 3),
            LV_OPA_COVER
        );
    }
}


void initPaint()
{
    ui.root = lv_obj_create(NULL);

    ui.toolbarDiv = lv_obj_create(ui.root);
    lv_obj_align(ui.toolbarDiv, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_grow(ui.toolbarDiv, 1);
    lv_obj_set_flex_flow(ui.toolbarDiv, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(ui.toolbarDiv, LV_PCT(60), LV_PCT(20));
    lv_obj_add_flag(ui.toolbarDiv, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(ui.toolbarDiv, LV_OPA_TRANSP, LV_PART_MAIN);

    ui.brushColorButton  =  lv_btn_create(ui.toolbarDiv);
    lv_obj_set_size(ui.brushColorButton, lv_pct(30), lv_pct(100));
    lv_obj_t *label = lv_label_create(ui.brushColorButton);
    lv_label_set_text(label, "Pen");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_add_flag(ui.brushColorButton, LV_OBJ_FLAG_CLICKABLE);

    ui.brushButton =  lv_btn_create(ui.toolbarDiv);
    lv_obj_set_size(ui.brushButton, lv_pct(30), lv_pct(100));
    label = lv_label_create(ui.brushButton);
    lv_label_set_text(label, "Size");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_add_flag(ui.brushButton, LV_OBJ_FLAG_CLICKABLE);

    ui.refreshButton =  lv_btn_create(ui.toolbarDiv);
    lv_obj_set_size(ui.refreshButton, lv_pct(30), lv_pct(100));
    label = lv_label_create(ui.refreshButton);
    lv_label_set_text(label, "Clear");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_add_flag(ui.refreshButton, LV_OBJ_FLAG_CLICKABLE);

    ui.brushColorwheel = lv_colorwheel_create(ui.root, true);
    lv_obj_set_size(ui.brushColorwheel,  150, 150);
    lv_obj_align_to(ui.brushColorwheel, ui.toolbarDiv, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_obj_add_flag(ui.brushColorwheel, LV_OBJ_FLAG_HIDDEN);

    ui.brushWidthSlider = lv_slider_create(ui.root);
    lv_slider_set_value(ui.brushWidthSlider, 10, LV_ANIM_OFF);
    lv_obj_center(ui.brushWidthSlider);
    lv_obj_add_flag(ui.brushWidthSlider, LV_OBJ_FLAG_HIDDEN);
    lv_slider_set_range(ui.brushWidthSlider, 2, 20);

    ui.canvas = lv_canvas_create(ui.root);
    uint16_t w = amoled.width();
    uint16_t h = amoled.height();
    lv_color_t *buf = (lv_color_t *)ps_malloc(w * h * sizeof(lv_color_t));
    lv_canvas_set_buffer(ui.canvas, buf, w, h, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(ui.canvas, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);
    lv_obj_move_background(ui.canvas);
    lv_obj_add_flag(ui.canvas, LV_OBJ_FLAG_CLICKABLE);

    lv_draw_line_dsc_init(&ui.brush);
    ui.brush.width = 8;
    ui.brush.round_start = true;
    ui.brush.round_end = true;
    ui.brush.color = lv_color_make(255, 0, 0);
    ui.brush.opa = LV_OPA_COVER;

    lv_disp_load_scr(ui.root);

    lv_obj_add_event_cb(ui.brushColorButton, onBrushColorLabelEvent, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.brushButton, onBrushLabelEvent, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.brushWidthSlider, onBrushWidthSliderEvent, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui.brushColorwheel, onBrushColorwheelEvent, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui.refreshButton, onRefreshLabelEvent, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.canvas, onCanvasEvent, LV_EVENT_ALL, NULL);
}