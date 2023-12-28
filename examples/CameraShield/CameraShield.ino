/**
 * @file      CameraShield.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-12-28
 * @note      Sketch Adaptation AMOLED Camera Shield , Only suitable for AMOLED 1.91 inches
 */
#include <LilyGo_AMOLED.h>      //To use LilyGo AMOLED series screens, please include <LilyGo_AMOLED.h>
#include "esp_camera.h"
#include "title.h"

#define CAMERA_PIN_PWDN     (-1)
#define CAMERA_PIN_RESET    (-1)
#define CAMERA_PIN_XCLK     (15)
#define CAMERA_PIN_SIOD     (42)
#define CAMERA_PIN_SIOC     (41)
#define CAMERA_PIN_D7       (16)
#define CAMERA_PIN_D6       (14)
#define CAMERA_PIN_D5       (13)
#define CAMERA_PIN_D4       (11)
#define CAMERA_PIN_D3       (1)
#define CAMERA_PIN_D2       (45)
#define CAMERA_PIN_D1       (46)
#define CAMERA_PIN_D0       (10)
#define CAMERA_PIN_VSYNC    (40)
#define CAMERA_PIN_HREF     (39)
#define CAMERA_PIN_PCLK     (12)

#define XCLK_FREQ_HZ        15000000

LilyGo_Class amoled;


void initCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sccb_sda = CAMERA_PIN_SIOD;
    config.pin_sccb_scl = CAMERA_PIN_SIOC;
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_240X240;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();

    if (s->id.PID == OV2640_PID) {
        s->set_hmirror(s, 0);
    } else {
        s->set_hmirror(s, 1);
        s->set_vflip(s, 1); // flip it back
    }

    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 1);  // up the blightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }
    s->set_sharpness(s, 2);
    s->set_awb_gain(s, 2);
}

void setup()
{
    Serial.begin(115200);

    initCamera();

    // Automatically determine the access device , Only suitable for AMOLED 1.91 inches
    if (!amoled.begin()) {
        while (1) {
            Serial.println("There is a problem with the device!~"); delay(1000);
        }
    }
    amoled.pushColors(0, 240, 240, 296, (uint16_t *)gImage_title);
}

void loop()
{
    camera_fb_t *frame = esp_camera_fb_get();
    if (frame) {
        amoled.setAddrWindow(0, 0, 239, 239);
        amoled.pushColors((uint16_t *)frame->buf, frame->len / 2);
        esp_camera_fb_return(frame);
    }
    delay(5);
}



















