/**
 * @file      LilyGo_Button.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-10-21
 *
 */
#include "LilyGo_Button.h"

void LilyGo_Button::init(uint32_t gpio, uint32_t debounceTimeout )
{
    this->gpio = gpio;
    setDebounceTime(debounceTimeout);
}


void LilyGo_Button::setDebounceTime(uint32_t ms)
{
    debounce_time_ms = ms;
}

void LilyGo_Button::setEventCallback(event_callback func)
{
    event_cb = func;
}

uint32_t LilyGo_Button::wasPressedFor()
{
    return down_time_ms;
}

uint32_t LilyGo_Button::getNumberOfClicks()
{
    return click_count;
}


uint32_t LilyGo_Button::getClickType()
{
    return last_click_type;
}


void LilyGo_Button::update()
{
    prev_state = curr_state;

    curr_state = touchInterruptGetLastStatus(gpio) == 0;

    if (prev_state == HIGH && curr_state == LOW) {
        down_ms = millis();
        pressed_triggered = false;
        click_count++;
        click_ms = down_ms;
    } else if (prev_state == LOW && curr_state == HIGH) {
        down_time_ms = millis() - down_ms;
        if (down_time_ms >= debounce_time_ms) {
            if (event_cb) {
                this->event_cb(BTN_RELEASED_EVENT);
            }
            if (down_time_ms >= LONGCLICK_MS) {
                longclick_detected = true;
            }
        }
    } else if (curr_state == LOW && !pressed_triggered && (millis() - down_ms >= debounce_time_ms && !long_pressed_detected)) {
        long_down_ms = millis();
        if (event_cb) {
            this->event_cb(BTN_PRESSED_EVENT);
        }
        pressed_triggered = true;
    } else if (pressed_triggered && ((millis() - long_down_ms) > LONGPRESS_MS) && !long_pressed_detected) {
        if (event_cb) {
            this->event_cb(BTN_LONG_PRESSED_EVENT);
        }
        pressed_triggered = false;
        long_pressed_detected = true;
    } else if (curr_state == HIGH && millis() - click_ms > DOUBLECLICK_MS) {
        if (long_pressed_detected) {
            long_pressed_detected = false;
            last_click_type = LONG_PRESS;
            click_count = 0;
        }
        if (click_count > 0) {
            long_down_ms = 0;
            pressed_triggered = false;
            switch (click_count) {
            case 1:
                last_click_type = SINGLE_CLICK;
                if (event_cb) {
                    this->event_cb(BTN_CLICK_EVENT);
                }
                break;
            case 2:
                last_click_type = DOUBLE_CLICK;
                if (event_cb) {
                    this->event_cb(BTN_DOUBLE_CLICK_EVENT);
                }
                break;
            case 3:
                last_click_type = TRIPLE_CLICK;
                if (event_cb) {
                    this->event_cb(BTN_TRIPLE_CLICK_EVENT);
                }
                break;
            }
        }
        click_count = 0;
        click_ms = 0;
    }
}
