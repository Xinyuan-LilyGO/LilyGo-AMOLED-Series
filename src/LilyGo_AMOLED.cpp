/**
 * @file      LilyGo_AMOLED.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-05-29
 *
 */

#include "LilyGo_AMOLED.h"
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
#include <driver/temp_sensor.h>
#else
#include <driver/temperature_sensor.h>
#endif

#define SEND_BUF_SIZE           (16384)
#define TFT_SPI_MODE            SPI_MODE0
#define DEFAULT_SPI_HANDLER    (SPI2_HOST)

LilyGo_AMOLED::LilyGo_AMOLED() : boards(NULL)
{
    pBuffer = NULL;
    _brightness = AMOLED_DEFAULT_BRIGHTNESS;
}

LilyGo_AMOLED::~LilyGo_AMOLED()
{
    if (pBuffer) {
        free(pBuffer);
        pBuffer = NULL;
    }
}
const BoardsConfigure_t *LilyGo_AMOLED::getBoarsdConfigure()
{
    return boards;
}

uint16_t  LilyGo_AMOLED::width()
{
    return boards->display.width;
}

uint16_t  LilyGo_AMOLED::height()
{
    return boards->display.height;
}

void inline LilyGo_AMOLED::setCS()
{
    digitalWrite(boards->display.cs, LOW);
}

void inline LilyGo_AMOLED::clrCS()
{
    digitalWrite(boards->display.cs, HIGH);
}

uint8_t LilyGo_AMOLED::getPoint(int16_t *x, int16_t *y, uint8_t get_point )
{
    int16_t tmpX, tmpY;
    uint8_t point =  TouchDrvCHSC5816::getPoint(&tmpX, &tmpY);
    *x = tmpY;
    *y = height() - tmpX;
    return point;
}

void deviceScan(TwoWire *_port, Stream *stream)
{
    uint8_t err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        _port->beginTransmission(addr);
        err = _port->endTransmission();
        if (err == 0) {
            stream->print("I2C device found at address 0x");
            if (addr < 16)
                stream->print("0");
            stream->print(addr, HEX);
            stream->println(" !");
            nDevices++;
        } else if (err == 4) {
            stream->print("Unknow error at address 0x");
            if (addr < 16)
                stream->print("0");
            stream->println(addr, HEX);
        }
    }
    if (nDevices == 0)
        stream->println("No I2C devices found\n");
    else
        stream->println("Done\n");
}

bool LilyGo_AMOLED::initPMU()
{
    bool res = XPowersAXP2101::init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, AXP2101_SLAVE_ADDRESS);
    if (!res) {
        return false;
    }

    setChargingLedMode(XPOWERS_CHG_LED_CTRL_CHG);

    // ALDO1 = AMOLED logic power & Sensor Power voltage
    setALDO1Voltage(1800);
    enableALDO1();

    // ALDO3 = Level conversion enable and AMOLED power supply
    setALDO3Voltage(3300);
    enableALDO3();

    // BLDO1 = AMOLED LOGIC POWER 1.8V
    setBLDO1Voltage(1800);
    enableBLDO1();

    // No use power channel
    disableDC2();
    disableDC3();
    disableDC4();
    disableDC5();
    disableCPUSLDO();

    // Enable PMU ADC
    enableBattDetection();
    enableVbusVoltageMeasure();
    enableBattVoltageMeasure();

    return res;
}

