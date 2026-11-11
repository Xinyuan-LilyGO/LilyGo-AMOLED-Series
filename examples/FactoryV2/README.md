# FactoryV2

The example is split into three layers:

- `FactoryApp.*`: board detection, SensorLib hardware access, periodic sampling, and action dispatch.
- `FactoryUi.*`: LVGL view and event forwarding. It consumes `FactorySnapshot` and never accesses board drivers.
- `FactoryData.h` / `FactoryExtension.h`: fixed-size data contracts and expansion interfaces.

The UI is an app-style LVGL launcher. In landscape mode the first page is a
compact, horizontally scrollable strip of flat color icons; in portrait mode
it becomes a vertically scrollable column. Tapping an icon opens that
function's dedicated test page. Function pages are click-entered and do not
participate in accidental page swipes. The touch-controller Home button always
returns to the launcher (and closes a full-screen display test if one is active).
The `Rotate` icon is a direct action: it changes the panel orientation in place
and deliberately does not open a separate page.
Each available hardware function has its own icon and test page; unavailable
hardware is omitted from the launcher.
Display, SD, RGB, touch, power, audio, microphone, LoRa TX/RX, IR TX/RX,
sensors, telemetry, and extensions are kept as separate pages.

The Audio page provides short synthesized test tones and a separate embedded
MP3 playback test. `audio/VanLifeMp3.h` contains the 96.7-second test track as
a 773,642-byte PROGMEM array. It is encoded as 44.1 kHz mono MP3 at 64 kbps so
the ES8311 sample rate remains unchanged while flash use is kept below one
quarter of the original 3,093,733-byte stereo file. Playback is decoded in the
main loop, and the Shield power amplifier is switched off when playback ends.

On boards with both SD storage and the AMOLED Plus Shield codec, the separate
SD Music app scans MP3 files directly inside `/` and `/music`. Scanning handles
one directory entry per application update and keeps the first 24 matching
files, so large directories do not block the UI. Tapping a row streams that
file from SD through ESP8266Audio. Returning Home or opening another app stops
the decoder, closes the file, and switches off the external amplifier.
The audio output synchronizes the ES8311 clock dividers with each stream's
decoded sample rate; microphone capture restores both sides to 44.1 kHz.
Playback uses a larger DMA queue and refills it on both sides of LVGL rendering
so display updates cannot stretch the audio with repeated buffer underruns.
The Audio page volume slider writes the ES8311 hardware volume directly; I2S
software gain remains fixed at unity.
The Audio page exposes only the side-by-side MP3 and beep tests. ES8311 volume
is capped at 54 in both the slider and hardware boundary to avoid DAC clipping
with full-scale program material.
Microphone sampling updates the waveform, level, and fault state in the UI
without continuous serial diagnostics, so an unopened serial port cannot delay
the real-time audio path.
The physical BOOT button is a global Home control. Its press edge responds
immediately, while debounce is applied only to release before accepting the
next press. It closes overlays, stops the active peripheral test, and returns
directly to the app menu; there is no separate Button test app.

The asset can be regenerated with the portable FFmpeg installed outside the
repository:

```bash
/mnt/code-server/bin/ffmpeg -i /mnt/code-server/projects/lilygo/van-life.mp3 \
  -map_metadata -1 -vn -ac 1 -ar 44100 -c:a libmp3lame -b:a 64k \
  -write_xing 0 -id3v2_version 0 /mnt/code-server/van-life-mono.mp3
xxd -i /mnt/code-server/van-life-mono.mp3
```

The extension contract is intended for future audio, LoRa, and sensor tests.
Each extension owns its hardware and exposes only an identifier, presentation
metadata, a status update, and an optional action handler:

```cpp
class MySensorExtension : public FactoryExtension {
public:
    const char *id() const override { return "temperature"; }
    const char *title() const override { return "Temperature"; }
    const char *icon() const override { return LV_SYMBOL_CHARGE; }
    bool begin(LilyGo_Class &, const BoardsConfigure_t *) override { return true; }
    void update(FactoryExtensionStatus &status) override {
        status.available = true;
        snprintf(status.value, sizeof(status.value), "%.1f C", readTemperature());
    }
    void action(uint8_t action) override {
        if (action == 1) runSelfTest();
    }
};
```

`id` is a stable machine identifier. `title` and `icon` are presentation
metadata. `FactoryExtensionStatus::value` is refreshed by `update()`, while
`FactoryActions::extensionAction` routes card presses back to the owning
extension. The application preserves the descriptor fields around each
`update()` call, so an implementation may replace the runtime state without
rebuilding its metadata. The fixed slot count (`FACTORY_MAX_EXTENSIONS`) keeps
the snapshot deterministic and avoids dynamic allocation in the periodic LVGL
update path.

Icons use LVGL's `LV_SYMBOL_*` Font Awesome glyphs. They are the open-source
symbol assets shipped in LVGL for extension metadata. The feature selector uses
the colorful SVG artwork from GitHub's `icons8/flat-color-icons` repository
([source](https://github.com/icons8/flat-color-icons)), converted to 40x40
RGB565A8 constants in `FactoryIcons.c`.

The launcher exposes display color cycle, SD, RGB, touch, power,
microphone, LoRa TX/RX, IR TX/RX, sensor, telemetry, and extension entries as
separate pages. Display, SD, RGB, and the passive board status pages are sampled
continuously. On the optional shield, the audio page plays a short RTTTL test
tone, the microphone page reports live I2S level, LoRa TX/RX use non-blocking
RadioLib operations, IR TX sends a NEC test frame once per second while enabled,
IR RX reports decoded frames using the ESP32-S3 RMT peripheral directly, and the
sensor page initializes and samples the QMI8658/QMC6309 pair.
Unavailable hardware is reported by the page without initializing its driver.

FactoryV2 also detects the optional AMOLED Plus Shield by probing I2C address
`0x20` on the board's shared I2C pins. When present, `FactoryShield` initializes
the XL9555 expander, SX1262 through RadioLib 7.8.1, the ES8311/I2S output path
through ESP8266Audio 2.4.1, IR TX/RX, and the QMI8658/QMC6309 SensorLib
drivers. When the address is absent, none of those drivers are initialized and
their UI entries are omitted.

Compass calibration has two stages. Tap `CALIBRATE COMPASS` and rotate the
device through every axis for 15 seconds. When the button changes to
`ALIGN NORTH`, place the device flat, point its top edge in the same direction
as a trusted compass showing North, and tap the button again. The first stage
removes hard-iron offset and compensates unequal X/Y axis ranges; the second
stage aligns the sensor coordinates with the physical device direction.
