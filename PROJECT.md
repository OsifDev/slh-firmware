# SLH Firmware - Project Contract

Read this file first. It is the single source of truth for how this
project is built and what the rules are. Update it when facts change.

## Hardware

| | Device 1 | Device 2 |
|---|---|---|
| MAC | 14:33:5C:6C:32:C0 | 14:33:5C:6C:81:40 |
| IP | 10.0.0.4 | 10.0.0.6 |
| key | ESP_14335C6C32C0 | ESP_14335C6C8140 |

Board: ESP32-2432S028 (CYD). ESP32-D0WD-V3, 4MB flash.
Display: ILI9341 320x240 landscape. Touch: XPT2046 on CS=33.
Pinout: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, RST -1, BL 21.
Touch calibration: rtCal = {3204, 740, 3095, 1355} in main.cpp.

## Network reality - READ THIS

The Telegram bot runs on Railway in the cloud. The devices are on
a home LAN at 10.0.0.x. THE CLOUD CANNOT REACH THE DEVICES.
Any bot-to-device control MUST go through a bridge running on PC1,
which is on the same LAN. Do not propose direct cloud-to-device calls.

PC1: hostname Osif2, user USER, 10.0.0.3, project at
C:\VirtualDrive\SLH_ECOSYSTEM (this is a subst of a real folder,
not a physical drive).
PC2: hostname SELHA-MAIN, user SELHA, 10.0.0.2, archive on D: (448GB).
Phone: Termux, SSH key installed on both PCs, no password needed.

## Server reality

slh-api (Railway) exposes only 5 routes:
  /health /api/health /api/connect/wallet /api/holders /api/live-stats
  NOTE: /api/holders returns an error - table connected_wallets missing.

slh-bot (Railway) is a Telegram bot, NOT an HTTP API.
  No FastAPI, no routers/, no include_router.
  Device data lives in state/devices.json on a persistent volume.
  state/ survives restarts. handlers/ does NOT - code written via
  /exec is lost. All bot code changes must go through GitHub + /deploy.

## Build and flash

cd C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware
pio run                                    # build only
.\flash-ota.ps1 -esp 10.0.0.4              # wireless (preferred)
$p=[System.IO.Ports.SerialPort]::GetPortNames()|Select -First 1
pio run -t upload --upload-port $p         # cable fallback

Partition: min_spiffs.csv (1.9MB per app slot, OTA capable).
Do NOT use huge_app.csv - it has no OTA slot and breaks wireless flash.

## Device HTTP API (port 80, on the LAN)

GET /state              full JSON: device, rows, touch, screen
GET /screen             {"current":"prices","all":[...]}
GET /screen?to=market   switch screen
GET /lesson?page=N      show lesson page
GET /backlight?pct=N    brightness 5-100
GET /touch              live touch read, raw + mapped
GET /setcal?x0&x1&y0&y1 update touch calibration (RAM only)
GET /rawdump            10 raw XPT2046 samples
GET /refresh            force price refresh
GET /view               what is on screen now, as text lines
GET /ota                firmware upload form

## Helper scripts (in this folder)

screen.ps1      ASCII render of what is on the device display
market.ps1      trading context table: range, volatility, fees
flash-ota.ps1   build + wireless flash + verify
view.ps1        renders /view - see the device screen from the terminal

## Ground rules - these exist because they were learned the hard way

1. Check before you write. Run Select-String to see if a function
   already exists. Running .Replace() twice creates duplicates that
   fail at link time. This happened repeatedly.

2. One block at a time, with verification. Every edit block must end
   with a Select-String or .Count that proves it worked. Long pasted
   blocks get truncated mid-way in this users terminal.

3. C++ needs forward declarations. If drawAll() calls drawMarket()
   defined later in the file, declare it up top or the build fails.

4. Never put a blocking loop inside an HTTP handler. It kills the
   web server and the device becomes unreachable until reboot.
   tft.calibrateTouch() did exactly this.

5. Do not use TFT_eSPI getTouch(). It does not work on this board.
   Use rtTouch() from rawtouch.h - direct XPT2046 with median filter.

6. Binance returns numbers as JSON strings. Use
   String((const char*)(doc["field"] | "0")).toFloat()
   not doc["field"] | 0.0f - the latter silently returns zero.

7. Hebrew in PowerShell here-strings breaks. Use the array pattern:
   $A = @(); $A += 'line'; $A | Set-Content file -Encoding UTF8

8. This user pastes terminal output back. Read it carefully -
   half the bugs are visible in what did NOT print.



9. lesson_bitmaps.h is RLE-encoded, not raw pixels. The decoder in
   lesson.h needs LESSON_SIZES[] emitted by render_lessons.py.
   Run render_lessons.py before any build that touches lesson pages,
   or the header goes stale and the build fails on LESSON_SIZES.

10. Utility scripts that read conventions from code (like /layout)
    must parse the source at runtime, not embed constants. The old
    /layout used y += 66 after the code moved to y += 40 and lied.

## Product rules

This device must NOT tell anyone which direction to trade.
No buy signals, no "enter long" arrows, no direction calls.
Reason: a 30 minute move on BTC is roughly the size of the fees.
A confident arrow on a nice screen teaches students that someone
knows the future. That is how accounts get emptied.

What the device DOES show: day range, where price sits in it,
volatility, fees as a percentage of the available move, and
risk per $1000 at different stop sizes. The student decides.

The SLH row shows a fixed presale rate with a PRESALE badge.
It is not a market price and must not be presented as one.

## Current state

Flash at 60.6% (1,191,449 / 1,966,080). 774KB free.
Lesson bitmaps are RLE compressed: 704KB raw down to 130KB.
Decoder lives in lesson.h. LESSON_SIZES holds each compressed length.
Re-run render_lessons.py after editing any lesson text.

Known issues:
  - UI/UX is rough. Built incrementally during debugging.
  - No consistent back button or nav bar across screens.
  - Device 2 (10.0.0.6) may still be on an older partition;
    if OTA returns ok:false, flash it by cable once.
  - Wallet screen is a placeholder, not connected to anything.

Not yet built:
  - Bridge on PC1 pulling commands from the bot
  - Real wallet balances on screen
  - Price alerts
  - Backtesting on the CSV the logger is collecting

## Data collection

C:\VirtualDrive\SLH_ECOSYSTEM\price-logger\logger.py runs in the
background via scheduled task SLH-PriceLogger. Writes prices.csv
every 60s: timestamp, symbol, price, change, high, low, volume,
trades. This is the foundation for any future backtesting.
Build strategy from this data, not the other way around.





