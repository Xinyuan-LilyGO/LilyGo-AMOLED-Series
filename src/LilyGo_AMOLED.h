/**
 * @file      LilyGo_AMOLED.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-05-29
 *
 */

#pragma once
#include <Arduino.h>

#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(2,0,5)
#error "Please update the esp32 core version to a version greater than 2.0.5 and less than v3.0.0, how to update, please see here https://docs.espressif.com/projects/arduino-esp32/en/latest/"
#endif

#if ESP_ARDUINO_VERSION > ESP_ARDUINO_VERSION_VAL(3,0,0)
// #error "Please update the esp32 core version to a version greater than 2.0.5 and less than v3.0.0, how to update, please see here https://docs.espressif.com/projects/arduino-esp32/en/latest/"
#endif

#include <driver/spi_master.h>
#include <SPI.h>
#include "XPowersLib.h"
#include "initSequence.h"
#include "TouchDrvCHSC5816.hpp"
#include "TouchDrvCSTXXX.hpp"
#include "SensorCM32181.hpp"
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>
#include <sys/cdefs.h>
#include "LilyGo_Display.h"
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
#include <driver/temp_sensor.h>
#else
#include <driver/temperature_sensor.h>
#endif

#include "SensorPCF85063.hpp"
#include "PowersBQ25896.tpp"

#if ARDUINO_USB_CDC_ON_BOOT != 1
#warning "If you need to monitor printed data, be sure to set USB_CDC_ON_BOOT to ENABLE, otherwise you will not see any data in the serial monitor"
#endif

#ifndef BOARD_HAS_PSRAM
#error "Detected that PSRAM is not turned on. Please set PSRAM to OPI PSRAM in ArduinoIDE"
#endif //BOARD_HAS_PSRAM

#ifndef SENSORLIB_VERSION_MAJOR
#error "Please search SensorLib through the arduino ide library manager and install version v0.2.4"
#endif


#define BOARD_NONE_PIN      (-1)
#define BOARD_PIXELS_PIN    (18)        //only 1.47 inch
#define BOARD_PIXELS_NUM    (1)
#define DEFAULT_SCK_SPEED   (30 * 1000 * 1000)

typedef struct __DisplayConfigure {
    int d0;
    int d1;
    int d2;
    int d3;
    int sck;
    int cs;
    int dc;
    int rst;
    int te;
    uint8_t cmdBit;
    uint8_t addBit;
    int  freq;
    lcd_cmd_t *initSequence;
    uint32_t initSize;
    uint16_t width;
    uint16_t height;
    uint32_t frameBufferSize;
    bool fullRefresh;
} DisplayConfigure_t;

typedef struct __BoardTouchPins {
    int sda;
    int scl;
    int irq;
    int rst;
} BoardTouchPins_t;

typedef struct __BoardPmuPins {
    int sda;
    int scl;
    int irq;
} BoardPmuPins_t;

typedef struct __BoardSensorPins {
    int sda;
    int scl;
    int irq;
} BoardSensorPins_t;

typedef struct __BoardSDCardPins {
    int miso;
    int mosi;
    int sck;
    int cs;
} BoardSDCardPins_t;

typedef struct __BoardsConfigure {
    DisplayConfigure_t display;
    const BoardTouchPins_t *touch;
    const BoardPmuPins_t *pmu;
    const BoardSensorPins_t *sensor;
    const BoardSDCardPins_t *sd;
    const int *pButtons;
    const int buttonNum;
    int pixelsPins;
    int adcPins;
    int PMICEnPins;
    bool framebuffer;
} BoardsConfigure_t;


// LILYGO 1.47 Inch AMOLED(SH8501) S3R8
// https://www.lilygo.cc/products/t-display-amoled
static const DisplayConfigure_t SH8501_AMOLED  = {
    7, //BOARD_DISP_DATA0,
    10,//BOARD_DISP_DATA1,
    11,//BOARD_DISP_DATA2,
    12,//BOARD_DISP_DATA3,
    5,//BOARD_DISP_SCK,
    4,//BOARD_DISP_CS,
    BOARD_NONE_PIN,//DC
    40,//BOARD_DISP_RESET,
    6,//BOARD_DISP_TE,
    8,//command bit
    24,//address bit
    30000000,
    (lcd_cmd_t *)sh8501_cmd,
    SH8501_INIT_SEQUENCE_LENGTH,
    SH8501_WIDTH, //width
    SH8501_HEIGHT, //height
    SH8501_WIDTH *SH8501_HEIGHT * sizeof(uint16_t), //frameBufferSize
    true //fullRefresh
};

