# Internet of Light

This Homie V3 standard compitble device provide you a Internet of Things enabled LED lamp.
It can control a dimmable (e.g. white) LED [*via D2*].
Additionally colored LEDs can be added, for now WS2812 LEDs are supported [*via D1*]. The complete RGB LED strip can be controlled, or each lamp individually.

Via an external switch [*via D0*] the white LED can be switched on, or off. The lamp will dim between the states using its PWM functionality. Pressing this switch until all colored LEDs are red, will reset the current stored configuration. (The button can be deactivated in the firmware with ```-D NOBUTTON```)

## Dependency
Homie for ESP8266 (e.g. nodemcu)

See https://github.com/homieiot/homie-esp8266

Development-Setup:
* platformio

## Installation
### Linux
* Build environment (apt install build-essential)
* git (apt install git)
* Visual Studio Code See: https://code.visualstudio.com/docs/?dv=linux64_deb

Search for the following extension in Visual Studio Code:
* PlatformIO

And the following Librarys in Visual Studio Code:
* Neo Pixel (from Adafruit)
 
### Windows
**Not working**

Compiler generates unbootable image.

Workaround:
* Generate Ubuntu VM and follow setup above inside
* copy ```firmware.bin``` from VM to Windows
* Install python & pip
* ```pip3 install esptool```
* ```./esptool.py.exe write_flash 0x0000 firmware.bin```

## Development
* File
 * Open Workspace
  * Select the file in this repository:  WIoLight.code-workspace

## Filesystem
### Configuration
Use the config-example.json from the host folder and create here a config.json file.
### HowTo upload
Start Platform.io
Open a new Atom-Terminal and generate the filesystem with the following command :
```pio run -t buildfs```
Upload this new generated filesystem with:
```pio run -t uploadfs```

# Hardware
* D1: WS2812 RGB LEDs
* D0: Button
* D2: N-Channel Mosfet (e.g. LL2705)

## Optional Temperatur

* D7: One-Wire temperature sensor (DS 18B20)

Must be activated in the firmware with **TEMP_ENABLE**

## Optional Motion
* D6: IR PIR motion sensor (needs pull-down resistor)

Must be activated in the firmware with **PIR_ENABLE**
