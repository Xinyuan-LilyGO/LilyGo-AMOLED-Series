/**
 * @file      USB_Host_Keyboard_Mouse.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-11-26
 * @note
 * * 1. After the sketch is uploaded, the ESP32S3 port will disappear and enter the host mode.
 * *    At this time, you need to connect the mouse through USB-C to USB-A,
 * *    and then press the BOOT button to turn on the OTG power supply state.
 * *    When the mouse is detected, the screen mouse will become operational.
 * * 2. When the program needs to be re-burned, T-Display-AMOLED must be manually
 * *    entered into the download mode so that the port can be generated correctly
 * *    and the sketch can be uploaded.
 * *     ** Steps to manually enter download mode **
 * *        - Connect the board via the USB cable
 * *        - Press and hold the BOOT button , While still pressing the BOOT button, press RST
 * *        - Release the RST
 * *        - Release the BOOT button
 * *        - Upload sketch
 * * 3. Only T4-S3 / AMOLED-1.91-PLUS has OTG function , 
 * *    Other devices require additional power supply for keyboard and mouse
 * *
 * * Youtube : https://youtu.be/cq4Wg4ffzKM
  */

#include <AceButton.h>
#include "usb_hid.h"
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

using namespace ace_button;
AceButton button;

// Mouse pointer image
LV_IMG_DECLARE(image_emoji);

LilyGo_Class amoled;

// Input device parameters
struct InputParams inputParams;


// The event handler for the button.
void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    // Print out a message for all events.
    Serial.print(F("handleEvent(): eventType: "));
    Serial.print(AceButton::eventName(eventType));
    Serial.print(F("; buttonState: "));
    Serial.println(buttonState);

    uint8_t id = button->getId();
    if (eventType != AceButton::kEventClicked) {
        return;
    }

    // Only T4-S3 / AMOLED-1.91-PLUS has OTG function
    if (amoled.hasOTG()) {
        if (amoled.isEnableOTG()) {
            amoled.disableOTG();
            Serial.println("disableOTG");
        } else {
            amoled.enableOTG();
            amoled.enableCharge();
            Serial.println("enableOTG");
        }
    }
}



void setup()
{
    Serial.begin(115200);

    Serial.println("USB_Host_Keyboard_Mouse example");


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


    // initialize the corresponding AceButton
    uint8_t pin = 0;    //BOOT Pin
    pinMode(pin, INPUT_PULLUP);
    button.init(pin, HIGH);
    // Configure the ButtonConfig with the event handler, and enable all higher
    // level events.
    ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);

    // Register lvgl helper
    beginLvglHelper(amoled);

    // Register USB input device
    inputParams.queue = xQueueCreate( 10, sizeof( struct InputData  ) );    //Creating an Input Queue
    inputParams.icon = (const void *)&image_emoji;  //Set mouse pointer icon
    beginLvglInputDevice(inputParams);  //Register lvgl to allow input device input


    setupUSB(inputParams.queue);    // Initialize USB Host

    // Creating an Input Box
    lv_obj_t *radio_ta = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(radio_ta, LV_PCT(80), LV_PCT(20));
    lv_textarea_set_text(radio_ta, "");
    lv_textarea_set_max_length(radio_ta, 1024);
    lv_obj_align(radio_ta, LV_ALIGN_TOP_MID, 0, 20);


    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
                      "and wrap long text automatically.");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(label2, 150);
    lv_label_set_text(label2, "It is a circularly scrolling text. ");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 20);



    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, -80, -40);
    lv_obj_t *label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 80, -40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);
    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);

}


void loop()
{

    button.check();
    lv_task_handler();
    delay(2);
}