static const int AMOLED_147_BUTTONTS[2] = {0, 21};
static const BoardTouchPins_t AMOLED_147_TOUCH_PINS = {1/*SDA*/, 2/*SCL*/, 13/*IRQ*/, 14/*RST*/};
static const BoardPmuPins_t AMOLED_147_PMU_PINS =  {1/*SDA*/, 2/*SCL*/, 3/*IRQ*/};
static const BoardSensorPins_t AMOLED_147_SENSOR_PINS =  {1/*SDA*/, 2/*SCL*/, 8/*IRQ*/};

static const int AMOLED_191_BUTTONTS[1] = {0};
static const BoardTouchPins_t AMOLED_191_TOUCH_PINS = {3 /*SDA*/, 2 /*SCL*/, 21/*IRQ*/, -1/*RST*/};
static const BoardSDCardPins_t AMOLED_191_SPI_SD_PINS =  {13/*MISO*/, 12/*MOSI*/, 14/*SCK*/, 11/*CS*/};
static const BoardPmuPins_t AMOLED_191_SPI_PMU_PINS =  {3/*SDA*/, 2/*SCL*/, 1/*IRQ*/};

// LILYGO 1.91 Inch AMOLED(RM67162) S3R8
// https://www.lilygo.cc/products/t-display-s3-amoled
static const DisplayConfigure_t RM67162_AMOLED  = {
    18,//BOARD_DISP_DATA0,
    7,//BOARD_DISP_DATA1,
    48,//BOARD_DISP_DATA2,
    5,//BOARD_DISP_DATA3,
    47,//BOARD_DISP_SCK,
    6,//BOARD_DISP_CS,
    BOARD_NONE_PIN,//DC
    17,//BOARD_DISP_RESET,
    9, //BOARD_DISP_TE,
    8, //command bit
    24,//address bit
    75000000,
    (lcd_cmd_t *)rm67162_cmd,
    RM67162_INIT_SEQUENCE_LENGTH,
    RM67162_WIDTH,//width
    RM67162_HEIGHT,//height
    0,//frameBufferSize
    false //fullRefresh
};

// LILYGO 1.91 Inch AMOLED(RM67162) S3R8
// https://www.lilygo.cc/products/t-display-s3-amoled
static const DisplayConfigure_t RM67162_AMOLED_SPI  = {
    18,//BOARD_DISP_DATA0,          //MOSI
    7,//BOARD_DISP_DATA1,           //DC
    -1,//BOARD_DISP_DATA2,
    -1,//BOARD_DISP_DATA3,
    47,//BOARD_DISP_SCK,            //SCK
    6,//BOARD_DISP_CS,              //CS
    BOARD_NONE_PIN,//DC
    17,//BOARD_DISP_RESET,          //RST
    9, //BOARD_DISP_TE,
    8, //command bit
    24,//address bit
    40000000,
    (lcd_cmd_t *)rm67162_spi_cmd,
    RM67162_INIT_SPI_SEQUENCE_LENGTH,
    RM67162_WIDTH,//width
    RM67162_HEIGHT,//height
    0,//frameBufferSize
    false //fullRefresh
};


