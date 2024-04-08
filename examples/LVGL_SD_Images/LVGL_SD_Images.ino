/**
 * @file      LVGL_SD_Images.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-04-08
 * @note      The example demonstrates how to use pictures stored in the SD card for display. For boards without SD card slots, an external SD card module needs to be connected.
 *
 * Connect the SD card to the following pins:
 * | SD Card | 1.47 Inch | 1.91 Inch | 2.41    |
 * | ------- | --------- | --------- | ------- |
 * | MISO    | 47        | 15        | onboard |
 * | MOSI    | 39        | 14        | onboard |
 * | SCK     | 39        | 13        | onboard |
 * | CS      | 9         | 12        | onboard |
 */
#include <LilyGo_AMOLED.h>      //To use LilyGo AMOLED series screens, please include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <AceButton.h>
#include <vector>


using namespace ace_button;

LilyGo_Class amoled;
static uint8_t btnPin = 0;
AceButton button(btnPin);

static uint8_t image_index = 1;
static lv_obj_t *img1 = NULL;
static int16_t x, y;
std::vector<String> images;
static bool mountSD = false;


void updateImages()
{
    if (!img1) {
        return;
    }
    lv_img_set_src(img1, images[image_index].c_str());
    image_index++;
    image_index %= images.size();
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


void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory() && levels) {
            Serial.print("  DIR : ");
            Serial.print(file.name());
            Serial.println();

            listDir(fs, file.path(), levels - 1);
        } else {
            String filename = String(file.name());
            if (filename.endsWith(".jpeg") || filename.endsWith(".jpg") || filename.endsWith(".png")) {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("  SIZE: ");
                Serial.println(file.size());

                // Save full image path
                String fullPath;

                fullPath = String(dirname) + "/" + filename;

                // Remove redundant "/"
                if (fullPath.startsWith("//")) {
                    fullPath = fullPath.substring(1);
                }

                // Add drive letter for lvgl index image
                fullPath = "A:" + fullPath;

                images.push_back(fullPath);
            }
        }
        file = root.openNextFile();
    }
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

    // Tried to initialize three times
    int retry = 3;
    while (retry--) {
        mountSD =  amoled.installSD();
        if (mountSD) {
            break;
        }
    }

    if (mountSD) {
        Serial.println("SD card installed successfully");
    } else {
        Serial.println("SD card installed failed");
        lv_obj_t *label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "SD card installed failed");
        lv_obj_center(label);
        amoled.setBrightness(255);
        return;
    }

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

    // Traverse files in SD card
    listDir(SD, "/", 5); // Change the root directory as needed

    Serial.print("\n\n\nTotal images found: ");
    Serial.println(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        Serial.print("Image ");
        Serial.print(i + 1);
        Serial.print(" path: ");
        Serial.println(images[i]);
    }

    if (images.size()) {
        img1 = lv_img_create(lv_scr_act());
        lv_img_set_src(img1, images[0].c_str());
        lv_obj_center(img1);
    } else {
        lv_obj_t *label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "Total images found: 0");
        lv_obj_center(label);
    }
}


void loop()
{
    amoled.getPoint(&x, &y);
    lv_task_handler();
    button.check();
}