bool LilyGo_AMOLED::initBUS()
{
    pinMode(boards->display.rst, OUTPUT);
    pinMode(boards->display.cs, OUTPUT);
    pinMode(boards->display.te, INPUT);

    //reset display
    digitalWrite(boards->display.rst, HIGH);
    delay(200);
    digitalWrite(boards->display.rst, LOW);
    delay(300);
    digitalWrite(boards->display.rst, HIGH);
    delay(200);

    spi_bus_config_t buscfg = {
        .data0_io_num = boards->display.d0,
        .data1_io_num = boards->display.d1,
        .sclk_io_num = boards->display.sck,
        .data2_io_num = boards->display.d2,
        .data3_io_num = boards->display.d3,
        .data4_io_num = BOARD_NONE_PIN,
        .data5_io_num = BOARD_NONE_PIN,
        .data6_io_num = BOARD_NONE_PIN,
        .data7_io_num = BOARD_NONE_PIN,
        .max_transfer_sz = (SEND_BUF_SIZE * 16) + 8,
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS,
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = boards->display.cmdBit,
        .address_bits = boards->display.addBit,
        .mode = TFT_SPI_MODE,
        .clock_speed_hz = boards->display.freq,
        .spics_io_num = -1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 17,
    };
    esp_err_t ret = spi_bus_initialize(DEFAULT_SPI_HANDLER, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        log_e("spi_bus_initialize fail!");
        return false;
    }
    ret = spi_bus_add_device(DEFAULT_SPI_HANDLER, &devcfg, &spi);
    if (ret != ESP_OK) {
        log_e("spi_bus_add_device fail!");
        return false;
    }

    lcd_cmd_t *t = boards->display.initSequence;
    for (uint32_t i = 0; i < boards->display.initSize; i++) {
        writeCommand(t[i].addr, t[i].param, t[i].len & 0x7F);
        if (t[i].len & 0x80) {
            delay(120);
        }
    }
    return true;
}


bool LilyGo_AMOLED::beginAMOLED_191()
{
    boards = &BOARD_AMOLED_191;
    return initBUS();
}


bool LilyGo_AMOLED::beginAMOLED_147()
{
    esp_err_t ret;

    boards = &BOARD_AMOLED_147;

    if (!initPMU()) {
        Serial.println("PMU is not online...");
        return false;
    }

    if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO) {
        deviceScan(&Wire, &Serial);
    }

    initBUS();


    if (boards->display.frameBufferSize) {
        if (psramFound()) {
            pBuffer = (uint16_t *)ps_malloc(boards->display.frameBufferSize);
        } else {
            pBuffer = (uint16_t *)malloc(boards->display.frameBufferSize);
        }
        assert(pBuffer);
    }

    TouchDrvCHSC5816::setPins(BOARD_TOUCH_RST, BOARD_TOUCH_IRQ);
    bool res = TouchDrvCHSC5816::begin(Wire, CHSC5816_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL);
    if (!res) {
        log_e("Failed to find CHSC5816 - check your wiring!");
        return false;
    }

    res = SensorCM32181::begin(Wire, CM32181_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL);
    if (!res) {
        log_e("Failed to find CM32181 - check your wiring!");
        return false;
    }
    /*
        Sensitivity mode selection
            SAMPLING_X1
            SAMPLING_X2
            SAMPLING_X1_8
            SAMPLING_X1_4
    */
    SensorCM32181::setSampling(SAMPLING_X2),
                  powerOn();


    // Temperature detect
    beginCore();

    return true;
}

void LilyGo_AMOLED::writeCommand(uint32_t cmd, uint8_t *pdat, uint32_t lenght)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.flags = (SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR);
    t.cmd = 0x02;
    t.addr = cmd;
    if (lenght != 0) {
        t.tx_buffer = pdat;
        t.length = 8 * lenght;
    } else {
        t.tx_buffer = NULL;
        t.length = 0;
    }
    setCS();
    spi_device_polling_transmit(spi, &t);
    clrCS();
}

void LilyGo_AMOLED::setBrightness(uint8_t level)
{
    _brightness = level;
    lcd_cmd_t t = {0x5100, {level}, 0x01};
    writeCommand(t.addr, t.param, t.len);
}

uint8_t LilyGo_AMOLED::getBrightness()
{
    return _brightness;
}

