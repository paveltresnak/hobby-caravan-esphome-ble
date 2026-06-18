# 🚀 Getting started — what to buy, install and set up

Step by step from empty hands to a running caravan in Home Assistant.
No soldering and no changes to the caravan's wiring — communication with the panel
is **wireless over Bluetooth LE**.

> 🌍 Languages: [Česky](getting-started.md) · **English** · [Deutsch](getting-started.de.md)

## 1. What you'll need (prerequisites)

- **Home Assistant** — a running instance (HA OS / Container / Supervised). → https://www.home-assistant.io/
- **ESPHome** — easiest as an **add-on in HA** (*Settings → Add-ons → ESPHome*), or Docker/CLI. → https://esphome.io/
- **A computer with Chrome/Edge** — only for the very first flash (over USB/WebSerial).
- **2.4 GHz Wi-Fi** within range of the caravan — the ESP32-C3 **does not support 5 GHz**.

## 2. Shopping list

| Item | Note |
|------|------|
| **ESP32-C3** (the "SuperMini" type recommended) | any ESP32-C3 with BLE; cheap (a few €). ⚠️ **2.4 GHz Wi-Fi only**. |
| **USB-C cable** | a **data** cable (not charge-only) — for the first flash. |
| **USB power** | the board has **USB-C** → power it from anything with USB (a socket in the caravan, a power bank, an adapter…). The power source is up to you. |

## 3. Install ESPHome

Easiest route = **HA add-on**: *Settings → Add-ons → Add-on Store → ESPHome →
Install → Start*. Open the ESPHome web UI (dashboard).
(Alternatives: `docker run … ghcr.io/esphome/esphome`, or `pip install esphome`.)

## 4. First firmware flash (over USB)

1. Connect the ESP32-C3 to your computer with a USB-C cable.
2. Open **https://web.esphome.io** in Chrome/Edge → **Connect** → pick the port →
   **Prepare for first use** → enter your **2.4 GHz Wi-Fi** (SSID + password).
3. The ESP joins Wi-Fi and shows up in the ESPHome dashboard, ready to "adopt".

> A custom component can't be uploaded straight from the web — hence this base install
> first, and only then our config via ESPHome (steps 5–6).

## 5. Add this project

1. Download/clone the repo:
   `git clone https://github.com/paveltresnak/hobby-caravan-esphome-ble`
2. In `esphome/`, copy `secrets.example.yaml` → `secrets.yaml` and fill in your Wi-Fi
   (**never commit `secrets.yaml`**).
3. Put `esphome/hobby-caravan.yaml` (and the `my_components/fendt_caravan/` folder)
   into your ESPHome configuration (a folder the ESPHome dashboard can see).

## 6. Find your panel's MAC and flash via OTA

1. **Find the BLE MAC** of your panel — either:
   - from the **ESPHome log** (with `esp32_ble_tracker` active you'll see the device
     `HobbyConnect Data` and its address), or
   - with the **nRF Connect** app / a BLE scanner (look for `HobbyConnect Data`).
2. Put the MAC into `hobby-caravan.yaml` (`ble_client: → mac_address:` instead of `XX:XX:…`).
3. Flash: in the ESPHome dashboard **Install → Wirelessly** (OTA), or
   `esphome run esphome/hobby-caravan.yaml`.

## 7. Done — in Home Assistant

- The **ESPHome** integration discovers the device automatically → confirm.
- Entities (temperatures, battery, lights, fridge…) appear with a prefix based on the device name.
- Import the ready-made dashboard from [`../ha/dashboard-karavan.yaml`](../ha/dashboard-karavan.yaml)
  (*Dashboard → Edit → Raw configuration editor → add as a new view*; adjust the entity prefix).

## If something doesn't work

- Boot-loop / safe-mode after flashing, API overload, reload after OTA → [`flashing.md`](flashing.md) (Gotchas, in Czech).
- Hardware, BLE connection, ESP placement → [`hardware.md`](hardware.md).
- Entities, precision, dashboard → [`home-assistant.md`](home-assistant.md).
- What each BLE message means → [`ble-protocol.md`](ble-protocol.md) + [`protocol-keys-full.md`](protocol-keys-full.md).

> ℹ️ The detailed reference docs (`flashing`, `hardware`, `home-assistant`, `ble-protocol`)
> are currently in Czech — translations welcome.
