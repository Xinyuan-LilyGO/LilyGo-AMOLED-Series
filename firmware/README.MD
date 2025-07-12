
## News

* The firmware supports any one in the list below. The firmware used in the list below is the same and there is no need to distinguish it.
* debug.bin outputs all debug messages from USBCDC
* release.bin outputs all debug information from Qwiic UART

## 1️⃣Support Product

| Product(PinMap)               | Release                   | Debug                   |
| ----------------------------- | ------------------------- | ----------------------- |
| [T-Display-AMOLED-Lite][1]    | [firmware](./release.bin) | [firmware](./debug.bin) |
| [T-Display-S3 AMOLED][2]      | [firmware](./release.bin) | [firmware](./debug.bin) |
| [T4-S3][3]                    | [firmware](./release.bin) | [firmware](./debug.bin) |
| [T-Display-S3 AMOLED Plus][4] | [firmware](./release.bin) | [firmware](./debug.bin) |

[1]: https://www.lilygo.cc/products/t-display-amoled-lite
[2]: https://www.lilygo.cc/products/t-display-s3-amoled
[3]: https://www.lilygo.cc/products/t4-s3
[4]: https://www.lilygo.cc/products/t-display-s3-amoled


## 2️⃣How to Flash ?


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


### Use ESP Download Tool

- Download [Flash_download_tool](https://www.espressif.com.cn/sites/default/files/tools/flash_download_tool_3.9.5_0.zip)

![web_flasher](./images/esp_downloader.gif)

* Note that after writing is completed, you need to press RST to reset.

### Use Web Flasher

- [ESP Web Flasher Online](https://espressif.github.io/esptool-js/)

![web_flasher](./images/web_flasher.gif)

* Note that after writing is completed, you need to press RST to reset.

### Use command line


If system asks about install Developer Tools, do it.

```
python3 -m pip install --upgrade pip
python3 -m pip install esptool
```

In order to launch esptool.py, exec directly with this:

```
python3 -m esptool
```

For ESP32-S3 use the following command to write

```
esptool --chip esp32s3  --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m 0x0 firmware.bin

```

# 2️⃣FAQ

- Can't upload any sketch，Please enter the upload mode manually.
   1. Connect the board via the USB cable
   2. Press and hold the **BOOT** button , While still pressing the **BOOT** button (If there is no **BOOT** button, you need to use wires to connect **GND** and **IO0** together.)
   3. Press **RST** button
   4. Release the **RST** button
   5. Release the **BOOT** button (If there is no **BOOT** button, disconnect IO0 from GND.)
   6. Upload sketch
   7. Press the **RST** button to exit download mode

- How do I prove to LilyGo that the board is damaged?
  1. You need to shoot a video to prove that the board is damaged. Follow the steps below to shoot a video to prove that the board is damaged.
  2. Follow the first step of the FAQ on this page to put the board into download mode
  3. Select a burning tool, download **debug.bin** and upload the firmware to the board through the burning tool
  4. After waiting for the burning to complete, press the RST button, or disconnect the USB and then reinsert it.
  5. Any of the above steps must be included in the video content for clarification