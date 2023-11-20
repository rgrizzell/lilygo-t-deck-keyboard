# LILYGO T-Deck Keyboard Firmware
Firmware for the LILYGO T-Deck Keyboard running on an ESP32-C3.

## Features
- **Keypress Buffer** - Key presses are stored for the host to retrieve.
- **I2C Commands** - Capable of receiving commands from the host.

## Hotkeys
| Action           | Hotkey  |
| ---------------- | ------- |
| Toggle Backlight | Alt + B |
| Clear Screen*    | Alt + L |


\* Requires OS support

## Flashing

Flashing requires a **USB-to-TTL adapter**.

The pins being connected to are slightly undersized from a standard debug header. The preferred method of connecting is using probes/test clips. However, male headers removed from the housing will work too, just ensure they don't touch each other. The LILYGO T-Deck includes a 6-pin header that can be soldered to the board, but that may be unnecessarily risky.

It is also necessary to short the boot and reset pins to ground in order to get the keyboard ready for flashing.

### PlatformIO
This project utilizes PlatformIO for its ease-of-use. Building via `CMake` will be added at a later date.

Place the keyboard into Download Mode by pressing both the BOOT and RESET buttons, then while still holding BOOT, releasing RESET. Then releasing BOOT. If successful, the keyboard backlight should turn on and stay on. Alternatively, you can connect to the UART interface and verify the output. **Remember to disconnect any terminals from the UART before flashing.**

```
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x1 (POWERON),boot:0x5 (DOWNLOAD(USB/UART0/1))
waiting for download
```

Flash the board. If you have PlatformIO Core (CLI) installed, you can run the following:

```shell
pio run -t upload
```

