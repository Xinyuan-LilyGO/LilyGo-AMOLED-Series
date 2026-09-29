<h1 align = "center">🌟LilyGO AMOLED Series🌟</h1>

[![PlatformIO CI](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/platformio.yml/badge.svg)](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/platformio.yml)
[![Arduino CI](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/arduino_ci.yml/badge.svg)](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/arduino_ci.yml)
[![arduino-library-badge](https://www.ardu-badge.com/badge/LilyGo-AMOLED-Series.svg?)](https://www.ardu-badge.com/LilyGo-AMOLED-Series)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/xinyuan-lilygo/library/LilyGo-AMOLED-Series.svg)](https://registry.platformio.org/libraries/xinyuan-lilygo/LilyGo-AMOLED-Series)

<!-- **English | [中文](README_CN.MD)** -->

## News

- `LilyGo-AMOLED-Series` is currently compatible with lvgl9, but there is only one `lvgl 9.x.x` example, and the rest of the examples are still based on `lvgl 8.x.x`. Please confirm the lvgl version before using the examples. How to change the lvgl version please see the following description
- `LilyGo-AMOLED-Series` esp-idf version example, please jump to this [LilyGo-Display-IDF](https://github.com/Xinyuan-LilyGO/LilyGo-Display-IDF)
- `T4-S3` **GPIO18** has been assigned to the display TE Pin, please do not use this Pin

## 1️⃣Support Product

| Product(PinMap)               | SOC        | Flash | PSRAM    | Resolution | Size      | Luminance |
| ----------------------------- | ---------- | ----- | -------- | ---------- | --------- | --------- |
| [T-Display-AMOLED-Lite][1]    | ESP32-S3R8 | 16MB  | 8MB(OPI) | 194x368    | 1.47 Inch | 450 nits  |
| [T-Display-S3 AMOLED][2]      | ESP32-S3R8 | 16MB  | 8MB(OPI) | 240x536    | 1.91 Inch | 350 nits  |
| [T4-S3][3]                    | ESP32-S3R8 | 16MB  | 8MB(OPI) | 450x600    | 2.41 Inch | 550 nits  |
| [T-Display-S3 AMOLED Plus][4] | ESP32-S3R8 | 16MB  | 8MB(OPI) | 240x536    | 1.91 Inch | 350 nits  |


| Current consumption           | Working current             | sleep current           | sleep mode            |
| ----------------------------- | --------------------------- | ----------------------- | --------------------- |
| [T-Display-AMOLED-Lite][1]    | (240MHz) WiFi On 90~230+ mA | About 1.1mA             | timer wakeup          |
| [T-Display-S3 AMOLED][2]      | (240MHz) WiFi On 90~230+ mA | About 230uA             | external gpio0 wakeup |
| [T4-S3][3]                    | (240MHz) WiFi On 90~230+ mA | About 230uA             | external gpio0 wakeup |
| [T-Display-S3 AMOLED Plus][4] | (240MHz) WiFi On 90~230+ mA | About 230~700uA dynamic | external gpio0 wakeup |


[1]: https://www.lilygo.cc/products/t-display-amoled-lite
[2]: https://www.lilygo.cc/products/t-display-s3-amoled
[3]: https://www.lilygo.cc/products/t4-s3
[4]: https://www.lilygo.cc/products/t-display-s3-amoled-plus

- This code supports T-Display-S3 AMOLED both touch and non-touch versions

## 🔧 Hardware Description

The ESP32-S3 GPIO numbers below are also the Arduino pin numbers.

### T-Display-AMOLED-Lite pinout

| Name                        | GPIO | Note                              |
| --------------------------- | ---- | --------------------------------- |
| Display DATA0               | 7    | SH8501 QSPI                       |
| Display DATA1               | 10   | SH8501 QSPI                       |
| Display DATA2               | 11   | SH8501 QSPI                       |
| Display DATA3               | 12   | SH8501 QSPI                       |
| Display SCLK                | 5    |                                   |
| Display CS                  | 4    |                                   |
| Display RST                 | 40   |                                   |
| Display TE                  | 6    |                                   |
| Touch SDA                   | 1    | Shared I2C bus                    |
| Touch SCL                   | 2    | Shared I2C bus                    |
| Touch IRQ                   | 13   | CHSC5816                          |
| Touch RST                   | 14   | CHSC5816                          |
| PMU SDA                     | 1    | Shared I2C bus                    |
| PMU SCL                     | 2    | Shared I2C bus                    |
| PMU IRQ                     | 3    | AXP2101                           |
| Ambient light sensor SDA    | 1    | Shared I2C bus                    |
| Ambient light sensor SCL    | 2    | Shared I2C bus                    |
| Ambient light sensor IRQ    | 8    | CM32181                           |
| BOOT button                 | 0    | Active low                        |
| User button                 | 21   | Active low                        |
| RGB LED                     | 18   | WS2812                            |
| External SD MISO            | 47   | Library default; no onboard slot  |
| External SD MOSI            | 39   | Library default; no onboard slot  |
| External SD SCLK            | 38   | Library default; no onboard slot  |
| External SD CS              | 9    | Library default; no onboard slot  |
| USB D-                      | 19   | Native USB                        |
| USB D+                      | 20   | Native USB                        |
| UART TX                     | 43   | U0TXD                             |
| UART RX                     | 44   | U0RXD                             |

### T-Display-S3 AMOLED pinout

| Name                        | GPIO | Note                                      |
| --------------------------- | ---- | ----------------------------------------- |
| Display DATA0               | 18   | RM67162 QSPI                              |
| Display DATA1               | 7    | RM67162 QSPI                              |
| Display DATA2               | 48   | RM67162 QSPI                              |
| Display DATA3               | 5    | RM67162 QSPI                              |
| Display SCLK                | 47   |                                           |
| Display CS                  | 6    |                                           |
| Display RST                 | 17   |                                           |
| Display TE                  | 9    |                                           |
| Display power enable(V2.0)  | 38   | Active high   (V1.0 is GREEN LED)         |
| Touch SDA                   | 3    | Touch version only                        |
| Touch SCL                   | 2    | Touch version only                        |
| Touch IRQ                   | 21   | CST816; touch version only                |
| BOOT button                 | 0    | Active low                                |
| Battery ADC                 | 4    |                                           |
| External SD MISO            | 13   | Library default; no onboard slot          |
| External SD MOSI            | 12   | Library default; no onboard slot          |
| External SD SCLK            | 14   | Library default; no onboard slot          |
| External SD CS              | 11   | Library default; no onboard slot          |
| USB D-                      | 19   | Native USB                                |
| USB D+                      | 20   | Native USB                                |
| UART TX                     | 43   | U0TXD                                     |
| UART RX                     | 44   | U0RXD                                     |

> \[!TIP]
>
> V1.0 GPIO 38 is GREEN LED , V2.0 GPIO38 is Display Power Enable
>

### T4-S3 pinout

| Name                        | GPIO | Note                   |
| --------------------------- | ---- | ---------------------- |
| Display DATA0               | 14   | RM690B0 QSPI           |
| Display DATA1               | 10   | RM690B0 QSPI           |
| Display DATA2               | 16   | RM690B0 QSPI           |
| Display DATA3               | 12   | RM690B0 QSPI           |
| Display SCLK                | 15   |                        |
| Display CS                  | 11   |                        |
| Display RST                 | 13   |                        |
| Display TE                  | 18   |                        |
| Display power enable        | 9    | Active high            |
| Touch SDA                   | 6    | Shared I2C bus         |
| Touch SCL                   | 7    | Shared I2C bus         |
| Touch IRQ                   | 8    | CST226SE               |
| Touch RST                   | 17   | CST226SE               |
| PMU SDA                     | 6    | Shared I2C bus         |
| PMU SCL                     | 7    | Shared I2C bus         |
| PMU IRQ                     | 5    | SY6970                 |
| MicroSD MISO                | 4    | Onboard slot           |
| MicroSD MOSI                | 2    | Onboard slot           |
| MicroSD SCLK                | 3    | Onboard slot           |
| MicroSD CS                  | 1    | Onboard slot           |
| BOOT button                 | 0    | Active low             |
| USB D-                      | 19   | Native USB             |
| USB D+                      | 20   | Native USB             |
| UART TX                     | 43   | U0TXD                  |
| UART RX                     | 44   | U0RXD                  |

### T-Display-S3 AMOLED Plus pinout

| Name                        | GPIO | Note                                  |
| --------------------------- | ---- | ------------------------------------- |
| Display MOSI                | 18   | RM67162 SPI                           |
| Display DC                  | 7    |                                       |
| Display SCLK                | 47   |                                       |
| Display CS                  | 6    |                                       |
| Display RST                 | 17   |                                       |
| Display TE                  | 9    |                                       |
| Display power enable        | 38   | Active high                           |
| Touch SDA                   | 3    | Shared I2C bus                        |
| Touch SCL                   | 2    | Shared I2C bus                        |
| Touch IRQ                   | 21   | CST816                                |
| PMU SDA                     | 3    | Shared I2C bus                        |
| PMU SCL                     | 2    | Shared I2C bus                        |
| PMU IRQ                     | 1    | SY6970 or BQ25896, depending on board |
| RTC SDA                     | 3    | PCF85063; shared I2C bus              |
| RTC SCL                     | 2    | PCF85063; shared I2C bus              |
| MicroSD MISO                | 13   | Onboard slot                          |
| MicroSD MOSI                | 12   | Onboard slot                          |
| MicroSD SCLK                | 14   | Onboard slot                          |
| MicroSD CS                  | 11   | Onboard slot                          |
| BOOT button                 | 0    | Active low                            |
| Battery ADC                 | 4    |                                       |
| USB D-                      | 19   | Native USB                            |
| USB D+                      | 20   | Native USB                            |
| UART TX                     | 43   | U0TXD                                 |
| UART RX                     | 44   | U0RXD                                 |

## 2️⃣Examples

```txt
examples/
├── Factory                             # Factory example
├── AdjustBrightness                    # Adjust brightness example
├── DS18x20_Temperature                 # OneWire example
├── LumenMeter                          # 1.47-inch AMOLED Built-in Ambient Light Sensor Example
├── LVGL_SD_Images                      # Display pictures saved in SD card
├── Lvgl_Images                         # Screen color test
├── PMU_ADC                             # PMU Voltage detection example , only support 1.47-inch AMOLED
├── PMU_Interrupt                       # PMU interrupt example , only support 1.47-inch AMOLED
├── PPM_Example_for_T4S3                # Power chip example, only used for T4-S3
├── PPM_Example_for_191Plus             # Power chip example, only used for AMOLED 1.91 Plus
├── CameraShield                        # Extern camera shield example, only supports 1.91 inches
├── SPI_SDCard                          # External SD card example, SPI communication
├── QWIIC_GPS_Shield                    # Extern GPS Shield example,use Serial communication
├── QWIIC_HP303BSensor                  # Extern Temperature Pressure Sensor example,use Wire communication
├── QWIIC_MAX3010X                      # External heart rate module,use Wire communication
├── TFT_eSPI_Sprite                     # Use TFT_eSPI Sprite example
├── TFT_eSPI_Sprite_ArcFill             # Use TFT_eSPI Sprite example
├── TFT_eSPI_Sprite_graphicstest_small  # Use TFT_eSPI Sprite example
├── TFT_eSPI_Sprite_RLE_Font            # Use TFT_eSPI Sprite example
├── TFT_eSPI_Sprite_Rotation            # Screen rotation, only available for 1.91, 2.41 inches
├── LVGL_Rotation                       # Screen rotation, only available for 1.91, 2.41 inches
├── Touchpad                            # Touchpad example
├── TouchPaint                          # TouchPaint example
├── USB_Host_Keyboard_Mouse             # USB host example
├── lvgl_version_9                      # Lvgl version 9 example
└── lvgl                                # LVGL examples
    ├── event
    ├── get_started
    ├── layouts
    │   ├── flex
    │   └── grid
    ├── scroll
    ├── style
    ├── third_party
    │   ├── ImageDecoder
    │   └── gif
    └── widgets
        └── ...
```

## 3️⃣ PlatformIO Quick Start (Recommended)

1. Install [Visual Studio Code](https://code.visualstudio.com/) and [Python](https://www.python.org/)
2. Search for the `PlatformIO` plugin in the `VisualStudioCode` extension and install it.
3. After the installation is complete, you need to restart `VisualStudioCode`
4. After restarting `VisualStudioCode`, select `File` in the upper left corner of `VisualStudioCode` -> `Open Folder` -> select the `LilyGO AMOLED Series` directory
5. Wait for the installation of third-party dependent libraries to complete
6. Click on the `platformio.ini` file, and in the `platformio` column
7. Uncomment one of the lines `src_dir = xxxx` to make sure only one line works
8. Click the (✔) symbol in the lower left corner to compile
9. Connect the board to the computer USB
10. Click (→) to upload firmware
11. Click (plug symbol) to monitor serial output
12. If it cannot be written, or the USB device keeps flashing, please check the **FAQ** below

## 4️⃣ Install from Arduino Library Manager (recommended)

> \[!IMPORTANT]
>
> Please use the recommended versions of the dependency libraries; newer versions may have compatibility issues.
>
> SensorLib -> v0.2.4
>

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Arduino ESP32 V2.0.16 or later or latest](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
3. `Sketch` -> `Include Library` -> `Manage Libraries`
4. `Library Search` -> `LilyGO AMOLED Series` -> `Install` -> `Install ALL`
5. `Library Search` -> `lvgl` -> `v8.4.0` -> `Install` , How to use `lvgl 9.x.x`, please see the lvgl upgrade settings below
6. `Library Search` -> `SensorLib` -> `v0.2.4` -> `Install`
7. `File` -> `Examples` -> `LilyGO AMOLED Series` -> `Any Examples`
8. `Tools` , Look at the picture to choose

   | Arduino IDE Setting                  | Value                             |
   | ------------------------------------ | --------------------------------- |
   | Board                                | **ESP32S3 Dev Module**            |
   | Port                                 | Your port                         |
   | USB CDC On Boot                      | Enable                            |
   | CPU Frequency                        | 240MHZ(WiFi)                      |
   | Core Debug Level                     | None                              |
   | USB DFU On Boot                      | Disable                           |
   | Erase All Flash Before Sketch Upload | Disable                           |
   | Events Run On                        | Core1                             |
   | Flash Mode                           | QIO 80MHZ                         |
   | Flash Size                           | **16MB(128Mb)**                   |
   | Arduino Runs On                      | Core1                             |
   | USB Firmware MSC On Boot             | Disable                           |
   | Partition Scheme                     | **16M Flash(3M APP/9.9MB FATFS)** |
   | PSRAM                                | **OPI PSRAM**                     |
   | Upload Mode                          | **UART0/Hardware CDC**            |
   | Upload Speed                         | 921600                            |
   | USB Mode                             | **CDC and JTAG**                  |

   - The options in bold are required, others are selected according to actual conditions.

9. Select `Port` ，Port keeps flashing? Please see the following tips
10. Click `upload` , Wait for compilation and writing to complete
11. If it cannot be written, or the USB device keeps flashing, please check the **FAQ** below

## 📽️ [Install from the Arduino IDE library manager](https://youtu.be/FOGPq-WfDkg)

> \[!IMPORTANT]
>
> 🤖 USB ports keep popping in and out?
> Follow the steps below to put your device into download mode
>
>  1. Connect the board via the USB cable
>  2. Press and hold the **BOOT** button , While still pressing the **BOOT** button
>  3. Release the **RST** button
>  4. Release the **BOOT** button
>  5. The USB port should be fixed and will not flash again. You can click Upload.
>  6. Press the **RST** button to exit download mode


## 5️⃣ Arduino IDE Manual installation (not recommended)

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Arduino ESP32 V2.0.16 or later or latest](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
3. Download `LilyGO AMOLED Series` , move to Arduino library folder (e.g. C:\Users\YourName\Documents\Arduino\libraries)
4. Copy all folders in [libdeps folder](./libdeps/) to Arduino library folder (e.g. C:\Users\YourName\Documents\Arduino\libraries)
5. Open ArduinoIDE ,`Tools` , Make your selection according to the table below
   | Arduino IDE Setting                  | Value                             |
   | ------------------------------------ | --------------------------------- |
   | Board                                | **ESP32S3 Dev Module**            |
   | Port                                 | Your port                         |
   | USB CDC On Boot                      | Enable                            |
   | CPU Frequency                        | 240MHZ(WiFi)                      |
   | Core Debug Level                     | None                              |
   | USB DFU On Boot                      | Disable                           |
   | Erase All Flash Before Sketch Upload | Disable                           |
   | Events Run On                        | Core1                             |
   | Flash Mode                           | QIO 80MHZ                         |
   | Flash Size                           | **16MB(128Mb)**                   |
   | Arduino Runs On                      | Core1                             |
   | USB Firmware MSC On Boot             | Disable                           |
   | Partition Scheme                     | **16M Flash(3M APP/9.9MB FATFS)** |
   | PSRAM                                | **OPI PSRAM**                     |
   | Upload Mode                          | **UART0/Hardware CDC**            |
   | Upload Speed                         | 921600                            |
   | USB Mode                             | **CDC and JTAG**                  |

   - The options in bold are required, others are selected according to actual conditions.

6. `File` -> `Examples` -> `LilyGO AMOLED Series` -> `Any Examples`
7. Select `Port` ，Port keeps flashing? Please see the following tips
8. Click `upload` , Wait for compilation and writing to complete
9. If it cannot be written, or the USB device keeps flashing, please check the **FAQ** below

## 📽️ [Arduino IDE Manual installation Video steps](https://youtu.be/WQIH7FD0yuU)

# 6️⃣ ESP32 basic examples

- [BLE Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
- [WiFi Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
- [SPIFFS Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS)
- [FFat Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/FFat)
- For more examples of esp32 chip functions, please refer to [arduino-esp32-libraries](https://github.com/espressif/arduino-esp32/tree/master/libraries)

# 7️⃣ FAQ

1. The board uses USB as the JTAG upload port. When printing serial port information on USB_CDC_ON_BOOT configuration needs to be turned on.
   If the port cannot be found when uploading the program or the USB has been used for other functions, the port does not appear.
   Please enter the upload mode manually.

   1. Connect the board via the USB cable
   2. Press and hold the **BOOT** button , While still pressing the **BOOT** button
   3. Release the **RST** button
   4. Release the **BOOT** button
   5. The USB port should be fixed and will not flash again. You can click Upload.
   6. Press the **RST** button to exit download mode

2. If the above is invalid, burn the [binary file](./firmware/README.MD) to check whether the hardware is normal
3. **1.47 inches AMOLED(T-Display-AMOLED-Lite)** does not support hardware screen rotation function
4. If you use external power supply instead of USBC, please turn off the CDC option. This is because the board will wait for USB access when it starts.

   - For Arduino IDE users, it can be turned off in the options , Please note that turning off USB CDC will turn off Serial redirection to USBC. At this time, you will not see any Serial message output when opening the port from USBC, but output from GPIO43 and GPIO44.

   ```c
   Tools -> USB CDC On Boot -> Disable
   ```

   - For Platformio users, you can add the following compilation flags in the ini file

   ```c
   build_flags =
       ; Enable -DARDUINO_USB_CDC_ON_BOOT will start printing and wait for terminal access during startup
       ; -DARDUINO_USB_CDC_ON_BOOT=1

       ; Enable -UARDUINO_USB_CDC_ON_BOOT will turn off printing and will not block when using the battery
       -UARDUINO_USB_CDC_ON_BOOT
   ```
5. How to find the location of your own libraries, [please see here](https://support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer)
     - Windows: `C:\Users\{username}\Documents\Arduino`
     - macOS: `/Users/{username}/Documents/Arduino`
     - Linux: `/home/{username}/Arduino`

# 8️⃣ Resource

| Product(PinMap)               | schematic                                              | Dimensions                                        | PCB 3D                                                  | Shell                                  |
| ----------------------------- | ------------------------------------------------------ | ------------------------------------------------- | ------------------------------------------------------- | -------------------------------------- |
| [T-Display-AMOLED-Lite][1]    | [schematic](./schematic/T-Display_AMOLED-Lite.pdf)     | [DWG](./dimensions/T-Display-AMOLED-Lite.DWG)     | [STP](./dimensions/T-Display-AMOLED-Lite-PCB-3D.7z)     | x                                      |
| [T-Display-S3 AMOLED][2]      | [schematic](./schematic/T-Display-S3-AMOLED-Touch.pdf) | [DWG](./dimensions/T-Display-S3-AMOLED-Touch.DWG) | [STP](./dimensions/T-Display-S3-AMOLED-Touch-PCB-3D.7z) | [STP](./shell/T-Display-S3-AMOLED.zip) |
| [T4-S3][3]                    | [schematic](./schematic/T-Display-S3-AMOLED-Touch.pdf) | [DWG](./dimensions/T4-S3-AMOLED.dxf)              | [STP](./dimensions/T4-S3-AMOLED-PCB.7z)                 | [STP](./shell/T4-S3-AMOLED.zip)        |
| [T-Display-S3 AMOLED Plus][4] | [schematic](./schematic/T-Display-S3-AMOLED-Plus.pdf)  | [DWG](./dimensions/T-Display-S3-AMOLED-Plus.DXF)  | [STP](./dimensions/T-Display-S3-AMOLED-Plus.7Z)         | [STP](./shell/T-Display-S3-AMOLED.zip) |

##### T-Display-AMOLED-Lite datasheet

- [AMOLED Driver datasheet](./datasheet/SH8501B0%20DataSheet.pdf)
- [Touchpad Driver datasheet](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CHSC5816%E8%A7%A6%E6%8E%A7%E8%8A%AF%E7%89%87%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8EV1-20221114.pdf)
- [PMU datasheet](https://github.com/lewisxhe/XPowersLib/blob/master/datasheet/AXP2101_Datasheet_V1.0_en.pdf)
- [Light Sensor datasheet](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CM32181A3OP.pdf)
- [ESP32S3-R8](https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

##### T-Display-S3 AMOLED datasheet

- [ESP32S3-R8](https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [AMOLED Driver datasheet](./datasheet/RM67162%20DataSheet.pdf)
- [CST816 datasheet](./datasheet/CST816_Register.pdf)

##### T4-S3 datasheet

- [ESP32S3-R8](https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [SY6970](./datasheet/SY6970%20Datasheet.pdf)
- [AMOLED Driver datasheet](./datasheet/RM690B0%20DataSheet_V0.2.pdf)
- [CST226 datasheet](<[./libdeps/SensorLib/](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/%E6%B5%B7%E6%A0%8E%E5%88%9B%E8%A7%A6%E6%91%B8%E8%8A%AF%E7%89%87%E7%A7%BB%E6%A4%8D%E6%89%8B%E5%86%8C-v3.5-20220701(1).pdf)>)

# 8️⃣ Depends on required libraries

- [XPowersLib v0.2.7](https://github.com/lewisxhe/XPowersLib)
- [SensorLib v0.2.4](https://github.com/lewisxhe/SensorsLib)
- [lvgl v8.x or v9.x](https://github.com/lvgl/lvgl)
- [AceButton v1.10.1](https://github.com/bxparks/AceButton)
- [Adafruit_NeoPixel v1.11.0](https://github.com/adafruit/Adafruit_NeoPixel)
- [TinyGPSPlus v1.0.3](https://github.com/mikalhart/TinyGPSPlusl)

# 9️⃣. lvgl 9 Upgrade Guide

1. Find `LilyGo-AMOLED-Series` in the library installation directory and enter the `src` directory
2. Rename `lv_conf.h` to `lv_conf.h.v8`
3. Rename `lv_conf.h.v9` to `lv_conf.h`
4. Open `Arduino IDE` -> `File` -> `examples`- > `lvgl_version_9`
5. Currently there is only one example, `lvgl_version_9`, and other examples are based on `lvgl 8`, please note.
