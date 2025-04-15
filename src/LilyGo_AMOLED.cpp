/**
 * @file      LilyGo_AMOLED.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-05-29
 *
 */

#include "LilyGo_AMOLED.h"
#include <driver/gpio.h>

#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <esp_adc_cal.h>
#endif

#if SENSORLIB_VERSION_MINOR == 3 && SENSORLIB_VERSION_PATCH == 0
#ifdef writeBytes
#undef writeBytes
#endif
#endif

#ifndef LCD_CMD_MADCTL
#define LCD_CMD_MADCTL       (0x36)     // Memory data access control
#endif
#ifndef LCD_CMD_CASET
#define LCD_CMD_CASET        (0x2A) // Set column address
#endif

#ifndef LCD_CMD_RASET
#define LCD_CMD_RASET        (0x2B) // Set row address
#endif

#ifndef LCD_CMD_RAMWR
#define LCD_CMD_RAMWR        (0x2C) // Write frame memory
#endif


#ifndef LCD_CMD_SLPIN
#define LCD_CMD_SLPIN        (0x10) // Go into sleep mode (DC/DC, oscillator, scanning stopped, but memory keeps content)
#endif

#ifndef LCD_CMD_BRIGHTNESS
#define LCD_CMD_BRIGHTNESS   (0x51)
#endif

#define SEND_BUF_SIZE           (16384)
#define TFT_SPI_MODE            SPI_MODE0
#define DEFAULT_SPI_HANDLER    (SPI3_HOST)

LilyGo_AMOLED::LilyGo_AMOLED() : boards(NULL), _hasRTC(false), _disableTouch(false)
{
    spiDev = NULL;
    pBuffer = NULL;
    spi = NULL;
    _brightness = AMOLED_DEFAULT_BRIGHTNESS;
    // Prevent previously set hold
    switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_EXT0 :
    case ESP_SLEEP_WAKEUP_EXT1 :
    case ESP_SLEEP_WAKEUP_TIMER:
    case ESP_SLEEP_WAKEUP_ULP :
        gpio_hold_dis(GPIO_NUM_14);
        gpio_deep_sleep_hold_dis();
        break;
    default :
        break;
    }
}

LilyGo_AMOLED::~LilyGo_AMOLED()
{
    if (pBuffer) {
        free(pBuffer);
        pBuffer = NULL;
    }

    if (spiDev) {
        spiDev->end();
        spiDev = NULL;
    }
}

const char *LilyGo_AMOLED::getName()
{
    if (boards == &BOARD_AMOLED_147) {
        return "1.47 inch";
    } else if (boards == &BOARD_AMOLED_191 ) {
        return "1.91 inch";
    } else if (boards == &BOARD_AMOLED_241) {
        return "2.41 inch";
    } else if (boards == &BOARD_AMOLED_191_SPI) {
        return "1.91 inch(SPI Interface)";
    }
    return "Unknown";
}

uint8_t LilyGo_AMOLED::getBoardID()
{
    if (boards == &BOARD_AMOLED_147) {
        return LILYGO_AMOLED_147;
    } else if (boards == &BOARD_AMOLED_191 ) {
        return LILYGO_AMOLED_191;
    } else if (boards == &BOARD_AMOLED_241) {
        return LILYGO_AMOLED_241;
    } else if (boards == &BOARD_AMOLED_191_SPI) {
        return LILYGO_AMOLED_191_SPI;
    }
    return LILYGO_AMOLED_UNKNOWN;
}

const BoardsConfigure_t *LilyGo_AMOLED::getBoardsConfigure()
{
    return boards;
}

uint16_t  LilyGo_AMOLED::width()
{
    return _width;
}

uint16_t  LilyGo_AMOLED::height()
{
    return _height;
}

void inline LilyGo_AMOLED::setCS()
{
    digitalWrite(boards->display.cs, LOW);
}

void inline LilyGo_AMOLED::clrCS()
{
    digitalWrite(boards->display.cs, HIGH);
}

bool LilyGo_AMOLED::isPressed()
{
    if (boards == &BOARD_AMOLED_147) {
        return TouchDrvCHSC5816::isPressed();
    } else if (boards == &BOARD_AMOLED_191 || boards == &BOARD_AMOLED_241 || boards == &BOARD_AMOLED_191_SPI) {
        return TouchDrvCSTXXX::isPressed();
    }
    return false;
}

