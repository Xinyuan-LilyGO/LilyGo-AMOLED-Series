/**
 * @file      LilyGo_Wristband.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-10-06
 *
 */
#include <sys/cdefs.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_commands.h>
#include <esp_check.h>
#include <hal/spi_types.h>
#include <driver/spi_common.h>
#include <esp_adc_cal.h>
#include "LilyGo_Wristband.h"
#include "initSequence.h"

static volatile bool touchDetected;
static void touchISR()
{
    touchDetected = true;
}


__BEGIN_DECLS


#define BOARD_DISP_HOST     SPI3_HOST

#define TAG  "jd9613"

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    uint8_t fb_bits_per_pixel;
    uint16_t *frame_buffer;
    uint8_t rotation;
} jd9613_panel_t;

static esp_err_t panel_jd9613_del(esp_lcd_panel_t *panel);
static esp_err_t panel_jd9613_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_jd9613_init(esp_lcd_panel_t *panel);
static esp_err_t panel_jd9613_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);



static esp_err_t esp_lcd_new_panel_jd9613(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    jd9613_panel_t *jd9613 = NULL;
    uint8_t fb_bits_per_pixel = 0;

    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");

    jd9613 = (jd9613_panel_t *)calloc(1, sizeof(jd9613_panel_t));

    ESP_GOTO_ON_FALSE(jd9613, ESP_ERR_NO_MEM, err, TAG, "no mem for jd9613 panel");


    jd9613->frame_buffer = (uint16_t *)heap_caps_malloc(JD9613_WIDTH * JD9613_HEIGHT * 2, MALLOC_CAP_DMA);
    if (!jd9613->frame_buffer) {
        free(jd9613);
        return ESP_FAIL;
    }

    if (panel_dev_config->reset_gpio_num >= 0) {
        pinMode(panel_dev_config->reset_gpio_num, OUTPUT);
    }

    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565
        fb_bits_per_pixel = 16;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    jd9613->io = io;
    jd9613->fb_bits_per_pixel = fb_bits_per_pixel;
    jd9613->reset_gpio_num = panel_dev_config->reset_gpio_num;
    jd9613->reset_level = panel_dev_config->flags.reset_active_high;
    jd9613->base.del = panel_jd9613_del;
    jd9613->base.reset = panel_jd9613_reset;
    jd9613->base.init = panel_jd9613_init;
    jd9613->base.draw_bitmap = panel_jd9613_draw_bitmap;
    jd9613->base.invert_color = NULL;
    jd9613->base.set_gap = NULL;
    jd9613->base.mirror = NULL;
    jd9613->base.swap_xy = NULL;
#if ESP_IDF_VERSION <  ESP_IDF_VERSION_VAL(4,4,6)
    jd9613->base.disp_off = NULL;
#else
    jd9613->base.disp_on_off = NULL;
#endif
    jd9613->rotation = 0;

    *ret_panel = &(jd9613->base);
    ESP_LOGD(TAG, "new jd9613 panel @%p", jd9613);

    return ESP_OK;

err:
    if (jd9613) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            pinMode(panel_dev_config->reset_gpio_num, OPEN_DRAIN);
        }
        free(jd9613);
    }
    return ret;
}


static esp_err_t panel_jd9613_del(esp_lcd_panel_t *panel)
{
    jd9613_panel_t *jd9613 = __containerof(panel, jd9613_panel_t, base);

    if (jd9613->reset_gpio_num >= 0) {
        pinMode(jd9613->reset_gpio_num, OPEN_DRAIN);
    }
    ESP_LOGD(TAG, "del jd9613 panel @%p", jd9613);
    free(jd9613);
    free(jd9613->frame_buffer);
    return ESP_OK;
}

static esp_err_t panel_jd9613_reset(esp_lcd_panel_t *panel)
{
    jd9613_panel_t *jd9613 = __containerof(panel, jd9613_panel_t, base);
    esp_lcd_panel_io_handle_t io = jd9613->io;

    // perform hardware reset
    if (jd9613->reset_gpio_num >= 0) {
        digitalWrite(jd9613->reset_gpio_num, jd9613->reset_level);
        delay((100));
        digitalWrite(jd9613->reset_gpio_num, !jd9613->reset_level);
        delay((100));
    } else {
        // perform software reset
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        delay((20)); // spec, wait at least 5ms before sending new command
    }

    return ESP_OK;
}


