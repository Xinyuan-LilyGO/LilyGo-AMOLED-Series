/**
 * @file      Lvgl_Images.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-01-18
 * @note      imageconverter - https://lvgl.io/tools/imageconverter
 */
#include <LilyGo_AMOLED.h>      //To use LilyGo AMOLED series screens, please include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <AceButton.h>
using namespace ace_button;

LilyGo_Class amoled;
static uint8_t btnPin = 0;
AceButton button(btnPin);

static uint8_t image_index = 1;
static lv_obj_t *img1;
static int16_t x, y;


// Image resources need to define the screen size
// #define USE_AMOLED_147    //1.47 inches 194x368
// #define USE_AMOLED_191    //1.91 inches 240x536
#define USE_AMOLED_241    //2.41 inches 450x600

#if defined(USE_AMOLED_147)
LV_IMG_DECLARE(image_1_368x194);
LV_IMG_DECLARE(image_2_368x194);
LV_IMG_DECLARE(image_3_368x194);
LV_IMG_DECLARE(image_4_368x194);
LV_IMG_DECLARE(image_5_368x194);
LV_IMG_DECLARE(image_6_368x194);
LV_IMG_DECLARE(image_7_368x194);
#elif defined(USE_AMOLED_241)
LV_IMG_DECLARE(image_1_600x450);
LV_IMG_DECLARE(image_2_600x450);
LV_IMG_DECLARE(image_3_600x450);
LV_IMG_DECLARE(image_4_600x450);
LV_IMG_DECLARE(image_5_600x450);
LV_IMG_DECLARE(image_6_600x450);
LV_IMG_DECLARE(image_7_600x450);
#elif defined(USE_AMOLED_191)
LV_IMG_DECLARE(image_1_536x240);
LV_IMG_DECLARE(image_2_536x240);
LV_IMG_DECLARE(image_3_536x240);
LV_IMG_DECLARE(image_4_536x240);
LV_IMG_DECLARE(image_5_536x240);
LV_IMG_DECLARE(image_6_536x240);
LV_IMG_DECLARE(image_7_536x240);
#else
#error "Image resources need to define the screen size"
#endif


const void *images[] = {
#if defined(USE_AMOLED_147)
    &image_1_368x194,
    &image_2_368x194,
    &image_3_368x194,
    &image_4_368x194,
    &image_5_368x194,
    &image_6_368x194,
    &image_7_368x194,
#elif defined(USE_AMOLED_241)
    &image_1_600x450,
    &image_2_600x450,
    &image_3_600x450,
    
    // Cannot customize partitions on ArduinoIDE, the maximum size is 3M APP
    // Platformio does not have this limitation
    // &image_4_600x450,
    // &image_5_600x450,
    // &image_6_600x450,
    // &image_7_600x450,
#elif defined(USE_AMOLED_191)
    &image_1_536x240,
    &image_2_536x240,
    &image_3_536x240,
    &image_4_536x240,
    &image_5_536x240,
    &image_6_536x240,
    &image_7_536x240,
#endif
};

void updateImages()
{
    lv_img_set_src(img1, images[image_index]);
    image_index++;
    image_index %= sizeof(images) / sizeof(images[0]);
}

void handleEvent(AceButton * /* button */, uint8_t eventType,
                 uint8_t /* buttonState */)
{
    switch (eventType) {
    case AceButton::kEventPressed:
        updateImages();
        break;
    default: break;
    }
}


void images_event_cb(lv_event_t *e)
{
    static uint8_t image_index = 1;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)return;
    lv_img_set_src(lv_event_get_target(e), images[image_index]);
    image_index++;
    image_index %= sizeof(images) / sizeof(images[0]);
}

void setup()
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

    beginLvglHelper(amoled);


    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, images[0]);
    lv_obj_center(img1);
    lv_obj_add_event_cb(img1, images_event_cb, LV_EVENT_CLICKED, NULL);

    // Home Button Only 1.91 Inch AMOLED board support ,other board not support
    amoled.setHomeButtonCallback([](void *ptr) {
        static uint32_t checkMs = 0;
        if (millis() > checkMs) {
            updateImages();
        }
        checkMs = millis() + 300;

    }, NULL);

    amoled.setBrightness(255);

    //Initial BOOT button, used as setting direction trigger
    pinMode(btnPin, INPUT_PULLUP);
    button.setEventHandler(handleEvent);
}


void loop()
{
    amoled.getPoint(&x, &y);
    lv_task_handler();
    button.check();
}
