# Wiimote Audio on the Wii U Example

Add an ogg vorbis file to the root of the sdcard and name it `sample.ogg`.

Connect a Wiimote, and run the app.

## Building

Install [devkitPro](https://devkitpro.org/wiki/Getting_Started)
```
/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake -S . -B wiiu-build

/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake --build wiiu-build
```
