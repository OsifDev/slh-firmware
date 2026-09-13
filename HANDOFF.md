# SLH FIRMWARE - HANDOFF

Last updated: 2026-09-13
Repo: https://github.com/OsifDev/slh-firmware
Read PROJECT.md too - it holds the hardware facts and pin map.

## What this project is

An ESP32-2432S028 (CYD) device that shows live crypto prices and
teaches trading context. It is a node in the SLH system: it talks to
the Telegram bot over MQTT and will eventually show wallet balances,
journal entries and governance votes.

The goal is a product that can be sold to students, not a demo.

## Devices

| | Device 1 | Device 2 |
|---|---|---|
| owner | OSIF | ZVIKA |
| MAC | 14:33:5C:6C:32:C0 | 14:33:5C:6C:81:40 |
| key | ESP_14335C6C32C0 | ESP_14335C6C8140 |

IP ADDRESSES CHANGE. The router reassigns them. Never hardcode an IP
in a test - scan for the device first:

  1..30 | ForEach-Object { $ip="10.0.0.$_"; try {
    $s=Invoke-RestMethod "http://$ip/state" -TimeoutSec 2
    Write-Host "$ip -> $($s.device.mac)" -ForegroundColor Green } catch {} }

## START HERE - run these before touching anything

1. Find the devices (scan above). Note the current IPs.
2. cd C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware
3. git pull --rebase origin main     # a CI action pushes firmware.bin
4. git status --short                # must be clean before you edit
5. pio run                           # must say SUCCESS before you change code
6. .\slh.ps1 status                  # both devices, screen, heap, data age

If step 5 fails, fix the build before doing anything else. Never
flash on top of a broken build.

## What works right now

PRICES    BTC/ETH/BNB live from CoinGecko, Binance fallback, keeps
          last known values with an age stamp if both fail
SLH ROW   fixed ILS 444 with a PRESALE badge - not a market price
SPARKLINE 24h chart per coin from Binance klines
SCREENS   home, prices, market, setup, show, lesson
LESSONS   11 Hebrew pages, RLE compressed (704KB raw -> 130KB)
BOOT      neural network animation, 16 nodes, sprite based
OTA       wireless flash with a progress bar on the device
TOUCH     working - z reads 600-1500, navbar buttons respond
CLOCK     NTP, Israel timezone, shown in the header
EVENTS    ring buffer of 32 entries, exposed at /events
MQTT      two-way over broker.hivemq.com, works from anywhere
FLASH     61.6% used, about 740KB free

## HTTP API (LAN only, port 80)

  GET /state              full device state as JSON
  GET /view               what is on screen, as text lines
  GET /screen             current screen + list of all
  GET /screen?to=market   switch screen
  GET /events             event log with timestamps
  GET /touch              live touch read, raw + mapped
  GET /lesson?page=N      show a lesson page
  GET /backlight?pct=N    brightness 5-100
  GET /refresh            force a price refresh
  GET /ota                firmware upload form

## MQTT (works from anywhere, not just the LAN)

  broker  broker.hivemq.com:1883, no auth
  topic   slh/esp/<device_key>/command   bot publishes here
          slh/esp/<device_key>/response  device replies here
  payload plain text, not JSON

  commands: ping | status | refresh | screen <name>

Test it without Telegram:
  python mqtt_test.py ping
  python mqtt_test.py "screen market"

From Telegram: /esp_ping ESP_14335C6C32C0

## SECURITY DEBT - blocks public release

The broker is public and unauthenticated. The topic is derived from
the MAC address, which is printed on the board and visible in any
network scan. Anyone who knows it can control the screen.

If voting ships over this channel, anyone can vote as any device.
That breaks the whole governance system.

Before selling devices: private broker with per-device credentials,
or sign the vote payload with a key held only by the owner.
This does not block development. It blocks release.

## Tooling on PC1

  slh.ps1 status            both devices at a glance
  slh.ps1 view              screen contents as text
  slh.ps1 screen <name>     switch both
  flash-ota.ps1 -esp IP     build + wireless flash + verify
  view.ps1 -esp IP          ASCII render of one screen
  market.ps1                trading context table
  touchmeter.ps1            live touch diagnostics
  mqtt_test.py <cmd>        send an MQTT command, show the reply
  edit-helper.ps1           Edit-Src, fails loudly if pattern missing

Price logger: C:\VirtualDrive\SLH_ECOSYSTEM\price-logger\logger.py
runs as scheduled task SLH-PriceLogger, writes prices.csv every 60s.
This is the only real data foundation for future backtesting.

## Ground rules - every one of these cost hours to learn

1. COMMIT AS SOON AS THE BUILD PASSES. A full day of work was nearly
   lost to a git stash sequence because it was never committed.

2. Verify every edit. PowerShell .Replace() fails silently when the
   indentation does not match. End every edit block with a
   Select-String that proves it landed.

3. Build after every single change, not after three.

4. Never delete a code block by scanning for a closing brace in
   column 0. Count braces, or replace by line number.