void LilyGo_AMOLED::disableTouch()
{
    _disableTouch = true;
}

void LilyGo_AMOLED::enableTouch()
{
    _disableTouch = false;
}

uint8_t LilyGo_AMOLED::getPoint(int16_t *x, int16_t *y, uint8_t get_point )
{
    uint8_t point = 0;
    if (boards == &BOARD_AMOLED_147) {
        point =  TouchDrvCHSC5816::getPoint(x, y);
    } else if (boards == &BOARD_AMOLED_191 || boards == &BOARD_AMOLED_241 || boards == &BOARD_AMOLED_191_SPI) {
        point =  TouchDrvCSTXXX::getPoint(x, y);
    }

    // Disable touch, just return the touch press touch point Set to 0, does not actually disable touch
    // https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/issues/70
    if (_disableTouch) {
        return 0;
    }
    return point;
}

uint16_t LilyGo_AMOLED::getBattVoltage(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards->pmu) {
                if (boards == &BOARD_AMOLED_147) {
                    return XPowersAXP2101::getBattVoltage();
                } else  if (boards == &BOARD_AMOLED_241) {
                    return SY.getBattVoltage();
                } else if (boards == &BOARD_AMOLED_191_SPI) {
                    return BQ.getBattVoltage();
                }
            }
        } else if (boards->adcPins != -1) {
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
            esp_adc_cal_characteristics_t adc_chars;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4,4,7)
            esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
#else
            esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
#endif
            uint32_t v1 = 0,  raw = 0;
            raw = analogRead(boards->adcPins);
            v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;
#else
            uint32_t v1 = analogReadMilliVolts(boards->adcPins);
            v1 *= 2;   //The hardware voltage divider resistor is half of the actual voltage, multiply it by 2 to get the true voltage
#endif
            return v1;
        }
    }
    return 0;
}

uint16_t LilyGo_AMOLED::getVbusVoltage(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                return XPowersAXP2101::getVbusVoltage();
            } else  if (boards == &BOARD_AMOLED_241) {
                return SY.getVbusVoltage();
            } else if (boards == &BOARD_AMOLED_191_SPI) {
                return BQ.getVbusVoltage();
            }
        }
    }
    return 0;
}

bool LilyGo_AMOLED::isBatteryConnect(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                return XPowersAXP2101::isBatteryConnect();
            } else  if (boards == &BOARD_AMOLED_241 || boards == &BOARD_AMOLED_191_SPI) {
                return getVbusVoltage() != 0;
            }
        }
    }
    return false;
}

uint16_t LilyGo_AMOLED::getSystemVoltage(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                return XPowersAXP2101::getSystemVoltage();
            } else  if (boards == &BOARD_AMOLED_241) {
                return SY.getSystemVoltage();
            } else if (boards == &BOARD_AMOLED_191_SPI) {
                return BQ.getSystemVoltage();
            }
        }
    }
    return 0;
}

bool LilyGo_AMOLED::isCharging(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                return XPowersAXP2101::isCharging();
            } else  if (boards == &BOARD_AMOLED_241) {
                return SY.isCharging();
            } else  if (boards == &BOARD_AMOLED_191_SPI) {
                return BQ.isCharging();
            }
        }
    }
    return false;
}

bool LilyGo_AMOLED::isVbusIn(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                return XPowersAXP2101::isVbusIn();
            } else  if (boards == &BOARD_AMOLED_241 ) {
                return SY.isVbusIn();
            } else  if (boards == &BOARD_AMOLED_191_SPI) {
                return BQ.isVbusIn();
            }
        }
    }
    return false;
}

void LilyGo_AMOLED::disableCharge(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                XPowersAXP2101::setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_0MA);
            } else  if (boards == &BOARD_AMOLED_241 ) {
                SY.disableCharge();
            } else  if (boards == &BOARD_AMOLED_191_SPI) {
                BQ.disableCharge();
            }
        }
    }
}

void LilyGo_AMOLED::enableCharge(void)
{
    if (boards) {
        if (boards->pmu) {
            if (boards == &BOARD_AMOLED_147) {
                XPowersAXP2101::setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_200MA);
            } else  if (boards == &BOARD_AMOLED_241 ) {
                SY.enableCharge();
            } else  if (boards == &BOARD_AMOLED_191_SPI) {
                BQ.enableCharge();
            }
        }
    }
}

