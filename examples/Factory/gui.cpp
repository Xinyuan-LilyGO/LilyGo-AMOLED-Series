/**
 * @file      gui.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-07-13
 *
 */

#include <LilyGo_AMOLED.h>
#include "gui.h"
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>

extern LilyGo_Class amoled;

LV_FONT_DECLARE(alibaba_font_48);
LV_FONT_DECLARE(alibaba_font_18);

LV_FONT_DECLARE(font_ali_70);

LV_IMG_DECLARE(icon_cloudy_sun);
LV_IMG_DECLARE(icon_sun);
LV_IMG_DECLARE(icon_cloudy);
LV_IMG_DECLARE(icon_thunderstorm);
LV_IMG_DECLARE(gif_rabbit);

LV_IMG_DECLARE(icon_battery);
LV_IMG_DECLARE(icon_cpu);
LV_IMG_DECLARE(icon_flash);
LV_IMG_DECLARE(icon_ram);
LV_IMG_DECLARE(icon_light_sensor);
LV_IMG_DECLARE(icon_usb);
LV_IMG_DECLARE(icon_micro_sd);

LV_IMG_DECLARE(ico_ethereum);
LV_IMG_DECLARE(icon_tether);
LV_IMG_DECLARE(icon_xrp);
LV_IMG_DECLARE(icon_bitcoin);

LV_IMG_DECLARE(icon_humidity);
LV_IMG_DECLARE(img_certification_amoled_191_tp);
LV_IMG_DECLARE(img_certification_t4_s3_241_tp);