static esp_err_t panel_jd9613_init(esp_lcd_panel_t *panel)
{
    jd9613_panel_t *jd9613 = __containerof(panel, jd9613_panel_t, base);
    esp_lcd_panel_io_handle_t io = jd9613->io;

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    int cmd = 0;
    while (jd9613_cmd[cmd].len != 0xff) {
        esp_lcd_panel_io_tx_param(io, jd9613_cmd[cmd].addr, jd9613_cmd[cmd].param, (jd9613_cmd[cmd].len - 1) & 0x1F);
        cmd++;
    }

    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    delay((120));

    esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);
    delay((120));

    return ESP_OK;
}

static esp_err_t panel_jd9613_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    jd9613_panel_t *jd9613 = __containerof(panel, jd9613_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = jd9613->io;

    uint32_t width = x_start + x_end;
    uint32_t height = y_start + y_end;
    uint32_t _x = x_start, _y = y_start, _xe = width, _ye = height;
    size_t write_colors_bytes = width * height * sizeof(uint16_t);
    uint16_t *data_ptr = (uint16_t *)color_data;

    if (jd9613->rotation) {
        _x = JD9613_WIDTH - (y_start + height);
        _y = x_start;
        _xe = height;
        _ye = width;
    }

    uint8_t data1[] = {lowByte(_x >> 8), lowByte(_x), lowByte((_xe - 1) >> 8), lowByte(_xe - 1)};
    esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, data1, 4);
    uint8_t data2[] = {lowByte(_y >> 8), lowByte(_y), lowByte((_ye - 1) >> 8), lowByte(_ye - 1)};
    esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, data2, 4);

    if (jd9613->rotation) {
        int index = 0;
        uint16_t *pdat = (uint16_t *)color_data;
        for (uint16_t j = 0; j < width; j++) {
            for (uint16_t i = 0; i < height; i++) {
                jd9613->frame_buffer[index++] = pdat[width * (height - i - 1) + j];
            }
        }
        data_ptr = jd9613->frame_buffer;
    }
    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, data_ptr, write_colors_bytes);
    return ESP_OK;
}

__END_DECLS


LilyGo_Wristband::LilyGo_Wristband(): _brightness(AMOLED_DEFAULT_BRIGHTNESS), panel_handle(NULL), threshold(2000)
{
}

LilyGo_Wristband::~LilyGo_Wristband()
{
    esp_lcd_panel_del(panel_handle);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
    ledcDetach(BOARD_VIBRATION_PIN);
#else
    ledcDetachPin(BOARD_VIBRATION_PIN);
#endif
    touchDetachInterrupt(BOARD_TOUCH_BUTTON);
}

void LilyGo_Wristband::setTouchThreshold(uint32_t threshold)
{
    touchDetachInterrupt(BOARD_TOUCH_BUTTON);
    touchAttachInterrupt(BOARD_TOUCH_BUTTON, touchISR, threshold);
}

void LilyGo_Wristband::detachTouch()
{
    touchDetachInterrupt(BOARD_TOUCH_BUTTON);
}

bool LilyGo_Wristband::getTouched()
{
    if (touchDetected) {
        touchDetected = false;
        return touchInterruptGetLastStatus(BOARD_TOUCH_BUTTON);
    }
    return false;
}

bool LilyGo_Wristband::isPressed()
{
    return touchInterruptGetLastStatus(BOARD_TOUCH_BUTTON);
}

bool LilyGo_Wristband::begin()
{
    if (panel_handle) {
        return true;
    }

    // Initialize display
    initBUS();

    // Initialize touch button
    touchAttachInterrupt(BOARD_TOUCH_BUTTON, touchISR, threshold);

    LilyGo_Button::init(BOARD_TOUCH_BUTTON);

    // Initialize vibration motor
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
    ledcAttach(BOARD_VIBRATION_PIN, 1000, 8);
    // Start up and vibrate suddenly
    ledcWrite(BOARD_VIBRATION_PIN, 50);
    delay(30);
    ledcWrite(BOARD_VIBRATION_PIN, 0);
#else
    ledcSetup(0, 1000, 8);
    ledcAttachPin(BOARD_VIBRATION_PIN, 0);
    // Start up and vibrate suddenly
    ledcWrite(0, 50);
    delay(30);
    ledcWrite(0, 0);
#endif



    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);

    // Initialize RTC PCF85063
    bool reslut = SensorPCF85063::init(Wire);
    if (!reslut) {
        log_e("Real time clock initialization failed!");
    }

    pinMode(BOARD_BHI_EN, OUTPUT);
    digitalWrite(BOARD_BHI_EN, HIGH);

    // Initialize Sensor
    SensorBHI260AP::setPins(BOARD_BHI_RST, BOARD_BHI_IRQ);
    reslut = SensorBHI260AP::init(SPI, BOARD_BHI_CS, BOARD_BHI_MOSI, BOARD_BHI_MISO, BOARD_BHI_SCK);
    if (!reslut) {
        log_e("Motion sensor initialization failed!");
    }

    return true;
}

