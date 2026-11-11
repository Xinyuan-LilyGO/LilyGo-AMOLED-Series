# Audio Codec (WM8960)

### Overview

This library provides functions to support the WM8960 audio codec. The Codec is connected via __I2S__ for the audio data and is configured via __I2C__.

It can drive some speakers via the amplifier output and supports full duplex mode where we can both read and write audio data at the same time.

This library is based on the [original provided by Infineon](https://github.com/Infineon/audio-codec-wm8960) and has been adapted for Arduino to use the Wire API.

Though this library can be used stand alone, I recommend to use it with the WM8960Stream class provided by the [AudioTools](https://github.com/pschatzmann/arduino-audio-tool) project.

### Documentation

- [API](https://pschatzmann.github.io/arduino-wm8960/docs/html/group__wm8960.html)
- [Boards](https://github.com/pschatzmann/arduino-wm8960/wiki/Pins-Connections)


### Installation in Arduino

You can download the library as zip and call include Library -> zip library. Or you can git clone this project into the Arduino libraries folder e.g. with

```
cd  ~/Documents/Arduino/libraries
git clone pschatzmann/arduino-wm8960.git
```

I recommend to use git because you can easily update to the latest version just by executing the ```git pull``` command in the project folder.


### Desktop Installation

You can use this library also outside of Arduino with cmake. In this case you need to provide the implementation for the i2c_write function yourself! 