// LILYGO 2.41 Inch AMOLED(RM690B0) S3R8
// https://www.lilygo.cc/products/t4-s3
static const DisplayConfigure_t RM690B0_AMOLED  = {
    14,//BOARD_DISP_DATA0,
    10,//BOARD_DISP_DATA1,
    16,//BOARD_DISP_DATA2,
    12,//BOARD_DISP_DATA3,
    15,//BOARD_DISP_SCK,
    11,//BOARD_DISP_CS,
    BOARD_NONE_PIN,//DC
    13,//BOARD_DISP_RESET,
    18, //BOARD_DISP_TE,
    8, //command bit
    24,//address bit
    36000000,
    (lcd_cmd_t *)rm690b0_cmd,
    RM690B0_INIT_SEQUENCE_LENGTH,
    RM690B0_WIDTH,//width
    RM690B0_HEIGHT,//height
    0,//frameBufferSize
    false //fullRefresh
};
static const int AMOLED_241_BUTTONTS[1] = {0};
static const BoardPmuPins_t AMOLED_241_PMU_PINS =  {6/*SDA*/, 7/*SCL*/, 5/*IRQ*/};
static const BoardTouchPins_t AMOLED_241_TOUCH_PINS =  {6/*SDA*/, 7/*SCL*/, 8/*IRQ*/, 17/*RST*/};
static const BoardSDCardPins_t AMOLED_241_SD_PINS =  {4/*MISO*/, 2/*MOSI*/, 3/*SCK*/, 1/*CS*/};


static const  BoardsConfigure_t BOARD_AMOLED_191 = {
    // RM67162 Driver
    RM67162_AMOLED,
    &AMOLED_191_TOUCH_PINS,     //Touch CST816T
    NULL,//PMU
    NULL,//SENSOR
    NULL,//SDCard
    AMOLED_191_BUTTONTS,//Button Pins
    1, //Button Number
    -1,//pixelsPins
    4, //adcPins
    38,//PMICEnPins
    false,//framebuffer
};

static const  BoardsConfigure_t BOARD_AMOLED_191_SPI = {
    // RM67162 Driver
    RM67162_AMOLED_SPI,
    &AMOLED_191_TOUCH_PINS,     //Touch CST816T
    &AMOLED_191_SPI_PMU_PINS,   //PMU
    NULL,                       //SENSOR
    &AMOLED_191_SPI_SD_PINS,    //SDCard
    AMOLED_191_BUTTONTS,        //Button Pins
    1, //Button Number
    -1,//pixelsPins
    4, //adcPins
    38,//PMICEnPins
    false,//framebuffer
};

// T-Display AMOLED H593
// https://www.lilygo.cc/products/t-display-amoled
static const  BoardsConfigure_t BOARD_AMOLED_147 = {
    SH8501_AMOLED,
    &AMOLED_147_TOUCH_PINS,     //Touch
    &AMOLED_147_PMU_PINS,       //PMU
    &AMOLED_147_SENSOR_PINS,    //SENSOR
    NULL,//SDCard
    AMOLED_147_BUTTONTS, //Button Pins
    2,  //Button Number
    18, // pixelsPins
    -1, //adcPins
    -1,//PMICEnPins
    true,//framebuffer
};


// T-Display AMOLED 2.41 Inch
// https://www.lilygo.cc/
static const  BoardsConfigure_t BOARD_AMOLED_241 = {
    RM690B0_AMOLED,
    &AMOLED_241_TOUCH_PINS, //Touch CS226SE
    &AMOLED_241_PMU_PINS,    //PMU
    NULL,    //SENSOR
    &AMOLED_241_SD_PINS,//SDCard
    AMOLED_241_BUTTONTS, //Button Pins
    1,  //Button Number
    -1, // pixelsPins
    -1, //adcPins
    9,  //PMICEnPins
    false,//framebuffer
};


enum AmoledBoardID {
    LILYGO_AMOLED_147 = 0x01,
    LILYGO_AMOLED_191,
    LILYGO_AMOLED_241,
    LILYGO_AMOLED_191_SPI,
    LILYGO_AMOLED_UNKNOWN,
};

