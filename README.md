# LILYGO T-Deck Keyboard Firmware
Firmware for the LILYGO T-Deck Keyboard running on an ESP32-C3.

## Features
- **Keypress Buffer** - Key presses are stored for the host to retrieve.
- **I2C Commands** - Capable of receiving commands from the host.
- **Log Output** - More information is printed to UART.

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

## I2C
Writing an I2C Slave for a microcontroller is a challenging task. The I2C Master controls communication and may request data at any time. The slave must be ready to respond to these requests even if it is busy. In most I2C devices, this is achieved with a hardware state machine, since a microcontroller may not be fast enough. For the T-Keyboard, a compromise is achieved by not responding to any write requests. The master should not wait for a respond when writing to the device. The slave will process the command and continue operating.

If writing to and then reading from the device is required, it may be necessary to introduce an artificial delay on the master between writing and then reading from the slave device. The firmware must also be patched to store any data until the master retrieves it.

For more details on the I2C Slaves: https://www.i2c-bus.org/slave/

| Address | Description   | Data       |
| ------- | ------------- | ---------- |
| 0x00    | Get key press | N/A        |
| 0x01    | Set backlight | Off=0 On=1 |

### Key Presses
Key press events are retrieved by reading from the device at any address.

```cpp
char key = (char)Wire.requestfrom(0x55, 1);
```
```
EVENT: l
INFO: I2C Transmit: data[1]=l
```

### Backlight
The backlight can be toggled by writing to the address without any data.

```cpp
byte val = 1;
Wire.beginTransmission((int)0x55);
Wire.write(val);
Wire.endTransmission();

delay(500);
Wire.beginTransmission((int)0x55);
Wire.write(val);
Wire.endTransmission();
```
```
INFO: I2C Receive: addr=1 data[0]=
LIGHT: 1
INFO: I2C Receive: addr=1 data[0]=
LIGHT: 0
```

```cpp
byte val[2];
val[0] = 1;
val[1] = 1;
Wire.beginTransmission((int)0x55);
Wire.write(val)
Wire.endTransmission();

val[1] = 0;
Wire.beginTransmission((int)0x55);
Wire.write(val)
Wire.endTransmission();
```
```
INFO: I2C Receive: addr=1 data[1]=1
LIGHT: 1
INFO: I2C Receive: addr=1 data[1]=0
LIGHT: 0
```