uint32_t deviceScan(TwoWire *_port, Stream *stream)
{
    stream->println("Devices Scan start.");
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
    return nDevices;
}

bool LilyGo_AMOLED::initPMU()
{
    bool res = XPowersAXP2101::init(Wire, boards->pmu->sda, boards->pmu->scl, AXP2101_SLAVE_ADDRESS);
    if (!res) {
        return false;
    }

    clearPMU();

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

bool LilyGo_AMOLED::initBUS(DriverBusType type)
{
    assert(boards);
    log_i("=====CONFIGURE======");
    log_i("RST    > %d", boards->display.rst);
    log_i("CS     > %d", boards->display.cs);
    log_i("SCK    > %d", boards->display.sck);
    log_i("D0     > %d", boards->display.d0);
    log_i("D1     > %d", boards->display.d1);
    log_i("D2     > %d", boards->display.d2);
    log_i("D3     > %d", boards->display.d3);
    log_i("TE     > %d", boards->display.te);
    log_i("Freq   > %d", boards->display.freq);
    log_i("Power  > %d", boards->PMICEnPins);
    log_i("==================");

    _width = boards->display.width;
    _height = boards->display.height;

    pinMode(boards->display.rst, OUTPUT);
    pinMode(boards->display.cs, OUTPUT);

    if (boards->display.te != -1) {
        pinMode(boards->display.te, INPUT);
    }

    if (boards->PMICEnPins != -1) {
        pinMode(boards->PMICEnPins, OUTPUT);
        digitalWrite(boards->PMICEnPins, HIGH);
    }

    //reset display
    digitalWrite(boards->display.rst, HIGH);
    delay(200);
    digitalWrite(boards->display.rst, LOW);
    delay(300);
    digitalWrite(boards->display.rst, HIGH);
    delay(200);

    if (type == QSPI_DRIVER) {
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
    } else {
        pinMode(boards->display.d1, OUTPUT);    //set dc output
        spiDev = new SPIClass(HSPI);
        assert(spiDev);
        spiDev->begin(boards->display.sck, -1 /*miso */, boards->display.d0);
    }
    // prevent initialization failure
    int retry = 2;
    while (retry--) {
        const lcd_cmd_t *t = boards->display.initSequence;
        for (uint32_t i = 0; i < boards->display.initSize; i++) {
            writeCommand(t[i].addr, (uint8_t *)t[i].param, t[i].len & 0x1F);
            if (t[i].len & 0x80) {
                delay(120);
            }
            if (t[i].len & 0x20) {
                delay(10);
            }
        }
    }
    return true;
}


bool LilyGo_AMOLED::begin()
{
    //Try find 1.47 inch i2c devices
    Wire.begin(1, 2);
    Wire.beginTransmission(AXP2101_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        return beginAMOLED_147();
    }

    log_e("Unable to detect 1.47-inch board model!");


    Wire.end();

    delay(10);

    // Try find 1.91 inch i2c devices
    Wire.begin(3, 2);
    Wire.beginTransmission(CSTXXX_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {

        // Check RTC Slave address
        Wire.beginTransmission(0x51);
        if (Wire.endTransmission() == 0) {
            log_i("Detect 1.91-inch SPI board model!");
            return beginAMOLED_191_SPI(true);
        } else {
            log_i("Detect 1.91-inch QSPI board model!");
            return beginAMOLED_191(true);
        }
    }
    log_e("Unable to detect 1.91-inch touch board model!");

    Wire.end();

    delay(10);

    // Try find 2.41 inch i2c devices
    Wire.begin(6, 7);
    Wire.beginTransmission(SY6970_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        return beginAMOLED_241();
    }
    log_e("Unable to detect 2.41-inch touch board model!");

    Wire.end();


    log_e("Begin 1.91-inch no touch board model");

    return beginAMOLED_191(false);
}


bool LilyGo_AMOLED::beginAutomatic()
{
    return begin();
}

bool LilyGo_AMOLED::beginAMOLED_191(bool touchFunc)
{
    boards = &BOARD_AMOLED_191;

    initBUS();

    if (touchFunc && boards->touch) {
        if (boards->touch->sda != -1 && boards->touch->scl != -1) {
            Wire.begin(boards->touch->sda, boards->touch->scl);
            deviceScan(&Wire, &Serial);

            // Try to find touch device
            Wire.beginTransmission(CST816_SLAVE_ADDRESS);
            if (Wire.endTransmission() == 0) {
                TouchDrvCSTXXX::setTouchDrvModel(TouchDrv_CST8XX);
                TouchDrvCSTXXX::setPins(boards->touch->rst, boards->touch->irq);
                bool res = TouchDrvCSTXXX::begin(Wire, CST816_SLAVE_ADDRESS, boards->touch->sda, boards->touch->scl);
                if (!res) {
                    log_e("Failed to find CST816T - check your wiring!");
                    // return false;
                    _touchOnline = false;
                } else {
                    _touchOnline = true;
                    TouchDrvCSTXXX::setCenterButtonCoordinate(600, 120);  //AMOLED 1.91 inch
                }
            }
        }
    } else {
        _touchOnline = false;
    }

    setRotation(0);

    return true;
}

bool LilyGo_AMOLED::beginAMOLED_191_SPI(bool touchFunc)
{
    boards = &BOARD_AMOLED_191_SPI;

    initBUS(SPI_DRIVER);

    if (boards->pmu) {
        uint8_t slaveAddress = 0;
        Wire.begin(boards->pmu->sda, boards->pmu->scl);
        deviceScan(&Wire, &Serial);

        Wire.beginTransmission(SY6970_SLAVE_ADDRESS);
        if (Wire.endTransmission() == 0) {
            slaveAddress = SY6970_SLAVE_ADDRESS;
            log_i("Detected SY6970 PPM chip");
        }
        Wire.beginTransmission(BQ25896_SLAVE_ADDRESS);
        if (Wire.endTransmission() == 0) {
            slaveAddress = BQ25896_SLAVE_ADDRESS;
            log_i("Detected Ti BQ25896 PPM chip");
        }
        if (slaveAddress == 0) {
            return false;
        }
        if (BQ.init(Wire, boards->pmu->sda, boards->pmu->scl, slaveAddress)) {
            BQ.enableMeasure();
            BQ.disableOTG();
            BQ.disableCharge();    //Default disable charge function
        } else {
            log_e("begin pmu failed !");
        }
    }

#if SENSORLIB_VERSION_MINOR > 2
    _hasRTC = SensorPCF85063::begin(Wire, boards->pmu->sda, boards->pmu->scl);
#else
    _hasRTC = SensorPCF85063::init(Wire, boards->pmu->sda, boards->pmu->scl);
#endif
    if (!_hasRTC) {
        log_e("begin rtc failed!");
    }

    if (touchFunc && boards->touch) {
        if (boards->touch->sda != -1 && boards->touch->scl != -1) {
            Wire.begin(boards->touch->sda, boards->touch->scl);
            deviceScan(&Wire, &Serial);

            // Try to find touch device
            Wire.beginTransmission(CST816_SLAVE_ADDRESS);
            if (Wire.endTransmission() == 0) {
                TouchDrvCSTXXX::setTouchDrvModel(TouchDrv_CST8XX);
                TouchDrvCSTXXX::setPins(boards->touch->rst, boards->touch->irq);
                bool res = TouchDrvCSTXXX::begin(Wire, CST816_SLAVE_ADDRESS, boards->touch->sda, boards->touch->scl);
                if (!res) {
                    log_e("Failed to find CST816T - check your wiring!");
                    // return false;
                    _touchOnline = false;
                } else {
                    _touchOnline = true;
                    TouchDrvCSTXXX::setCenterButtonCoordinate(600, 120);  //AMOLED 1.91 inch
                }
            }
        }
    } else {
        _touchOnline = false;
    }

    setRotation(0);

    installSD();

    return true;
}



bool LilyGo_AMOLED::beginAMOLED_241(bool disable_sd, bool disable_state_led)
{
    boards = &BOARD_AMOLED_241;

    initBUS();

    if (boards->pmu) {
        Wire.begin(boards->pmu->sda, boards->pmu->scl);
        SY.init(Wire, boards->pmu->sda, boards->pmu->scl, SY6970_SLAVE_ADDRESS);
        SY.enableMeasure();
        SY.disableOTG();
        if (disable_state_led) {
            SY.disableStatLed();
        }
    }

    if (boards->touch) {
        // Try to find touch device
        Wire.beginTransmission(CST226SE_SLAVE_ADDRESS);
        if (Wire.endTransmission() == 0) {
            TouchDrvCSTXXX::setTouchDrvModel(TouchDrv_CST226);
            TouchDrvCSTXXX::setPins(boards->touch->rst, boards->touch->irq);
            bool res = TouchDrvCSTXXX::begin(Wire, CST226SE_SLAVE_ADDRESS, boards->touch->sda, boards->touch->scl);
            if (!res) {
                log_e("Failed to find CST226SE - check your wiring!");
                // return false;
            } else {
                _touchOnline = true;
            }
        }
    }

    if (boards->sd && !disable_sd) {
        SPI.begin(boards->sd->sck, boards->sd->miso, boards->sd->mosi);
        // Set mount point to /fs
        if (!SD.begin(boards->sd->cs, SPI, 4000000U, "/fs")) {
            log_e("Failed to detect SD Card!");
        }
        if (SD.cardType() != CARD_NONE) {
            log_i("SD Card Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
        }
    }

    setRotation(0);

    return true;
}

// Default SPI Pin
#define AMOLED_191_DEFAULT_MISO  13
#define AMOLED_191_DEFAULT_MOSI  12
#define AMOLED_191_DEFAULT_SCLK  14
#define AMOLED_191_DEFAULT_CS    11

#define AMOLED_147_DEFAULT_MISO  47
#define AMOLED_147_DEFAULT_MOSI  39
#define AMOLED_147_DEFAULT_SCLK  38
#define AMOLED_147_DEFAULT_CS    9
/**
 * @brief   Hang on SD card
 * @note   If the specified Pin is not passed in, the default Pin will be used as the SPI
 * @param  miso: 1.91 Inch [GPIO13] 1.47 Inch [GPIO47]    2.41 Inch defaults to onboard SD slot
 * @param  mosi: 1.91 Inch [GPIO12] 1.47 Inch [GPIO39]    2.41 Inch defaults to onboard SD slot
 * @param  sclk: 1.91 Inch [GPIO14] 1.47 Inch [GPIO38]    2.41 Inch defaults to onboard SD slot
 * @param  cs:   1.91 Inch [GPIO11] 1.47 Inch [GPIO9]     2.41 Inch defaults to onboard SD slot
 * @retval Returns true if successful, otherwise false
 */
bool LilyGo_AMOLED::installSD(int miso, int mosi, int sclk, int cs)
{
    if (boards == &BOARD_AMOLED_241 || boards == &BOARD_AMOLED_191_SPI) {
        miso = boards->sd->miso;
        mosi = boards->sd->mosi;
        sclk = boards->sd->sck;
        cs = boards->sd->cs;
    } else if (boards == &BOARD_AMOLED_147) {
        sclk = (sclk == -1)  ? AMOLED_147_DEFAULT_SCLK : sclk;
        miso = (miso == -1) ? AMOLED_147_DEFAULT_MISO : miso;
        mosi = (mosi == -1) ? AMOLED_147_DEFAULT_MOSI : mosi;
        cs = (cs == -1)   ? AMOLED_147_DEFAULT_CS : cs;
    } else if (boards == &BOARD_AMOLED_191) {
        sclk = (sclk == -1)  ? AMOLED_191_DEFAULT_SCLK : sclk;
        miso = (miso == -1) ? AMOLED_191_DEFAULT_MISO : miso;
        mosi = (mosi == -1) ? AMOLED_191_DEFAULT_MOSI : mosi;
        cs = (cs == -1)   ? AMOLED_191_DEFAULT_CS : cs;
    }

    SPI.begin(sclk, miso, mosi);

    // Set mount point to /fs
    if (!SD.begin(cs, SPI, 4000000U, "/fs")) {
        log_e("Failed to detect SD Card!!");
        return false;
    }
    if (SD.cardType() != CARD_NONE) {
        log_i("SD Card Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
        return true;
    }
    return false;
}

void LilyGo_AMOLED::uninstallSD()
{
    SD.end();
}

bool LilyGo_AMOLED::beginAMOLED_147()
{
    boards = &BOARD_AMOLED_147;

    if (!initPMU()) {
        log_e("Failed to find AXP2101 - check your wiring!");
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

    TouchDrvCHSC5816::setPins(boards->touch->rst, boards->touch->irq);
    _touchOnline = TouchDrvCHSC5816::begin(Wire, CHSC5816_SLAVE_ADDRESS, boards->touch->sda, boards->touch->scl);
    if (!_touchOnline) {
        log_e("Failed to find CHSC5816 - check your wiring!");
        // return false;
    } else {
        TouchDrvCHSC5816::setMaxCoordinates(_width, _height);
        TouchDrvCHSC5816::setSwapXY(true);
        TouchDrvCHSC5816::setMirrorXY(false, true);
    }

    // Share I2C Bus
    bool res = SensorCM32181::begin(Wire, CM32181_SLAVE_ADDRESS, boards->sensor->sda, boards->sensor->scl);
    if (!res) {
        log_e("Failed to find CM32181 - check your wiring!");
        // return false;
    } else {
        /*
            Sensitivity mode selection
                SAMPLING_X1
                SAMPLING_X2
                SAMPLING_X1_8
                SAMPLING_X1_4
        */
        SensorCM32181::setSampling(SensorCM32181::SAMPLING_X2),
                      powerOn();
    }

    return true;
}

void LilyGo_AMOLED::writeCommand(uint32_t cmd, uint8_t *pdat, uint32_t length)
{
    if (spiDev) {
        // Write spi command
        setCS();
        spiDev->beginTransaction(SPISettings(boards->display.freq, MSBFIRST, TFT_SPI_MODE));
        digitalWrite(boards->display.d1, LOW);
        spiDev->write(cmd);
        digitalWrite(boards->display.d1, HIGH);
        spiDev->endTransaction();
        clrCS();

        // Write spi data
        if (pdat && length) {
            setCS();
            spiDev->beginTransaction(SPISettings(boards->display.freq, MSBFIRST, TFT_SPI_MODE));
            digitalWrite(boards->display.d1, HIGH);
            spiDev->writeBytes(pdat, length);
            spiDev->endTransaction();
            clrCS();
        }
        return;
    }

    // QSPI
    setCS();
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.flags = (SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR);
    t.cmd = 0x02;
    t.addr = cmd << 8;
    if (length != 0) {
        t.tx_buffer = pdat;
        t.length = 8 * length;
    } else {
        t.tx_buffer = NULL;
        t.length = 0;
    }
    spi_device_polling_transmit(spi, &t);
    clrCS();
}

void LilyGo_AMOLED::setBrightness(uint8_t level)
{
    _brightness = level;
    lcd_cmd_t t = {LCD_CMD_BRIGHTNESS, {level}, 0x01};
    writeCommand(t.addr, t.param, t.len);
}

uint8_t LilyGo_AMOLED::getBrightness()
{
    return _brightness;
}

void LilyGo_AMOLED::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    xs += _offset_x;
    ys += _offset_y;
    xe += _offset_x;
    ye += _offset_y;
    lcd_cmd_t t[3] = {
        {
            LCD_CMD_CASET, {
                (uint8_t)((xs >> 8) & 0xFF),
                (uint8_t)(xs & 0xFF),
                (uint8_t)((xe >> 8) & 0xFF),
                (uint8_t)(xe & 0xFF)
            }, 0x04
        },
        {
            LCD_CMD_RASET, {
                (uint8_t)((ys >> 8) & 0xFF),
                (uint8_t)(ys & 0xFF),
                (uint8_t)((ye >> 8) & 0xFF),
                (uint8_t)(ye & 0xFF)
            }, 0x04
        },
        {
            LCD_CMD_RAMWR, {
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
    if (spiDev) {
        setCS();
        spiDev->beginTransaction(SPISettings(boards->display.freq, MSBFIRST, TFT_SPI_MODE));
        digitalWrite(boards->display.d1, HIGH);
        spiDev->writeBytes((uint8_t *)data, len * sizeof(uint16_t));
        spiDev->endTransaction();
        clrCS();
        return;
    }

    bool first_send = true;
    uint16_t *p = data;
    assert(p);
    assert(spi);
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
        uint16_t _x = this->height() - (y + hight);
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

void LilyGo_AMOLED::pushColorsDMA(uint16_t *data, uint32_t len)
{
    if (!spi) return;

    bool first_send = true;
    setCS();

    while (len > 0) {
        size_t chunk_size = len;
        if (chunk_size > SEND_BUF_SIZE) {
            chunk_size = SEND_BUF_SIZE;
        }

        spi_transaction_ext_t t = {0};
        memset(&t, 0, sizeof(t));

        if (first_send) {
            t.base.flags = SPI_TRANS_MODE_QIO;
            t.base.cmd = 0x32;
            t.base.addr = 0x002C00;
            first_send = 0;
        } else {
            t.base.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
            t.command_bits = 0;
            t.address_bits = 0;
            t.dummy_bits = 0;
        }

        t.base.tx_buffer = data;
        t.base.length = chunk_size * 16;

        esp_err_t ret = spi_device_queue_trans(spi, &t.base, portMAX_DELAY);
        if (ret != ESP_OK) {
            log_e("DMA transfer failed!");
        }

        spi_transaction_t *trans_result;
        ret = spi_device_get_trans_result(spi, &trans_result, portMAX_DELAY);
        if (ret != ESP_OK) {
            log_e("DMA SPI transfer failed!");
        }

        data += chunk_size;
        len -= chunk_size;
    }

    clrCS();
}

float LilyGo_AMOLED::readCoreTemp()
{
    return temperatureRead();
}


void LilyGo_AMOLED::attachPMU(void(*cb)(void))
{
    assert(boards);
    if (boards->pmu) {
        pinMode(boards->pmu->irq, INPUT_PULLUP);
        attachInterrupt(boards->pmu->irq, cb, FALLING);
    }
}

uint64_t LilyGo_AMOLED::readPMU()
{
    assert(boards);
    if (!boards->pmu) {
        return 0;
    }
    if (boards == &BOARD_AMOLED_147) {
        return XPowersAXP2101::getIrqStatus();
    }
    return 0;
}

void LilyGo_AMOLED::clearPMU()
{
    if (boards) {
        if (boards->pmu && (boards == &BOARD_AMOLED_147)) {
            log_i("clearPMU");
            XPowersAXP2101::clearIrqStatus();
        }
    }
}

void LilyGo_AMOLED::enablePMUInterrupt(uint32_t params)
{
    if (boards) {
        if (boards->pmu && (boards == &BOARD_AMOLED_147)) {
            XPowersAXP2101::enableIRQ(params);
        }
    }
}
void LilyGo_AMOLED::disablePMUInterrupt(uint32_t params)
{
    if (boards) {
        if (boards->pmu && (boards == &BOARD_AMOLED_147)) {
            XPowersAXP2101::disableIRQ(params);
        }
    }
}


void LilyGo_AMOLED::sleep(bool touchpad_sleep_enable)
{
    assert(boards);

    //Wire amoled to sleep mode
    lcd_cmd_t t = {LCD_CMD_SLPIN, {0x00}, 1}; //Sleep in
    writeCommand(t.addr, t.param, t.len);

    if (boards) {

        if (boards == &BOARD_AMOLED_241 || boards == &BOARD_AMOLED_191_SPI) {

            if (boards == &BOARD_AMOLED_241) {
                SY.disableADCMeasure();
                SY.disableOTG();
            } else {
                BQ.disableMeasure();
                BQ.disableOTG();
            }

            // Disable amoled power
            digitalWrite(boards->PMICEnPins, LOW);

            if (touchpad_sleep_enable) {
                TouchDrvCSTXXX::sleep();
            }

        } else if (boards == &BOARD_AMOLED_147) {
            log_i("PMU Disable AMOLED Power");

            // Turn off Sensor
            SensorCM32181::powerDown();

            // Turn off ADC data monitoring to save power
            disableTemperatureMeasure();
            disableBattDetection();
            disableVbusVoltageMeasure();
            disableBattVoltageMeasure();
            disableSystemVoltageMeasure();
            setChargingLedMode(XPOWERS_CHG_LED_OFF);

            // Disable amoled power
            disableBLDO1();
            disableALDO3();

            // Don't turn off ALDO1
            // disableALDO1();

            // Keep touch reset to HIGH
            if (touchpad_sleep_enable) {
                digitalWrite(boards->touch->rst, HIGH);
                gpio_hold_en((gpio_num_t )boards->touch->rst);
                gpio_deep_sleep_hold_en();
                // Enter sleep mode
                TouchDrvCHSC5816::sleep();
            }

        } else {
            if (boards->PMICEnPins != -1) {
                // Disable amoled power
                digitalWrite(boards->PMICEnPins, LOW);
                if (touchpad_sleep_enable) {
                    TouchDrvCSTXXX::sleep();
                }
            }
        }
    }
}

void LilyGo_AMOLED::disp_sleep()
{
    lcd_cmd_t t = {LCD_CMD_SLPIN, {0x00}, 1};// Sleep in
    writeCommand(t.addr, t.param, t.len);
}

void LilyGo_AMOLED::disp_wakeup()
{
    lcd_cmd_t t = {0x11, {0x00}, 1};// Sleep Out
    writeCommand(t.addr, t.param, t.len);
}

bool LilyGo_AMOLED::hasTouch()
{
    if (boards && _touchOnline) {
        if (boards->touch) {
            return true;
        }
    }
    return false;
}

bool LilyGo_AMOLED::hasOTG()
{
    uint8_t board = getBoardID();
    if (board == LILYGO_AMOLED_241 || board == LILYGO_AMOLED_191_SPI) {
        return true;
    }
    return false;
}

void LilyGo_AMOLED::setRotation(uint8_t rotation)
{
    uint8_t data = 0x00;
    rotation %= 4;
    _rotation = rotation;
    if (boards == &BOARD_AMOLED_191 || boards == &BOARD_AMOLED_191_SPI) {
        switch (_rotation) {
        case 1:
            data = RM67162_MADCTL_RGB;
            _height = boards->display.height;
            _width = boards->display.width;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(true);
                TouchDrvCSTXXX::setMirrorXY(true, false);
            }
            break;
        case 2:
            data = RM67162_MADCTL_MV | RM67162_MADCTL_MY | RM67162_MADCTL_RGB;
            _height = boards->display.width;
            _width = boards->display.height;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(false);
                TouchDrvCSTXXX::setMirrorXY(true, true);
            }
            break;
        case 3:
            data = RM67162_MADCTL_MX | RM67162_MADCTL_MY | RM67162_MADCTL_RGB;
            _height = boards->display.height;
            _width = boards->display.width;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(true);
                TouchDrvCSTXXX::setMirrorXY(false, true);
            }
            break;
        default:
            data = RM67162_MADCTL_MX | RM67162_MADCTL_MV | RM67162_MADCTL_RGB;
            _height = boards->display.width;
            _width = boards->display.height;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(false);
                TouchDrvCSTXXX::setMirrorXY(false, false);
            }
            break;
        }
        writeCommand(LCD_CMD_MADCTL, &data, 1);
    } else if (boards == &BOARD_AMOLED_241) {
        switch (_rotation) {
        case 1:
            _offset_x = 16;
            _offset_y = 0;
            data = RM690B0_MADCTL_RGB;
            _height = boards->display.width;
            _width = boards->display.height;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(false);
                TouchDrvCSTXXX::setMirrorXY(false, false);
            }
            break;
        case 2:
            _offset_x = 0;
            _offset_y = 16;
            data = RM690B0_MADCTL_MV | RM690B0_MADCTL_MY | RM690B0_MADCTL_RGB;
            _height = boards->display.height;
            _width = boards->display.width;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(true);
                TouchDrvCSTXXX::setMirrorXY(true, false);
            }
            break;
        case 3:
            _offset_x = 16;
            _offset_y = 0;
            data = RM690B0_MADCTL_MX | RM690B0_MADCTL_MY | RM690B0_MADCTL_RGB;
            _height = boards->display.width;
            _width = boards->display.height;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(false);
                TouchDrvCSTXXX::setMirrorXY(true, true);
            }
            break;
        default:
            _offset_x = 0;
            _offset_y = 16;
            data = RM690B0_MADCTL_MX | RM690B0_MADCTL_MV | RM690B0_MADCTL_RGB;
            _height = boards->display.height;
            _width = boards->display.width;
            if (_touchOnline) {
                TouchDrvCSTXXX::setMaxCoordinates(_width, _height);
                TouchDrvCSTXXX::setSwapXY(true);
                TouchDrvCSTXXX::setMirrorXY(false, true);
            }
            break;
        }
        writeCommand(LCD_CMD_MADCTL, &data, 1);
    } else {
        log_e("The screen you are currently using does not support screen rotation!!!");
    }
}

uint8_t LilyGo_AMOLED::getRotation()
{
    return (_rotation);
}

bool LilyGo_AMOLED::needFullRefresh()
{
    if (boards) {
        return boards->display.fullRefresh;
    }
    return false;
}

bool LilyGo_AMOLED::hasRTC()
{
    return _hasRTC;
}