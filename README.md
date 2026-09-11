# SLH Firmware

Firmware for SLH hardware nodes. Currently: ESP32-2432S028 (CYD) crypto ticker.

## Hardware

- ESP32-D0WD-V3, 4MB flash, dual core 240MHz
- 2.8 inch ILI9341 320x240 TFT
- XPT2046 resistive touch
- USB-serial: CH340

## Pinout

MISO 12 | MOSI 13 | SCLK 14 | CS 15 | DC 2 | RST -1 | BL 21 | TOUCH_CS 33

## Setup

1. Copy src/secrets.h.example to src/secrets.h
2. Fill in your WiFi SSID and password
3. Flash: pio run -t upload --upload-port COM4

Serial monitor: pio device monitor --baud 115200

## Features

Ticker: BTC, ETH, BNB live from CoinGecko, Binance fallback.
Last known values stay on screen with an age stamp if both fail.
SLH shows a fixed presale rate.

Demo mode: touch the top header bar. Four pages - touch test,
graphics benchmark, sprite animation with FPS, device info.
Touch the bottom bar to navigate.

Refresh: touch anywhere below the header. Auto-refresh every 60s.

## Notes

- src/secrets.h is gitignored. Never commit WiFi credentials.
- Touch calibration is hardcoded in main.cpp (touchCal).
- White screen or inverted colours: swap ILI9341_2_DRIVER for ILI9341_DRIVER in platformio.ini.
