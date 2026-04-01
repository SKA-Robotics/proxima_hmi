<p align="center"><img align="center" src="./docs/logo.svg"/><h1 align="center">Proxima HMI</h1><p align="center">
Software for the <b>Proxima</b> rover HMI (human-machine interface), powered by the <b>Raspberry Pi Pico 2</b></p>

## Hardware
The HMI is based on the Waveshare 30706 development board, which features the **RP2350** microcontroller (same as in Pico 2) along with:
- 2.8 inch touch screen
- integrated RTC and IMU
- audio output with speakers
- TF card slot (microSD)

## Project Structure
- `src/hmi` - main HMI application
- `src/common` - shared static library
- `dist` - output for `.uf2` binaries

## Editing
1. Open this folder in [Visual Studio Code](https://code.visualstudio.com/).
2. Install the **Raspberry Pi Pico** extension (`raspberry-pi.raspberry-pi-pico`). When prompted, accept the other recommended extensions (C/C++, Cortex-Debug, etc.).
3. Let the extension install/configure the **Pico SDK**, toolchain, and tools under `~/.pico-sdk` (or `%USERPROFILE%\.pico-sdk` on Windows) if you do not already have them.

## Build
- **From VS Code:** run the **Compile Project** task provided by Pico extension

## How to flash
1. Hold **BOOTSEL** while connecting USB or hold **BOOTSEL** and press **RESET** button so it appears as a USB drive
2. Copy **`dist/hmi.uf2`** onto that drive. The Pico reboots and runs the new firmware