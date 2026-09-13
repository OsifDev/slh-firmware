# SLH Firmware - Session Handoff

Updated: 2026-09-12
Read PROJECT.md first for hardware facts and ground rules.
This file is the live state: what works, what is broken, what is next.

## Devices

| | Device 1 | Device 2 |
|---|---|---|
| owner | OSIF | ZVIKA |
| MAC | 14:33:5C:6C:32:C0 | 14:33:5C:6C:81:40 |
| IP | 10.0.0.4 | 10.0.0.6 |
| key | ESP_14335C6C32C0 | ESP_14335C6C8140 |

Device 2 needs OWNER_NAME changed to "ZVIKA" in src/secrets.h before
flashing it. secrets.h is gitignored so each unit keeps its own.

## What works

- BTC/ETH/BNB live from CoinGecko, Binance fallback, last-known on failure
- SLH row shows fixed ILS 444 with a PRESALE badge
- 24h sparklines from Binance klines
- 6 screens: home, prices, market, setup, show, lesson
- 11 Hebrew lesson pages, RLE compressed (704KB raw to 130KB)
- Neural network boot animation (16 nodes, sprite based)
- OTA over WiFi with progress bar on the device screen
- HTTP API for full remote control and readout
- Flash 61.2%, about 740KB free

## What is broken

TOUCH. Resolved 2026-09-13: the code and wiring are correct. Board 2 (ZVIKA, MAC 81:40) reads touch perfectly with z 500-1500. Board 1 (OSIF, MAC 32:C0) returns all zeros - hardware fault, likely damaged during a scanner that wrote to input-only GPIOs. Same firmware, opposite result. Do not debug the driver.

Facts established:
  - XPT2046 on CS=33, SPI on SCK 25 / MISO 39 / MOSI 32
  - Early in the day a standalone scanner returned z=731 x=1919 y=1986
  - Later the SAME code from commit cf01502 returned all zeros
  - 5 different pin combinations, both boards, all zeros
  - A raw SPI read with no filtering, no latch, no TFT also returns zeros
  - The user did see a calibration screen and a touch response on cf01502

What was tried and did NOT fix it:
  - restoring the z-pressure gate (rz was hardcoded to 0)
  - adding and then removing a touch latch
  - time-based latch release instead of failure-based
  - tft.endWrite() before the SPI transaction
  - removing and restoring TOUCH_CS from platformio.ini
  - full SPI re-init inside the handler
  - reverting rawtouch.h and then all of src/ to cf01502

Conclusion: this is most likely hardware, not code. Same code, same
board, different result across the day. Possible causes: the touch FPC
cable worked loose, or a GPIO was damaged (an early scanner wrote to
GPIO34/35/36/39 which are input-only).

Next step for touch: physical inspection. Look for a second flat cable
and an XPT2046 chip on the back. Then try a third CYD board that has
not been used for experiments. Do NOT spend more hours on the code
until hardware is ruled out.

## In progress: MQTT bridge to Telegram

This is the path that turns the device from a display into part of
the SLH system. It does NOT depend on touch.

Why MQTT and not HTTP: the bot runs on Railway in the cloud, the
devices are on a home LAN at 10.0.0.x. The cloud cannot reach them.
With MQTT both sides connect OUT to broker.hivemq.com, so it works
from anywhere with no bridge machine and no port forwarding.

The bot already has the infrastructure:
  handlers/esp_handler.py    315 lines, paho-mqtt, broker.hivemq.com:1883
  commands: /esp_status /esp_ping /esp_activate /esp_start /esp_stop
            /esp_progress /esp_heartbeat /esp_broadcast_progress
  protocol: topic = dev["mqtt_topic"] or "slh/esp/<device_id>"
            publishes plain text to <topic>/command
            listens on <topic>/response

Firmware side: src/mqtt_link.h is written and PubSubClient@^2.8 is in
platformio.ini. NOT yet wired into main.cpp. Three steps remain:

  1. Add after #include "secrets.h":
       #include "mqtt_link.h"
       WiFiClient   mqNet;
       PubSubClient mq(mqNet);
       String       mqBase = "";
     Build. Commit.

  2. Add void mqApply(const String& cmd) immediately BEFORE void setup().
     It must come after SCREEN_NAMES, g_currentScreen and
     drawCurrentScreen are defined or the build fails.
     Handles: ping, status, screen <name>, refresh.
     Build. Commit.

  3. Call mqBegin() in setup() after WiFi connects, and mqLoop() at
     the top of loop(). Build. Commit. Flash.