void LilyGo_Wristband::update()
{
    SensorBHI260AP::update();
    LilyGo_Button::update();
}

void LilyGo_Wristband::setBrightness(uint8_t level)
{
    lcd_cmd_t t = {0x51, {level}, 1};
    writeCommand(t.addr, t.param, t.len);
}

uint8_t LilyGo_Wristband::getBrightness()
{
    return _brightness;
}

void LilyGo_Wristband::setRotation(DispRotation rotation)
{
    assert(panel_handle);
    jd9613_panel_t *jd9613 = __containerof(panel_handle, jd9613_panel_t, base);
    jd9613->rotation = rotation;
    _rotation = rotation;
}

DispRotation LilyGo_Wristband::getRotation()
{
    assert(panel_handle);
    jd9613_panel_t *jd9613 = __containerof(panel_handle, jd9613_panel_t, base);
    return static_cast<DispRotation>(jd9613->rotation);
}

void LilyGo_Wristband::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h)
{
    assert(panel_handle);
    jd9613_panel_t *jd9613 = __containerof(panel_handle, jd9613_panel_t, base);
    uint8_t data1[] = {lowByte(xs >> 8), lowByte(xs), lowByte((xs + w - 1) >> 8), lowByte(xs + w - 1)};
    esp_lcd_panel_io_tx_param(jd9613->io, LCD_CMD_CASET, data1, 4);
    uint8_t data2[] = {lowByte(ys >> 8), lowByte(ys), lowByte((ys + h - 1) >> 8), lowByte(ys + h - 1)};
    esp_lcd_panel_io_tx_param(jd9613->io, LCD_CMD_RASET, data2, 4);
}

void LilyGo_Wristband::pushColors(uint16_t *data, uint32_t len)
{
    assert(panel_handle);
    jd9613_panel_t *jd9613 = __containerof(panel_handle, jd9613_panel_t, base);
    esp_lcd_panel_io_tx_color(jd9613->io, LCD_CMD_RAMWR, data, len);
}

void LilyGo_Wristband::pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *data)
{
    assert(panel_handle);
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, width, height, data);
}

bool LilyGo_Wristband::initBUS()
{
    /*
    spi_bus_config_t buscfg = {
        .mosi_io_num = BOARD_DISP_MOSI,
        .miso_io_num = BOARD_DISP_MISO,
        .sclk_io_num = BOARD_DISP_SCK,
        .quadwp_io_num = BOARD_NONE_PIN,
        .quadhd_io_num = BOARD_NONE_PIN,
        .max_transfer_sz = JD9613_HEIGHT * 80 * sizeof(uint16_t),
    };
    */
    spi_bus_config_t buscfg;
    buscfg.mosi_io_num = BOARD_DISP_MOSI;
    buscfg.miso_io_num = BOARD_DISP_MISO;
    buscfg.sclk_io_num = BOARD_DISP_SCK;
    buscfg.quadwp_io_num = BOARD_NONE_PIN;
    buscfg.quadhd_io_num = BOARD_NONE_PIN;
    buscfg.data4_io_num = 0;
    buscfg.data5_io_num = 0;
    buscfg.data6_io_num = 0;
    buscfg.data7_io_num = 0;
    buscfg.max_transfer_sz = JD9613_HEIGHT * 80 * sizeof(uint16_t);
    buscfg.flags = 0x00;
    buscfg.intr_flags = 0x00;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
    buscfg.isr_cpu_id = INTR_CPU_ID_AUTO;
#endif

    ESP_ERROR_CHECK(spi_bus_initialize(BOARD_DISP_HOST, &buscfg, SPI_DMA_CH_AUTO));

    log_i( "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    /*
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = BOARD_DISP_CS,
        .dc_gpio_num = BOARD_DISP_DC,
        .spi_mode = 0,
        .pclk_hz = DEFAULT_SCK_SPEED,
        .trans_queue_depth = 10,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    */
    esp_lcd_panel_io_spi_config_t io_config;
    io_config.cs_gpio_num = BOARD_DISP_CS;
    io_config.dc_gpio_num = BOARD_DISP_DC;
    io_config.spi_mode = 0;
    io_config.pclk_hz = DEFAULT_SCK_SPEED;
    io_config.trans_queue_depth = 10;
    io_config.on_color_trans_done = NULL;
    io_config.user_ctx = NULL;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;

    io_config.flags.dc_low_on_data = 0;
    io_config.flags.octal_mode = 0;
    io_config.flags.lsb_first = 0;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
    io_config.flags.quad_mode = 0;
    io_config.flags.sio_mode = 0;
    io_config.flags.cs_high_active = 0;
#else
    io_config.flags.dc_as_cmd_phase = 0;
#endif


    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BOARD_DISP_HOST, &io_config, &io_handle));

    /*
        esp_lcd_panel_dev_config_t panel_config = {
            .reset_gpio_num = BOARD_DISP_RST,
            .color_space = ESP_LCD_COLOR_SPACE_RGB,
            .bits_per_pixel = 16,
            .flags.reset_active_high = 0,
            .vendor_config = NULL
        };
    */
    esp_lcd_panel_dev_config_t panel_config;
    panel_config.reset_gpio_num = BOARD_DISP_RST;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    panel_config.color_space = ESP_LCD_COLOR_SPACE_RGB;