5. When removing a line by index, use $m[0..($i-1)] not ($i-2).
   An off-by-one here deleted mqLoop() and cost an hour.

6. C++ needs forward declarations. A function that calls another
   defined later in the file will not link.

7. Never put a blocking call inside the main loop. pollServer() hit
   a 404 with a 6 second timeout every 15 seconds. That single line
   starved MQTT, froze price refresh, and made touch look dead.

8. Do not use tft.getTouch(). Use rtTouch() from rawtouch.h.
   TOUCH_CS must stay in platformio.ini so TFT_eSPI compiles, but
   tft.setTouch() must NEVER be called - it takes ownership of the
   XPT2046 and fights rtBegin() for the same chip. That conflict is
   what killed touch for two days.

9. Binance returns numbers as JSON strings. Use
   String((const char*)(doc["field"] | "0")).toFloat()

10. When a fix fails twice, stop fixing and start measuring. Five
    symptom patches were applied to touch before the real cause was
    isolated by comparing two boards running the same firmware.

## ROADMAP - in order, each step is a session

### STEP 1 - UI rebuild (next session)

The UI was built incrementally during debugging and looks it.
Do not patch it. Rebuild it once, properly.

What it needs:
  - one animation engine, sprite based, reused everywhere
  - scrolling for lists that do not fit
  - a consistent colour palette in one header file
  - larger, clearly separated touch targets
  - visual feedback on every press

Verify before starting:
  pio run                    must be SUCCESS
  .\slh.ps1 status           both devices online
  touch a navbar button      screen must change

Verify when done:
  every screen fits in 240px with no overlap
  .\view.ps1 renders each screen correctly
  flash both devices, both boot clean

### STEP 2 - journal and tasks on the device

BLOCKED: the bot has /journal for writing but nothing that reads it
back. Same for tasks.

Add to the bot via GitHub + /deploy (NOT via /exec - code written
through /exec does not survive a restart, only state/ persists):

  handlers/device_api_handler.py
    /journal_list    returns the last 8 entries as JSON
    /tasks_open      returns open tasks as JSON
  register it in handlers/loader.py

Then in firmware: MQTT commands journal and tasks, a JOURNAL screen
with scrolling.

### STEP 3 - governance voting from the device

BLOCKED TWICE.

First: the data was not found. /gov_status reports 6 proposals and 2
open, but db["governance"]["proposals"] came back empty. Find where
proposals actually live before writing any code:

  /exec cd /app && sed -n "110,140p" handlers/governance_handler.py

Second: the security debt above. Voting over an open public broker
is not acceptable in a product. Fix the transport first.

### STEP 4 - wallet balances

BLOCKED: /api/holders returns an error - the connected_wallets table
does not exist in the database. That has to be fixed server side
before the device can show anything real.

Note: the ESP_<timestamp> entries in devices.json are internal
records, not crypto wallets. There is no private key and no on-chain
address. Do not present them as wallets to users.

### STEP 5 - price alerts

Not blocked. Set a threshold, the screen flashes and MQTT publishes
to the response topic so the bot can notify. This is the most
useful feature still unbuilt and it depends on nothing.

### STEP 6 - backtesting

Wait until prices.csv has at least a week of data. Then analyse
volatility and test a specific hypothesis against it.

Order matters: data first, then hypothesis, then test. Never the
other way around.

## PRODUCT RULE - does not change

The device must never tell anyone which direction to trade. No buy
signals, no direction arrows, no entry calls.

Reason: a 30 minute move on BTC is roughly the size of the fees.
Measured on 2026-09-11, ETH moved 4.52% in a day - about 0.19% per
half hour. Round-trip fees with spread are 0.1 to 0.2%. The expected
value of a 30 minute trade is near zero before mistakes.

A confident arrow on a nice screen teaches a student that someone
knows the future. That is how accounts get emptied.

What the device shows instead: the day range, where price sits in
it, volatility, fees as a share of the available move, and risk per
$1000 at different stop sizes. The student decides.

The SETUP screen turning the fee line red on a quiet day is the most
valuable thing this device does. It teaches when NOT to trade.

## FOUND 2026-09-13: governance data structure

Proposals live in db["governance"]["proposals"] as a LIST, not a dict.
Each entry: id, status (open|approved), type, title.
Currently 6 proposals, 2 open (ids 3 and 4).
votes {} is empty; individual_votes has 4 entries.
Other keys: agents_registry, sessions, task_map, rules, source_of_truth.

Read it with:
  /exec cd /app && python3 -c "import json;d=json.load(open(\"state/db.json\",encoding=\"utf-8\"));g=d[\"governance\"];print([(p[\"id\"],p[\"status\"],p[\"title\"][:40]) for p in g[\"proposals\"]])"

The container has python3 but NOT curl. Fetch URLs with:
  /exec python3 -c "import urllib.request;print(urllib.request.urlopen(URL,timeout=20).read().decode()[:3000])"

Step 3 is now unblocked on data. It remains blocked on security:
do not ship voting over the public broker.
