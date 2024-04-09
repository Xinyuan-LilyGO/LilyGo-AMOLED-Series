<h1 align = "center">🌟LilyGO AMOLED Series🌟</h1>

[![PlatformIO CI](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/platformio.yml/badge.svg)](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/platformio.yml)
[![Arduino CI](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/arduino_ci.yml/badge.svg)](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/actions/workflows/arduino_ci.yml)
[![arduino-library-badge](https://www.ardu-badge.com/badge/LilyGo-AMOLED-Series.svg?)](https://www.ardu-badge.com/LilyGo-AMOLED-Series)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/xinyuan-lilygo/library/LilyGo-AMOLED-Series.svg)](https://registry.platformio.org/libraries/xinyuan-lilygo/LilyGo-AMOLED-Series)

<!-- **English | [中文](README_CN.MD)** -->

## News

- `LilyGo-AMOLED-Series` depends on lvgl **v8.3.x** version. Currently lvgl has released lvgl **v9.0** version. When compilation error occurs, please check whether lvgl belongs to **v8.3.x**
- `LilyGo-AMOLED-Series` esp-idf version example, please jump to this [LilyGo-Display-IDF](https://github.com/Xinyuan-LilyGO/LilyGo-Display-IDF)

## 1️⃣Support Product

| Product(PinMap)            | SOC        | Flash | PSRAM    | Resolution | Size      |
| -------------------------- | ---------- | ----- | -------- | ---------- | --------- |
| [T-Display-AMOLED-Lite][1] | ESP32-S3R8 | 16MB  | 8MB(OPI) | 194x368    | 1.47 Inch |
| [T-Display-S3 AMOLED][2]   | ESP32-S3R8 | 16MB  | 8MB(OPI) | 240x536    | 1.91 Inch |
| [T4-S3][3]                 | ESP32-S3R8 | 16MB  | 8MB(OPI) | 450x600    | 2.41 Inch |

| Current consumption        | Working current             | sleep current | sleep mode      |
| -------------------------- | --------------------------- | ------------- | --------------- |
| [T-Display-AMOLED-Lite][1] | (240MHz) WiFi On 90~230+ mA | About 1.1mA   | timer wakeup    |
| [T-Display-S3 AMOLED][2]   | (240MHz) WiFi On 90~230+ mA | About 160uA   | external wakeup |
| [T4-S3][3]                 | (240MHz) WiFi On 90~230+ mA | About 160uA   | external wakeup |

[1]: https://www.lilygo.cc/products/t-display-amoled-lite
[2]: https://www.lilygo.cc/products/t-display-s3-amoled
[3]: https://www.lilygo.cc/products/t4-s3

- This code supports T-Display-S3 AMOLED both touch and non-touch versions

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

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Arduino ESP32 V 2.0.5 or above and below V3.0](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
3. `Sketch` -> `Include Library` -> `Manage Libraries`
4. `Library Search` -> `LilyGO AMOLED Series` -> `Install` -> `Install ALL`
5. `Library Search` -> `lvgl` -> `v8.3.11` -> `Install`
6. `File` -> `Examples` -> `LilyGO AMOLED Series` -> `Any Examples`
7. `Tools` , Look at the picture to choose
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

8. Select `Port`
9. Click `upload` , Wait for compilation and writing to complete
10. If it cannot be written, or the USB device keeps flashing, please check the **FAQ** below

## 5️⃣ Arduino IDE Manual installation (not recommended)

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Arduino ESP32 V 2.0.5 or above and below V3.0](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
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
7. Select `Port`
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
   2. Press and hold the BOOT button , While still pressing the BOOT button, press RST
   3. Release the RST
   4. Release the BOOT button
   5. Upload sketch

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

| Product(PinMap)            | schematic                                                | Dimensions                                        | PCB 3D                                                  | Shell                                  |
| -------------------------- | -------------------------------------------------------- | ------------------------------------------------- | ------------------------------------------------------- | -------------------------------------- |
| [T-Display-AMOLED-Lite][1] | [schematic](./schematic/T-Display_AMOLED-Lite.pdf)       | [DWG](./dimensions/T-Display-AMOLED-Lite.DWG)     | [STP](./dimensions/T-Display-AMOLED-Lite-PCB-3D.7z)     | x                                      |
| [T-Display-S3 AMOLED][2]   | [schematic](./schematic/T-Display-S3-AMOLED-Touch.pdf)   | [DWG](./dimensions/T-Display-S3-AMOLED-Touch.DWG) | [STP](./dimensions/T-Display-S3-AMOLED-Touch-PCB-3D.7z) | [STP](./shell/T-Display-S3-AMOLED.zip) |
| [T4-S3][3]                 | [schematic](./schematic/T-Display-S3%20AMOLED-Touch.pdf) | [DWG](./dimensions/T4-S3-AMOLED.DWG)              | [STP](./dimensions/T4-S3-AMOLED-PCB-3D.7z)              | [STP](./shell/T4-S3-AMOLED.zip)        |

##### T-Display-AMOLED-Lite datasheet

- [AMOLED Driver datasheet](./datasheet/SH8501B0%20DataSheet.pdf)
- [Touchpad Driver datasheet](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CHSC5816%E8%A7%A6%E6%8E%A7%E8%8A%AF%E7%89%87%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8EV1-20221114.pdf)
- [PMU datasheet](https://github.com/lewisxhe/XPowersLib/blob/master/datasheet/AXP2101_Datasheet_V1.0_en.pdf)
- [Light Sensor datasheet](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CM32181A3OP.pdf)
- [ESP32S3-R8](https://www.espressif.com.cn/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=842)

##### T-Display-S3 AMOLED datasheet

- [ESP32S3-R8](https://www.espressif.com.cn/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=842)
- [AMOLED Driver datasheet](./datasheet/RM67162%20DataSheet.pdf)
- [CST816 datasheet](<[./libdeps/SensorLib/](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/%E6%B5%B7%E6%A0%8E%E5%88%9B%E8%A7%A6%E6%91%B8%E8%8A%AF%E7%89%87%E7%A7%BB%E6%A4%8D%E6%89%8B%E5%86%8C-v3.5-20220701(1).pdf)>)

##### T4-S3 datasheet

- [ESP32S3-R8](https://www.espressif.com.cn/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=842)
- [SY6970](./datasheet/SY6970%20Datasheet.pdf)
- [AMOLED Driver datasheet](./datasheet/RM690B0%20DataSheet_V0.2.pdf)
- [CST226 datasheet](<[./libdeps/SensorLib/](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/%E6%B5%B7%E6%A0%8E%E5%88%9B%E8%A7%A6%E6%91%B8%E8%8A%AF%E7%89%87%E7%A7%BB%E6%A4%8D%E6%89%8B%E5%86%8C-v3.5-20220701(1).pdf)>)

# 8️⃣ Depends on required libraries

- [XPowersLib](https://github.com/lewisxhe/XPowersLib)
- [SensorLib](https://github.com/lewisxhe/SensorsLib)
- [lvgl 8.3.11](https://github.com/lvgl/lvgl)
- [AceButton](https://github.com/bxparks/AceButton)
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
- [TinyGPSPlusl](https://github.com/mikalhart/TinyGPSPlusl)