#else
    panel_config.color_space = LCD_RGB_ELEMENT_ORDER_RGB;
#endif
    panel_config.bits_per_pixel = 16;
    panel_config.flags.reset_active_high = 0;
    panel_config.vendor_config = NULL;

    log_i( "Install JD9613 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9613(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    return true;
}

void LilyGo_Wristband::writeCommand(uint32_t cmd, uint8_t *pdat, uint32_t lenght)
{
    jd9613_panel_t *jd9613 = __containerof(panel_handle, jd9613_panel_t, base);
    esp_lcd_panel_io_tx_param(jd9613->io, cmd, pdat, lenght);
}

void LilyGo_Wristband::enableTouchWakeup(int threshold)
{
    touchSleepWakeUpEnable(BOARD_TOUCH_BUTTON, threshold);
}

void LilyGo_Wristband::sleep()
{
    lcd_cmd_t t = {0x10, {0x00}, 1}; //Sleep in
    writeCommand(t.addr, t.param, t.len);

    esp_deep_sleep_start();
}

void LilyGo_Wristband::wakeup()
{
    lcd_cmd_t t = {0x11, {0x00}, 1};// Sleep Out
    writeCommand(t.addr, t.param, t.len);
}

uint16_t  LilyGo_Wristband::width()
{
    return _rotation ? JD9613_WIDTH : JD9613_HEIGHT;
}

uint16_t  LilyGo_Wristband::height()
{
    return _rotation ? JD9613_HEIGHT : JD9613_WIDTH;
}

bool LilyGo_Wristband::hasTouch()
{
    return false;
}

uint8_t LilyGo_Wristband::getPoint(int16_t *x, int16_t *y, uint8_t get_point )
{
    return 0;
}

uint16_t LilyGo_Wristband::getBattVoltage(void)
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    uint32_t v1 = 0, raw = 0;
    raw = analogRead(BOARD_BAT_ADC);
    v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;
    return v1;
}


int LilyGo_Wristband::getBatteryPercent()
{
    const static int table[11] = {
        3000, 3650, 3700, 3740, 3760, 3795,
        3840, 3910, 3980, 4070, 4100
    };
    uint16_t voltage = getBattVoltage();
    if (voltage < table[0])
        return 0;
    for (int i = 0; i < 11; i++) {
        if (voltage < table[i])
            return i * 10 - (10UL * (int)(table[i] - voltage)) /
                   (int)(table[i] - table[i - 1]);;
    }
    return 100;
}


void LilyGo_Wristband::vibration(uint8_t duty, uint32_t delay_ms)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
    // Start up and vibrate suddenly
    ledcWrite(BOARD_VIBRATION_PIN, duty);
    delay(delay_ms);
    ledcWrite(BOARD_VIBRATION_PIN, 0);
#else
    ledcWrite(0, duty);
    delay(delay_ms);
    ledcWrite(0, 0);
#endif
}