Then in Telegram, register the topic (the bot reads mqtt_topic
singular, our devices.json has mqtt_topics plural):

  /exec cd /app && python3 -c "
  import json
  p=\"state/devices.json\"
  d=json.load(open(p,encoding=\"utf-8\"))
  for k in [\"ESP_14335C6C32C0\",\"ESP_14335C6C8140\"]:
      if k in d[\"devices\"]:
          d[\"devices\"][k][\"mqtt_topic\"]=\"slh/esp/\"+k
          d[\"devices\"][k][\"owner\"]=\"8789977826\"
  json.dump(d,open(p,\"w\",encoding=\"utf-8\"),ensure_ascii=False,indent=2)
  print(\"OK\")"

Test: /esp_ping ESP_14335C6C32C0 should reply "pong 10.0.0.4".

## Tooling on PC1

All in C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware

  slh.ps1 status            both devices at a glance
  slh.ps1 view              what is on each screen, as text
  slh.ps1 screen <name>     switch both devices
  slh.ps1 flash             OTA both
  flash-ota.ps1 -esp IP     build + wireless flash + verify
  view.ps1 -esp IP          ASCII render of one screen
  market.ps1                trading context table
  touchmeter.ps1            live touch diagnostics
  edit-helper.ps1           Edit-Src, fails loudly if pattern missing

Price logger: C:\VirtualDrive\SLH_ECOSYSTEM\price-logger\logger.py
runs via scheduled task SLH-PriceLogger, writes prices.csv every 60s.
This is the data foundation for any future backtesting.

## Hard-won lessons - read these

1. COMMIT BEFORE EXPERIMENTING. A full days work (neural animation,
   navbar, OTA UI) was nearly lost because it was never committed
   before a git stash and checkout sequence. It survived only as
   src/main.cpp.today on disk.

2. Verify every edit. PowerShell .Replace() fails silently when the
   indentation does not match. Use Edit-Src, or edit by line number,
   and always end with a Select-String that proves it landed.

3. Build after every single change. Not after three.

4. Do not delete code blocks by scanning for a closing brace in
   column 0. Count braces, or replace line by line.

5. Long pasted blocks get truncated in this users terminal. Keep
   each block short enough to survive a paste.

6. When a fix does not work twice, stop fixing and start measuring.
   The touch problem consumed hours because symptoms were patched
   five times before the hardware was isolated and tested.

## Product rule that does not change

The device must never tell anyone which direction to trade. No buy
signals, no direction arrows. A 30 minute move on BTC is roughly the
size of the fees. What it shows instead: day range, position in that
range, volatility, fees as a share of the available move, and risk
per $1000 at different stop sizes. The student decides.
## Update 2026-09-12 evening — MQTT + cloud OTA live

Both devices on c2af79e. MQTT bridge to bot works.

Devices (DHCP renewed — IPs changed):
  ESP_14335C6C32C0  ->  10.0.0.2  (MAC 14:33:5C:6C:32:C0)
  ESP_14335C6C8140  ->  10.0.0.3  (MAC 14:33:5C:6C:81:40)

Verified commands (Telegram):
  /esp_ping <id>   ->  pong <ip>           (works both)

Firmware supports new MQTT command:
  "ota <url>"      -> HTTP download + self-flash + restart
  (bot-side /esp_ota not yet implemented)

Security debt (does NOT block testing, DOES block shipping):
  broker.hivemq.com is public. Topic = slh/esp/<MAC>.
  Anyone who guesses the MAC can send commands including ota <url>.
  Before any external user: private broker with auth, or shared secret.

Open issues:
  - bot does not have /esp_status or /esp_screen (only /esp_ping)
  - OWNER_NAME duplicated at src/main.cpp lines ~276-277 (cosmetic)
  - touch still broken (hardware suspected) — unchanged


## Update 2026-09-12 night — CLOUD OTA COMPLETE

Both devices on b75af36. Cloud OTA via Telegram WORKS end-to-end.

Commits (slh-firmware):
  b75af36  fix: follow redirects inside mqRunOta
  3147545  fix: follow redirects in httpGet helper (bonus)
  7ca1b0f  release: firmware/firmware.bin committed to repo for raw delivery

Working OTA command (anywhere, no PC needed):
  /esp_ota <device_id> https://raw.githubusercontent.com/OsifDev/slh-firmware/main/firmware/firmware.bin

Why raw.githubusercontent and not GitHub Releases:
  Releases redirect (302) from github.com to objects.githubusercontent.com.
  ESP32 HTTPClient + WiFiClientSecure cannot complete TLS across that host change.
  raw.githubusercontent.com returns 200 directly with valid SNI. Works.

Workflow to ship firmware (still requires PC for build):
  1. Edit src/main.cpp
  2. pio run
  3. Copy .pio\build\esp32-2432s028\firmware.bin  ->  firmware/firmware.bin
  4. git add firmware/firmware.bin src/ && git commit && git push
  5. In Telegram: /esp_ota <id> https://raw.githubusercontent.com/OsifDev/slh-firmware/main/firmware/firmware.bin

Next step (not tonight):
  GitHub Action to build firmware.bin on every push to main.
  Then even the build does not need a PC powered on.

Security debt unchanged:
  broker.hivemq.com is public. Anyone knowing MAC can send ota <url>.
  Before external users: private broker + auth, or signed payloads.


