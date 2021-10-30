# Io Light
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
To be done

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
* D6: IR PIR motion sensor
* D2: N-Channel Mosfet (e.g. LL2705)