class LilyGo_AMOLED:
    public LilyGo_Display,
    public XPowersAXP2101,
    public TouchDrvCHSC5816,
    public SensorCM32181,
    public TouchDrvCSTXXX,
    public SensorPCF85063
{
public:
    // LILYGO_AMOLED_191_SPI USE BQ25896
    PowersSY6970 SY;
    // LILYGO_AMOLED_241 USE SY6970
    PowersBQ25896 BQ;

    LilyGo_AMOLED();

    ~LilyGo_AMOLED();

    // Automatically identify hardware
    bool begin();

    bool beginAutomatic() __attribute__((deprecated("please use begin instead")));

    // LILYGO 1.91 Inc AMOLED(RM67162) S3R8
    // https://www.lilygo.cc/products/t-display-s3-amoled
    bool beginAMOLED_191(bool touchFunc = true);

    // LILYGO 1.91 Inc AMOLED(RM67162 SPI Interface) S3R8
    bool beginAMOLED_191_SPI(bool touchFunc = true);

    // LILYGO 1.47 Inc AMOLED(SH8501) S3R8
    // https://www.lilygo.cc/products/t-display-amoled
    bool beginAMOLED_147();

    // LILYGO 2.41 Inc AMOLED(RM690B0) S3R8
    // https://www.lilygo.cc/products/t4-s3
    /**
     * @brief  beginAMOLED_241
     * @param  disable_sd:  true : disable sd card init , false: Init SD in begin
     * @param  disable_state_led: true: disable charge led , false: default state
     */
    bool beginAMOLED_241(bool disable_sd = false, bool disable_state_led = false);


    void setBrightness(uint8_t level);
    uint8_t getBrightness();

    // void setRotation(uint8_t r) __attribute__((error("setRotation Method Not implemented")));
    void setRotation(uint8_t rotation);
    uint8_t getRotation();

    void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
    void pushColors(uint16_t *data, uint32_t len);
    void pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t hight, uint16_t *data);
    void pushColorsDMA(uint16_t *data, uint32_t len);

    /**
     * @brief   Hang on SD card
     * @note   If the specified Pin is not passed in, the default Pin will be used as the SPI
     * @param  miso: 1.91 Inch [GPIO13] 1.47 Inch [GPIO47]    2.41 Inch defaults to onboard SD slot
     * @param  mosi: 1.91 Inch [GPIO12] 1.47 Inch [GPIO39]    2.41 Inch defaults to onboard SD slot
     * @param  sclk: 1.91 Inch [GPIO14] 1.47 Inch [GPIO38]    2.41 Inch defaults to onboard SD slot
     * @param  cs:   1.91 Inch [GPIO11] 1.47 Inch [GPIO9]     2.41 Inch defaults to onboard SD slot
     * @retval Returns true if successful, otherwise false
     */
    bool installSD(int miso = -1, int mosi = -1, int sclk = -1, int cs = -1);

    void uninstallSD();

    float readCoreTemp();

    uint16_t  width();
    uint16_t  height();

    // Disable touch, just return the touch press touch point Set to 0, does not actually disable touch
    // https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/issues/70
    void disableTouch();
    void enableTouch();

    // override
    uint8_t getPoint(int16_t *x_array, int16_t *y_array, uint8_t get_point = 1) override;
    bool isPressed() override;
    uint16_t getBattVoltage(void) override;
    uint16_t getVbusVoltage(void) override;
    bool isBatteryConnect(void) override;
    uint16_t getSystemVoltage(void) override;
    bool isCharging(void) override;
    bool isVbusIn(void) override;

    void disableCharge(void) ;
    void enableCharge(void) ;

    // PMU Function , only 1.47' inches support
    void attachPMU(void(*cb)(void));
    uint64_t readPMU();
    void clearPMU();
    void enablePMUInterrupt(uint32_t params);
    void disablePMUInterrupt(uint32_t params);

    const BoardsConfigure_t *getBoardsConfigure();
    const char *getName();
    uint8_t getBoardID();

    void sleep(bool touchpad_sleep_enable = false);

    void disp_sleep();
    void disp_wakeup();
    bool hasTouch();
    bool hasOTG();

    bool needFullRefresh();


    bool hasRTC();
private:

    enum DriverBusType {
        QSPI_DRIVER,
        SPI_DRIVER,
    };

    bool initBUS(DriverBusType type = QSPI_DRIVER);
    bool initPMU();
    void inline setCS();
    void inline clrCS();
    void writeCommand(uint32_t cmd, uint8_t *pdat, uint32_t length);
    uint16_t *pBuffer;
    spi_device_handle_t spi;
    uint8_t _brightness;
    const BoardsConfigure_t *boards;
    bool _touchOnline;
    uint16_t _width, _height;

#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5,0,0)
    temperature_sensor_handle_t temp_sensor;
#endif

    bool  _hasRTC;

    bool _disableTouch;

    SPIClass *spiDev;
};

#ifndef LilyGo_Class
#define LilyGo_Class LilyGo_AMOLED
#endif

