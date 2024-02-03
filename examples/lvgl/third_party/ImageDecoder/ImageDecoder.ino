/**
 * @file      ImageDecoder.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-05-01
 * @Note      You need to upload the files from the data file to the SPIFFS
 *            file system before using this example
 * @note      platfromio user use <pio run -t uploadfs> ,
 *                  Note that you need to place the sample data folder in the same level directory as <platformio.ini>
 *                  If using 2.41 inch, please copy all the pictures in the data folder to the SD card
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

#if !LV_USE_PNG || !LV_USE_BMP || !LV_USE_SJPG
#error "lvgl png , bmp , sjpg decoder library is not enable!"
#endif

const char *filename[] = {
    "logo.bmp", "logoColor1.png", "logoColor2.png", "ttgo.jpg",
    "product0.png", "product1.png", "product2.png", "product3.png",
    "product4.png", "product5.png"
};

LilyGo_Class amoled;

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

    beginLvglHelper(amoled);

    String file =  String("/") + filename[0];


    uint8_t id = amoled.getBoardID();

    if (id == LILYGO_AMOLED_147 || id == LILYGO_AMOLED_191) {
        if (!SPIFFS.exists(file)) {
            while (1) {
                Serial.println("You need to upload the files from the data file to the SPIFFS file system before using this example");
                delay(1000);
            }
        }
    } else if (id == LILYGO_AMOLED_241) {
        if (!SD.exists(file)) {
            while (1) {
                Serial.println("You need to upload the files from the data file to the SD file system before using this example");
                delay(1000);
            }
        }
    } else {
        while (1) {
            Serial.println("Unkonw board!");
            delay(1000);
        }
    }


    lv_obj_t *img =  lv_img_create(lv_scr_act());
    lv_obj_center(img);

    lv_timer_create([](lv_timer_t *t) {
        static int i = 0;
        String fspath = LV_FS_POSIX_LETTER + String(":/") + filename[i];
        lv_img_set_src((lv_obj_t *)t->user_data, fspath.c_str());
        i++;
        i %= sizeof(filename) / sizeof(filename[0]);
    }, 1000, img);

}

void loop()
{
    lv_task_handler();
    delay(5);
}