void LilyGo_AMOLED::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    lcd_cmd_t t[3] = {
        {
            0x2A00, {
                (uint8_t)((xs >> 8) & 0xFF),
                (uint8_t)(xs & 0xFF),
                (uint8_t)((xe >> 8) & 0xFF),
                (uint8_t)(xe & 0xFF)
            }, 0x04
        },
        {
            0x2B00, {
                (uint8_t)((ys >> 8) & 0xFF),
                (uint8_t)(ys & 0xFF),
                (uint8_t)((ye >> 8) & 0xFF),
                (uint8_t)(ye & 0xFF)
            }, 0x04
        },
        {
            0x2C00, {
                0x00
            }, 0x00
        },
    };

    for (uint32_t i = 0; i < 3; i++) {
        writeCommand(t[i].addr, t[i].param, t[i].len);
    }
}

// Push (aka write pixel) colours to the TFT (use setAddrWindow() first)
void LilyGo_AMOLED::pushColors(uint16_t *data, uint32_t len)
{
    bool first_send = true;
    uint16_t *p = data;
    setCS();
    do {
        size_t chunk_size = len;
        spi_transaction_ext_t t = {0};
        memset(&t, 0, sizeof(t));
        if (first_send) {
            t.base.flags = SPI_TRANS_MODE_QIO;
            t.base.cmd = 0x32 ;
            t.base.addr = 0x002C00;
            first_send = 0;
        } else {
            t.base.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
            t.command_bits = 0;
            t.address_bits = 0;
            t.dummy_bits = 0;
        }
        if (chunk_size > SEND_BUF_SIZE) {
            chunk_size = SEND_BUF_SIZE;
        }
        t.base.tx_buffer = p;
        t.base.length = chunk_size * 16;
        spi_device_polling_transmit(spi, (spi_transaction_t *)&t);
        len -= chunk_size;
        p += chunk_size;
    } while (len > 0);
    clrCS();
}

void LilyGo_AMOLED::pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t hight, uint16_t *data)
{

    if (boards->display.frameBufferSize) {
        assert(pBuffer);
        uint16_t _x = this->width() - (y + hight);
        uint16_t _y = x;
        uint16_t _h = width;
        uint16_t _w = hight;
        uint16_t *p = data;
        uint32_t cum = 0;
        for (uint16_t j = 0; j < width; j++) {
            for (uint16_t i = 0; i < hight; i++) {
                pBuffer[cum] = ((uint16_t)p[width * (hight - i - 1) + j]);
                cum++;
            }
        }
        setAddrWindow(_x, _y, _x + _w - 1, _y + _h - 1);
        pushColors(pBuffer, width * hight);
    } else {
        setAddrWindow(x, y, x + width - 1, y + hight - 1);
        pushColors(data, width * hight);
    }
}


void LilyGo_AMOLED::beginCore()
{
    // https://docs.espressif.com/projects/esp-idf/zh_CN/v4.4.4/esp32s3/api-reference/peripherals/temp_sensor.html
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    temp_sensor_config_t temp_sensor = {
        .dac_offset = TSENS_DAC_L2,
        .clk_div = 6,
    };
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
#else
    // https://docs.espressif.com/projects/esp-idf/zh_CN/v5.0.1/esp32s3/api-reference/peripherals/temp_sensor.html
    static temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    temperature_sensor_install(&temp_sensor_config, &temp_sensor);
    temperature_sensor_enable(temp_sensor);
#endif
}


float LilyGo_AMOLED::readCoreTemp()
{
    float tsens_value;
    // https://docs.espressif.com/projects/esp-idf/zh_CN/v4.4.4/esp32s3/api-reference/peripherals/temp_sensor.html
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    temp_sensor_read_celsius(&tsens_value);
#else
    // https://docs.espressif.com/projects/esp-idf/zh_CN/v5.0.1/esp32s3/api-reference/peripherals/temp_sensor.html
    temperature_sensor_get_celsius(temp_sensor, &tsens_value);
#endif
    return tsens_value;
}


LilyGo_AMOLED amoled;