static lv_obj_t *time_label;
static lv_obj_t *day_label;
static lv_obj_t *week_label;
static lv_obj_t *month_label;
static bool colon;
const char *week_char[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *month_char[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
static lv_obj_t *tileview;
extern Adafruit_NeoPixel *pixels;
// Save the ID of the current page
static uint8_t pageId = 0;


CoinMarketCapApiSubsribe coinSubsribe[] = {
    {1027, &ico_ethereum, "ETH"},
    {1, &icon_bitcoin, "BTC"},
    {825, &icon_tether, "USDT"},
    {52, &icon_xrp, "XPR"},
};

void createPaintUI(lv_obj_t *parent);

static void update_date(lv_timer_t *e)
{
    struct tm timeinfo;
    time_t now;

    if (amoled.hasRTC()) {
        // When a real-time clock chip is present, the hardware time is used
        amoled.getDateTime(&timeinfo);
    } else {
        // When the real-time clock chip does not exist, the ESP32 internal time is used
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    lv_label_set_text_fmt(time_label, "%02d%s%02d", timeinfo.tm_hour, colon != 0 ? "#ffffff :#" : "#000000 :#", timeinfo.tm_min);
    colon = !colon;
    lv_label_set_text_fmt(day_label, "%d", timeinfo.tm_mday);
    lv_label_set_text_fmt(week_label, "%s", week_char[timeinfo.tm_wday]);
    lv_label_set_text_fmt(month_label, "%s", month_char[timeinfo.tm_mon]);
    if (pageId == 3) {
        lv_msg_send(TEMPERATURE_MSG_ID, NULL);
    }
}


void createDisplayBadPixelsTest(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(150));
    lv_obj_set_style_bg_color(cont, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);

    lv_obj_t *colors_obj = lv_obj_create(cont);
    lv_obj_set_size(colors_obj, lv_pct(100), lv_disp_get_ver_res(NULL));
    lv_obj_set_style_bg_color(colors_obj, lv_color_make(255, 0, 0), LV_PART_MAIN);
    lv_obj_align(colors_obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_border_width(colors_obj, 0, 0);
    lv_obj_set_style_pad_all(colors_obj, 0, 0);


    lv_obj_t *btns_cont = lv_obj_create(cont);
    lv_obj_set_width(btns_cont, lv_pct(100));
    lv_obj_set_style_bg_color(btns_cont, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(btns_cont, 0, 0);
    lv_obj_align_to(btns_cont, colors_obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    lv_obj_remove_style(cont, 0, LV_PART_SCROLLBAR);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(colors_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(btns_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(btns_cont, LV_FLEX_FLOW_ROW);

    const char *txt[] = {"R", "G", "B", "W", "B"};
    static lv_color_t test_color[] = {
        lv_color_make(255, 0, 0),
        lv_color_make(0, 255, 0),
        lv_color_make(0, 0, 255),
        lv_color_make(255, 255, 255),
        lv_color_make(0, 0, 0)
    };
    for (int i = 0; i < sizeof(txt) / sizeof(*txt); ++i) {
        lv_obj_t *btn1 = lv_btn_create(btns_cont);
        lv_obj_set_size(btn1, lv_pct(18), 70);
        lv_obj_t *label = lv_label_create(btn1);
        lv_label_set_text(label, txt[i]);
        lv_obj_center(label);
        lv_obj_set_user_data(btn1, colors_obj);

        lv_obj_add_event_cb(btn1, [](lv_event_t *e) {
            lv_obj_t *bg = (lv_obj_t *)lv_obj_get_user_data(lv_event_get_target(e));
            lv_color_t *color = (lv_color_t *)lv_event_get_user_data(e);
            lv_obj_set_style_bg_color(bg, *color, LV_PART_MAIN);
        }, LV_EVENT_CLICKED, (void *)&test_color[i]);

    }
}


void createTimeUI(lv_obj_t *parent)
{
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE);
    // TIME
    lv_obj_t *time_cont = lv_obj_create(parent);
    lv_obj_set_size(time_cont, LV_PCT(70), LV_PCT(100));
    lv_obj_set_style_bg_color(time_cont, lv_color_black(), 0);
    lv_obj_set_style_bg_color(time_cont, lv_color_white(), 0);
    lv_obj_set_scrollbar_mode(time_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(time_cont, LV_DIR_NONE);

    time_label = lv_label_create(time_cont);
    lv_label_set_recolor(time_label, 1);
    lv_label_set_text(time_label, "12:34");
    lv_obj_set_style_text_color(time_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(time_label, &font_ali_70, 0);
    lv_obj_set_style_pad_top(time_label, 15, 0);

    if (lv_disp_get_ver_res(NULL) > 300) {
        lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    } else {
        lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 20);
    }

    week_label = lv_label_create(time_cont);
    lv_obj_set_style_text_color(week_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(week_label, &alibaba_font_18, 0);
    lv_label_set_text(week_label, "Thu");
    lv_obj_align(week_label, LV_ALIGN_BOTTOM_LEFT, 15, -35);

    month_label = lv_label_create(time_cont);
    lv_obj_set_style_text_color(month_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(month_label, &alibaba_font_18, 0);
    lv_label_set_text(month_label, "Feb");
    lv_obj_align_to(month_label, week_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);



    // show cutie
    lv_obj_t *gif = lv_gif_create(time_cont);
    lv_gif_set_src(gif, &gif_rabbit);
    lv_obj_align_to(gif, time_cont, LV_ALIGN_BOTTOM_RIGHT, -10, -10);


    day_label = lv_label_create(time_cont);
    lv_obj_set_style_text_color(day_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(day_label, &alibaba_font_48, 0);
    lv_label_set_text(day_label, "30");
    lv_obj_align_to(day_label, gif, LV_ALIGN_OUT_LEFT_MID, -10, 0);


    // Second cont
    lv_obj_t *second_cont = lv_obj_create(parent);
    lv_obj_set_size(second_cont, LV_PCT(30), LV_PCT(100));
    lv_obj_set_scroll_dir(second_cont, LV_DIR_VER);
    lv_obj_align(second_cont, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_set_style_bg_color(second_cont, lv_color_black(), 0);
    lv_obj_set_flex_flow(second_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(second_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(second_cont, 0, 0);
    lv_obj_set_style_pad_all(second_cont, 0, 0);

    lv_obj_t *img;
    lv_obj_t *label;

    for (int i = 0; i < sizeof(coinSubsribe) / sizeof(coinSubsribe[0]); ++i) {
        lv_obj_t *cont = lv_obj_create(second_cont);
        lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
        lv_obj_set_scroll_dir(cont, LV_DIR_NONE);
        lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
        lv_obj_set_style_border_width(cont, 0, 0);
        lv_obj_set_style_pad_all(cont, 0, 0);

        img = lv_img_create(cont);
        lv_img_set_src(img, coinSubsribe[i].src_img);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, -25);

        label = lv_label_create(cont);
        lv_label_set_text(label, "$0.00");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -50);
        lv_msg_subsribe_obj(COIN_MSG_ID, label, NULL);

        // Addde last obj message cb
        lv_obj_add_event_cb( label, [](lv_event_t *e) {
            lv_obj_t *label = (lv_obj_t *)lv_event_get_target(e);
            lv_msg_t *msg = lv_event_get_msg(e);
            const CoinMarketCapApiDataStream *val = (CoinMarketCapApiDataStream *)lv_msg_get_payload(msg);
            const CoinMarketCapApiSubsribe *target = (CoinMarketCapApiSubsribe *)(e->user_data);
            Serial.print("RECV ID:"); Serial.print(val->id);
            Serial.print("  TARGET ID:"); Serial.println(target->id);
            if (target->id == val->id) {
                lv_label_set_text_fmt(label, "$%.2f", val->price);
                lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -50);
            }
        }, LV_EVENT_MSG_RECEIVED, &coinSubsribe[i]);
    }


    lv_timer_create(update_date, 500, NULL);
}


static void slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    lv_obj_t *slider_label =  (lv_obj_t *)lv_event_get_user_data(e);
    uint8_t level = (uint8_t)lv_slider_get_value(slider);
    int percentage = map(level, 100, 255, 0, 100);
    lv_label_set_text_fmt(slider_label, "%u%%", percentage);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_CENTER, 0, 0);
    amoled.setBrightness(level);
}

void createBrightnessUI(lv_obj_t *parent)
{
    lv_obj_t *label;
    uint8_t b = amoled.getBrightness();
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_size(cont, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) );
    lv_obj_remove_style(cont, 0, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(cont);
    lv_obj_set_style_border_width(cont, 0, 0);

    // MAC
    label = lv_label_create(cont);
    lv_label_set_text_fmt(label, "MAC:%s", WiFi.macAddress().c_str());
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);

    // IPADDRESS
    label = lv_label_create(cont);
    lv_label_set_text(label, "IP:NONE" );
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    // lv_msg_subsribe_obj(WIFI_MSG_ID, label, NULL);

    // Added got ip address message cb
    // lv_obj_add_event_cb( label, [](lv_event_t *e) {
    //     lv_obj_t *label = (lv_obj_t *)lv_event_get_target(e);
    //     Serial.print("-->>IP:"); Serial.println(WiFi.localIP());
    // }, LV_EVENT_MSG_RECEIVED, NULL);

    lv_timer_create([](lv_timer_t *t) {
        lv_obj_t *label = (lv_obj_t *)t->user_data;
        if (WiFi.isConnected()) {
            lv_label_set_text_fmt(label, "IP:%s RSSI:%d",  (WiFi.localIP().toString().c_str()), WiFi.RSSI());
        } else {
            lv_label_set_text(label, "IP:NONE");
        }
    }, 2000, label);


    // Temperature
    label = lv_label_create(cont);
    lv_label_set_text(label, "Temperature:0°C");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_msg_subsribe_obj(TEMPERATURE_MSG_ID, label, NULL);

    // Added temperature change message cb
    lv_obj_add_event_cb( label, [](lv_event_t *e) {
        lv_obj_t *label = (lv_obj_t *)lv_event_get_target(e);
        lv_label_set_text_fmt(label, "Temperature:%.2f°C",  amoled.readCoreTemp());
    }, LV_EVENT_MSG_RECEIVED, NULL);


    //SDCard
    const  BoardsConfigure_t *boards = amoled.getBoardsConfigure();
    if (boards->sd) {
        label = lv_label_create(cont);
        if (SD.cardType() != CARD_NONE) {
            lv_label_set_text_fmt(label, "SDCard: %u MBytes", (uint32_t)(SD.cardSize() / 1024 / 1024));
        } else {
            lv_label_set_text(label, "SDCard: NULL");
        }
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_msg_subsribe_obj(TEMPERATURE_MSG_ID, label, NULL);
    }

    // BRIGHTNESS
    label = lv_label_create(cont);
    lv_label_set_text(label, "Brightness:");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_msg_subsribe_obj(WIFI_MSG_ID, label, NULL);

    /*Create a slider and add the style*/
    lv_obj_t *slider = lv_slider_create(cont);
    lv_obj_set_size(slider, lv_disp_get_physical_hor_res(NULL) - 60, 30);
    lv_slider_set_range(slider, 100, 255);
    lv_slider_set_value(slider, b, LV_ANIM_OFF);

    /*Create a label below the slider*/
    lv_obj_t *slider_label = lv_label_create(slider);
    lv_label_set_text_fmt(slider_label, "%lu%%", map(b, 0, 255, 0, 100));
    lv_obj_set_style_text_color(slider_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_CENTER, 0, 0);



    uint8_t board_id = amoled.getBoardID();
    if (board_id == LILYGO_AMOLED_191_SPI) {
        lv_obj_t *btn_charge = lv_btn_create(parent);
        label = lv_label_create(btn_charge);
        lv_label_set_text(label, "ChargeOFF");
        lv_obj_add_flag(btn_charge, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(btn_charge, [](lv_event_t *e) {
            lv_obj_t *btn = lv_event_get_target(e);
            lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
            lv_state_t  state =  lv_obj_get_state(btn);
            switch (state) {
            case 2 :
                // OFF
                amoled.disableCharge();
                lv_label_set_text(label, "ChargeOFF");
                break;
            case 3:
                // ON
                amoled.enableCharge();
                lv_label_set_text(label, "ChargeON");
                break;
            default:
                break;
            }

        }, LV_EVENT_CLICKED, label);

        lv_obj_align(btn_charge, LV_ALIGN_RIGHT_MID, -20, -10 );
    }

}


static void pixels_event_handler(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    uint8_t *index = (uint8_t *)lv_obj_get_user_data(target);
    if (!index || !pixels)return;
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_VALUE_CHANGED ) {
        switch (*index) {
        case 0:
            pixels->setPixelColor(0, pixels->Color(255, 0, 0)); pixels->show();
            break;
        case 1:
            pixels->setPixelColor(0, pixels->Color(0, 255, 0)); pixels->show();
            break;
        case 2:
            pixels->setPixelColor(0, pixels->Color(0, 0, 255)); pixels->show();
            break;
        case 3: {
            lv_obj_t *cw =  (lv_obj_t *)lv_event_get_user_data(e);
            lv_color_t c =  lv_colorwheel_get_rgb(cw);
            pixels->setPixelColor(0, pixels->Color(c.ch.red, (c.ch.green_h << 3) | c.ch.green_l, c.ch.blue)); pixels->show();
        }
        break;
        case 4: {
            lv_obj_t *slider_label =  (lv_obj_t *)lv_event_get_user_data(e);
            uint8_t level = (uint8_t)lv_slider_get_value(target);
            int percentage = map(level, 0, 255, 0, 100);
            lv_label_set_text_fmt(slider_label, "%u%%", percentage);
            lv_obj_align_to(slider_label, target, LV_ALIGN_CENTER, 0, 0);
            uint8_t val = (uint8_t)lv_slider_get_value(target);
            pixels->setBrightness(val);
            pixels->show();
        }
        break;
        default:
            break;
        }
    }
}

void createPixelsUI(lv_obj_t *parent)
{
    static  lv_style_t style;
    lv_style_set_border_width(&style, 0);
    lv_style_set_bg_color(&style, lv_color_black());


    /*Create a container with ROW flex direction*/
    lv_obj_t *cont_row = lv_obj_create(parent);
    lv_obj_set_size(cont_row, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) - 10);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);

    lv_obj_t *cont_col = lv_obj_create(parent);
    lv_obj_set_size(cont_col, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) - 10);
    lv_obj_align_to(cont_col, cont_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    lv_obj_remove_style(cont_row, 0, LV_PART_SCROLLBAR);
    lv_obj_add_style(cont_row, &style, 0);
    lv_obj_remove_style(cont_col, 0, LV_PART_SCROLLBAR);
    lv_obj_add_style(cont_col, &style, 0);

    lv_obj_t *label;

    static uint8_t index[] = { 0, 1, 2, 3, 4};
    int w = 105, h =  lv_disp_get_ver_res(NULL) - 80;

    lv_obj_t *btn1 = lv_btn_create(cont_row);
    lv_obj_set_user_data(btn1, &index[0]);
    lv_obj_set_size(btn1, w, h);
    lv_obj_set_style_bg_color(btn1, lv_color_make(255, 0, 0), 0);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Red");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &alibaba_font_18, 0);
    lv_obj_add_event_cb(btn1, pixels_event_handler, LV_EVENT_CLICKED, NULL);

    btn1 = lv_btn_create(cont_row);
    lv_obj_set_user_data(btn1, &index[1]);
    lv_obj_set_size(btn1, w, h);
    lv_obj_set_style_bg_color(btn1, lv_color_make(0, 255, 0), 0);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Green");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &alibaba_font_18, 0);
    lv_obj_add_event_cb(btn1, pixels_event_handler, LV_EVENT_CLICKED, NULL);

    btn1 = lv_btn_create(cont_row);
    lv_obj_set_user_data(btn1, &index[2]);
    lv_obj_set_size(btn1, w, h);
    lv_obj_set_style_bg_color(btn1, lv_color_make(0, 0, 255), 0);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Blue");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &alibaba_font_18, 0);
    lv_obj_add_event_cb(btn1, pixels_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cw = lv_colorwheel_create(cont_col, true);
    lv_obj_set_user_data(cw, &index[3]);
    lv_obj_set_size(cw, 100, 100);
    lv_obj_add_event_cb(cw, pixels_event_handler, LV_EVENT_CLICKED, cw);
    lv_obj_align(cw, LV_ALIGN_LEFT_MID, 30, 0);

    /*Create a slider and add the style*/
    lv_obj_t *slider = lv_slider_create(cont_col);
    lv_obj_set_user_data(slider, &index[4]);
    lv_obj_set_size(slider, 135, 30);
    lv_slider_set_range(slider, 0, 255);
    lv_slider_set_value(slider, 255, LV_ANIM_OFF);

    // /*Create a label below the slider*/
    lv_obj_t *slider_label = lv_label_create(slider);
    lv_label_set_text_fmt(slider_label, "%u%%", 100);
    lv_obj_set_style_text_color(slider_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_add_event_cb(slider, pixels_event_handler, LV_EVENT_VALUE_CHANGED, slider_label);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_CENTER, 0, 0);


    lv_obj_align_to(slider, cw, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
}

void createDeviceInfoUI(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_size(cont, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) );
    lv_obj_remove_style(cont, 0, LV_PART_SCROLLBAR);
    lv_obj_center(cont);
    lv_obj_set_style_border_width(cont, 0, 0);

    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_color(&font_style, lv_color_white());
    lv_style_set_text_font(&font_style, &lv_font_montserrat_18);

    // CPU
    lv_obj_t *img_cpu = lv_img_create(cont);
    lv_img_set_src(img_cpu, &icon_cpu);
    lv_obj_align(img_cpu, LV_ALIGN_LEFT_MID, 55, -50);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_add_style(label, &font_style, 0);
    lv_label_set_text(label, ESP.getChipModel());
    lv_obj_align_to(label, img_cpu, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);


    //Flash
    lv_obj_t *img_flash = lv_img_create(cont);
    lv_img_set_src(img_flash, &icon_flash);
    lv_obj_align_to(img_flash, img_cpu, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

    label = lv_label_create(cont);
    lv_obj_add_style(label, &font_style, 0);
    lv_label_set_text_fmt(label, "%uMB", ESP.getFlashChipSize() / 1024 / 1024);
    lv_obj_align_to(label, img_flash, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);


    //PSRAM
    lv_obj_t *img_ram = lv_img_create(cont);
    lv_img_set_src(img_ram, &icon_ram);
    lv_obj_align_to(img_ram, img_flash, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

    float ram_size = abs(ESP.getPsramSize() / 1024.0 / 1024.0);
    label = lv_label_create(cont);
    lv_obj_add_style(label, &font_style, 0);
    lv_label_set_text_fmt(label, "%.1fMB", ram_size);
    lv_obj_align_to(label, img_ram, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    static lv_obj_t *pdat[3];

    //BATTERY
    lv_obj_t *img_battery = lv_img_create(cont);
    lv_img_set_src(img_battery, &icon_battery);
    lv_obj_align_to(img_battery, img_cpu, LV_ALIGN_OUT_BOTTOM_MID, 0, 35);

    label = lv_label_create(cont);
    lv_obj_add_style(label, &font_style, 0);
    lv_label_set_text(label, "N/A");
    lv_obj_align_to(label, img_battery, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    pdat[0] = label;

    //USB
    lv_obj_t *img_usb = lv_img_create(cont);
    lv_img_set_src(img_usb, &icon_usb);
    lv_obj_align_to(img_usb, img_flash, LV_ALIGN_OUT_BOTTOM_MID, 0, 35);

    label = lv_label_create(cont);
    lv_obj_add_style(label, &font_style, 0);
    lv_label_set_text(label, "N/A");
    lv_obj_align_to(label, img_usb, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    pdat[1] = label;

    //SENSOR
    lv_obj_t *img_sensor = lv_img_create(cont);
    lv_img_set_src(img_sensor, &icon_light_sensor);
    lv_obj_align_to(img_sensor, img_ram, LV_ALIGN_OUT_BOTTOM_MID, 0, 35);

    label = lv_label_create(cont);
    lv_obj_add_style(label, &font_style, 0);
    lv_label_set_text(label, "N/A");
    lv_obj_align_to(label, img_sensor, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    pdat[2] = label;


    //SDCard
    const  BoardsConfigure_t *boards = amoled.getBoardsConfigure();
    if (boards->sd) {
        lv_obj_t *img_sd = lv_img_create(cont);
        lv_img_set_src(img_sd, &icon_micro_sd);
        lv_obj_align_to(img_sd, img_ram, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

        label = lv_label_create(cont);
        lv_obj_add_style(label, &font_style, 0);
        if (SD.cardType() != CARD_NONE) {
            lv_label_set_text_fmt(label, "%.2fG", SD.cardSize() / 1024 / 1024 / 1024.0);
        } else {
            lv_label_set_text(label, "N/A");
        }
        lv_obj_align_to(label, img_sd, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    }

    lv_timer_create([](lv_timer_t *t) {

        lv_obj_t **p = (lv_obj_t **)t->user_data;
        uint16_t vol =   amoled.getBattVoltage();
        lv_label_set_text_fmt(p[0], "%u mV", vol);

        const  BoardsConfigure_t *boards = amoled.getBoardsConfigure();
        if (boards->pmu) {
            vol =   amoled.getVbusVoltage();
            lv_label_set_text_fmt(p[1], "%u mV", vol);

            if (boards->sensor) {
                float lux = amoled.getLux();
                lv_label_set_text_fmt(p[2], "%.1f lux", lux);
            }
        }

    }, 1000, pdat);

}

void weather_event_cb(lv_event_t *e)
{
    lv_obj_t *label = (lv_obj_t *)lv_event_get_target(e);
    uint8_t *index = (uint8_t *)lv_event_get_user_data(e);
    lv_msg_t *msg = lv_event_get_msg(e);
    OpenWeatherMapApi *api =  (OpenWeatherMapApi *)lv_msg_get_payload(msg);
    if (!index) {
        Serial.println("Empty index point");
        return;
    }
    if (!api ) {
        Serial.println("Empty api point");
        return;
    }
    switch (*index) {
    case 0://City
        lv_label_set_text(label, api->city.c_str());
        break;
    case 1://Max Min Temp
        lv_label_set_text_fmt(label, "Min/Max:%.1f/%.1f°C", api->temp_min, api->temp_max);
        break;
    case 2: //Humidity
        lv_label_set_text_fmt(label, "Humidity:%.2f%%", api->humidity);
        break;
    case 3: //Pressure
        lv_label_set_text_fmt(label, "Pressure:%.1f Pa", api->pressure);
        break;
    case 4: //Temperature
        lv_label_set_text_fmt(label, "%d°C", (int)(api->temperature));
        break;
    default:
        break;
    }
}


// Need too many icons, don't change icons here
void createWeatherUI(lv_obj_t *parent)
{
    lv_obj_t *img;
    lv_obj_t *label;

    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_bg_color(&cont_style, lv_color_black());
    lv_style_set_border_width(&cont_style, 0);

    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_add_style(cont, &cont_style, 0);
    if (lv_disp_get_ver_res(NULL) > 300) {
        lv_obj_set_size(cont, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) / 2);
    } else {
        lv_obj_set_size(cont, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL));
    }
    lv_obj_remove_style(cont, 0, LV_PART_SCROLLBAR);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    static uint8_t index[6] = {0, 1, 2, 3, 4, 5};
    // WEATHER STRING CONT  WEATHER STRING CONT   WEATHER STRING CONT
    lv_obj_t *weather_string_cont = lv_obj_create(cont);
    lv_obj_add_style(weather_string_cont, &cont_style, 0);
    lv_obj_set_size(weather_string_cont, LV_PCT(60), LV_PCT(100));
    lv_obj_remove_style(weather_string_cont, 0, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(weather_string_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(weather_string_cont, LV_DIR_NONE);

    label = lv_label_create(weather_string_cont);
    lv_label_set_text(label, "ShenZhen");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_msg_subsribe_obj(WEATHER_MSG_ID, label, NULL);
    lv_obj_add_event_cb( label, weather_event_cb, LV_EVENT_MSG_RECEIVED, &index[0]);


    label = lv_label_create(weather_string_cont);
    lv_label_set_text(label, "Min/Max:30/35°C");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_msg_subsribe_obj(WEATHER_MSG_ID, label, NULL);
    lv_obj_add_event_cb( label, weather_event_cb, LV_EVENT_MSG_RECEIVED, &index[1]);


    label = lv_label_create(weather_string_cont);
    lv_label_set_text(label, "Humidity:53%");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_msg_subsribe_obj(WEATHER_MSG_ID, label, NULL);
    lv_obj_add_event_cb( label, weather_event_cb, LV_EVENT_MSG_RECEIVED, &index[2]);


    label = lv_label_create(weather_string_cont);
    lv_label_set_text(label, "Pressure:1000 Pa");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_msg_subsribe_obj(WEATHER_MSG_ID, label, NULL);
    lv_obj_add_event_cb( label, weather_event_cb, LV_EVENT_MSG_RECEIVED, &index[3]);


    // WEATHER ICON CONT  WEATHER ICON CONT  WEATHER ICON CONT
    lv_obj_t *icon_cont = lv_obj_create(cont);
    lv_obj_add_style(icon_cont, &cont_style, 0);
    lv_obj_set_size(icon_cont, LV_PCT(40), LV_PCT(100));
    lv_obj_remove_style(icon_cont, 0, LV_PART_SCROLLBAR);
    lv_obj_set_scroll_dir(icon_cont, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(icon_cont, LV_SCROLLBAR_MODE_OFF);

    img = lv_img_create(icon_cont);
    lv_img_set_src(img, &icon_sun);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 8);

    label = lv_label_create(icon_cont);
    lv_label_set_text_fmt(label, "%d°C", 32);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_msg_subsribe_obj(WEATHER_MSG_ID, label, NULL);
    lv_obj_add_event_cb( label, weather_event_cb, LV_EVENT_MSG_RECEIVED, &index[4]);


    // Free api, so there is no way to get the weather forecast, so the following is fake data
    lv_obj_t *forecast = lv_obj_create(parent);
    lv_obj_set_style_border_width(forecast, 0, 0);
    lv_obj_set_style_bg_color(forecast, lv_color_black(), 0);

    if (lv_disp_get_ver_res(NULL) > 300) {
        lv_obj_set_size(forecast, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) / 2);
    } else {
        lv_obj_set_size(forecast, lv_disp_get_physical_hor_res(NULL), lv_disp_get_ver_res(NULL) - 20);
    }

    lv_obj_remove_style(forecast, 0, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(forecast, LV_FLEX_FLOW_ROW);
    lv_obj_align_to(forecast, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_scrollbar_mode(forecast, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(forecast, LV_DIR_NONE);

    const lv_img_dsc_t *src_img[] = {&icon_thunderstorm, &icon_cloudy, &icon_cloudy_sun};

    for (int i = 0; i < 3; ++i) {
        lv_obj_t *obj = lv_obj_create(forecast);
        lv_obj_set_style_border_width(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_set_size(obj, lv_disp_get_physical_hor_res(NULL) / 3 - 15, lv_disp_get_ver_res(NULL));
        img = lv_img_create(obj);
        lv_img_set_src(img, src_img[i]);
        lv_obj_align( img, LV_ALIGN_TOP_MID, 0, 8);

        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "%d°C", -16);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_align_to(label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }
}

static volatile bool smartConfigStart = false;
static lv_timer_t *wifi_timer = NULL;
static uint32_t wifi_timer_counter = 0;
static uint32_t wifi_connnect_timeout = 60;

static void wifi_config_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        if (smartConfigStart) {
            lv_label_set_text(label, "Config Start");
            if (wifi_timer) {
                lv_timer_del(wifi_timer);
                wifi_timer = NULL;
            }
            WiFi.stopSmartConfig();
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
            Serial.println("return smart Config has Start;");
            smartConfigStart = false;
            return ;
        }
        WiFi.disconnect();
        smartConfigStart = true;
        WiFi.beginSmartConfig();
        lv_label_set_text(label, "Config Stop");
        lv_obj_add_state(btn, LV_STATE_CHECKED);

        wifi_timer =  lv_timer_create([](lv_timer_t *t) {
            lv_obj_t *btn = (lv_obj_t *)t->user_data;
            lv_obj_t *label = lv_obj_get_child(btn, 0);
            bool destory = false;
            wifi_timer_counter++;
            if (wifi_timer_counter > wifi_connnect_timeout && !WiFi.isConnected()) {
                Serial.println("Connect timeout!");
                destory = true;
                lv_label_set_text(label, "Time Out");
            }
            if (WiFi.isConnected() ) {
                Serial.println("WiFi has connected!");
                Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
                Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
                destory = true;
                String IP = WiFi.localIP().toString();
                lv_label_set_text(label, IP.c_str());
            }
            if (destory) {
                WiFi.stopSmartConfig();
                smartConfigStart = false;
                lv_timer_del(wifi_timer);
                wifi_timer = NULL;
                wifi_timer_counter = 0;
                lv_obj_clear_state(btn, LV_STATE_CHECKED);
            }
            // Every seconds check conected
        }, 1000, btn);
    }
}

void createWiFiConfigUI(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);

    if (lv_disp_get_ver_res(NULL) > 300) {
        lv_obj_set_size(cont, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    } else {
        lv_obj_set_size(cont, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) * 2 + 20);
    }

    lv_obj_remove_style(cont, 0, LV_PART_SCROLLBAR);
    lv_obj_set_style_border_width(cont, 0, 0);

    String txt = "Use ESP Touch App Configure your network\n";
    txt += "1. Install ESPTouch App\n";
    txt += "2. Turn on ESPTouch -> Click EspTouch\n";
    txt += "3. Enter Your WiFi Password,\n\tOther setting use default\n";
    txt += "4. Confirm\n";
    txt += "5. Click Config WiFi Button\n";
    txt += "6. Wait config done\n";


    lv_obj_t *label, *tips_label ;
    tips_label = lv_label_create(cont);
    lv_obj_set_width(tips_label, LV_PCT(100));
    lv_label_set_long_mode(tips_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_color(tips_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(tips_label, txt.c_str());
    lv_obj_align(tips_label, LV_ALIGN_TOP_MID, 0, 20);

    const char *android_url = "https://github.com/EspressifApp/EsptouchForAndroid/releases/download/v2.3.2/esptouch-v2.3.2.apk";
    const char *ios_url = "https://apps.apple.com/cn/app/espressif-esptouch/id1071176700";


    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_GREEN, 5);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_NONE, 4);

    lv_coord_t size = 120;
    lv_obj_t *android_rq_code = lv_qrcode_create(cont, size, fg_color, bg_color);
    lv_qrcode_update(android_rq_code, android_url, strlen(android_url));
    lv_obj_align_to(android_rq_code, tips_label, LV_ALIGN_OUT_BOTTOM_LEFT, 30, 5);
    /*Add a border with bg_color*/
    lv_obj_set_style_border_color(android_rq_code, bg_color, 0);
    lv_obj_set_style_border_width(android_rq_code, 5, 0);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Android" );
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(label, android_rq_code, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t *ios_rq_code = lv_qrcode_create(cont, size, fg_color, bg_color);
    lv_qrcode_update(ios_rq_code, ios_url, strlen(ios_url));
    lv_obj_align_to(ios_rq_code, android_rq_code, LV_ALIGN_OUT_RIGHT_MID, 40, -5);

    /*Add a border with bg_color*/
    lv_obj_set_style_border_color(ios_rq_code, bg_color, 0);
    lv_obj_set_style_border_width(ios_rq_code, 5, 0);
    label = lv_label_create(cont);
    lv_label_set_text(label, "IOS" );
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(label, ios_rq_code, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


    //BTN
    lv_obj_t *btn =  lv_btn_create(cont);
    label = lv_label_create(btn);
    lv_label_set_text(label, "Config WiFi");
    lv_obj_set_width(btn, 300);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(label, btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, wifi_config_event_handler, LV_EVENT_CLICKED, label);
    lv_obj_center(btn);

    if (lv_disp_get_ver_res(NULL) > 300) {
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    } else {
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    }

}



void tileview_change_cb(lv_event_t *e)
{
    lv_obj_t *tileview = lv_event_get_target(e);
    pageId = lv_obj_get_index(lv_tileview_get_tile_act(tileview));
    lv_event_code_t c = lv_event_get_code(e);
    Serial.print("Code : ");
    Serial.print(c);
    uint32_t count =  lv_obj_get_child_cnt(tileview);
    Serial.print(" Count:");
    Serial.print(count);
    Serial.print(" pageId:");
    Serial.println(pageId);

}


void showCertification(uint32_t delay_ms)
{
    uint8_t board = amoled.getBoardID();

    lv_obj_t *img =  lv_img_create(lv_scr_act());

    if (board == LILYGO_AMOLED_191 && amoled.hasTouch()) {
        lv_img_set_src(img, &img_certification_amoled_191_tp);
    } else if (board == LILYGO_AMOLED_241) {
        lv_img_set_src(img, &img_certification_t4_s3_241_tp);
    } else {
        lv_obj_del(img); return;
    }
    lv_obj_center(img);

    uint32_t start_ms = millis();
    while ((millis() - start_ms) < delay_ms) {
        lv_timer_handler();
        delay(2);
    }
    lv_obj_del(img);
}

uint16_t max_item_num = 6;

void factoryGUI(void)
{
    static lv_style_t bgStyle;
    lv_style_init(&bgStyle);
    lv_style_set_bg_color(&bgStyle, lv_color_black());

    tileview = lv_tileview_create(lv_scr_act());
    lv_obj_add_style(tileview, &bgStyle, LV_PART_MAIN);
    lv_obj_set_size(tileview, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_add_event_cb(tileview, tileview_change_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *t1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t4 = lv_tileview_add_tile(tileview, 3, 0,  LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t5 = lv_tileview_add_tile(tileview, 4, 0,  LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t6 = lv_tileview_add_tile(tileview, 5, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t7 = lv_tileview_add_tile(tileview, 6, 0, LV_DIR_HOR | LV_DIR_BOTTOM);

    createTimeUI(t1);
    createWeatherUI(t2);
    createDeviceInfoUI(t3);
    createBrightnessUI(t4);

    const  BoardsConfigure_t *boards = amoled.getBoardsConfigure();
    if (boards->pixelsPins != -1) {
        createPixelsUI(t5);
        createWiFiConfigUI(t6);
        createDisplayBadPixelsTest(t7);
        lv_obj_t *t8 = lv_tileview_add_tile(tileview, 7, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
        createPaintUI(t8);
        max_item_num = 8;
    } else {
        createWiFiConfigUI(t5);
        createDisplayBadPixelsTest(t6);
        createPaintUI(t7);
        max_item_num = 7;
        // lv_obj_del(t7);
    }

}


void selectNextItem()
{
    static int id = 0;
    id++;
    id %= max_item_num;
    lv_obj_set_tile_id(tileview, id, 0, LV_ANIM_ON);
}


/////////////////////////////////////////////////////\

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

void createPaintUI(lv_obj_t *parent)
{
    ui.root = (parent);
    lv_obj_set_size(ui.root, lv_pct(100), lv_pct(100));

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


    lv_obj_add_event_cb(ui.brushColorButton, onBrushColorLabelEvent, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.brushButton, onBrushLabelEvent, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.brushWidthSlider, onBrushWidthSliderEvent, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui.brushColorwheel, onBrushColorwheelEvent, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui.refreshButton, onRefreshLabelEvent, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui.canvas, onCanvasEvent, LV_EVENT_ALL, NULL);
}