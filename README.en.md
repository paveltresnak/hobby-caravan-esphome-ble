# 🚐 Hobby caravan → Home Assistant (ESP32-C3 / ESPHome / BLE)

A bridge between the **Hobby De Luxe 495 UL** caravan (control panel *HobbyConnect /
HobbyConnect*) and **Home Assistant**, using a cheap **ESP32-C3** that connects
to the caravan's control unit over **Bluetooth LE** and exposes its data and controls
as native HA entities — lights, dimmers, fridge, heating, boiler, battery and temperatures.

It is a **Home Assistant integration over Wi-Fi and Bluetooth LE via an ESP32**:

```
Caravan (HobbyConnect panel)  ──BLE──►  ESP32-C3  ──Wi-Fi──►  Home Assistant
```

![The "Karavan" dashboard in Home Assistant](docs/img/dashboard.png)

The same HobbyConnect module is also used by **Fendt** caravans, so this project builds
on the ESPHome component [`fendt_caravan`](https://github.com/esphome/esphome/pull/13327)
(PR #13327 by *rawsludge*), which we **extended** to full parity with the HobbyConnect
mobile app and added **control (WRITE) commands**.

> ℹ️ That PR was **closed** in August 2026 (the author is rewriting the component in
> smaller pieces). This fork lives here and runs on its own — including fixes for the
> crash paths the ESPHome review found (see [`docs/component-changes.md`](docs/component-changes.md)).

> 🌍 Languages: [Česky](README.md) · **English** (this file) · [Deutsch](README.de.md)

---

## ✨ Features

| Domain | HA entities | Status |
|--------|-------------|--------|
| 🌡️ Temperatures | inside, outside (°C) | ✅ read |
| 🔋 Battery | voltage, current, charge %, remaining time, temp | ✅ read (`IBS0_*`) |
| 💡 Switched lights | kitchen, bathroom, exterior, ambient 1–3 — plus **any other key** your panel reports ³ | ✅ control + state |
| 🎚️ Dimmers | living room, rear-window ambient, bed right/left (0–15) | ✅ control |
| 🔆 Central | master switch, all lights | ✅ control + state |
| 🔥 Heating | boiler (floor heating optional per model) | ✅ control + state |
| 🧊 Fridge | on/off · source (Auto/Gas/12V/230V) · temperature (1–5) | ✅ control + state |
| 🔌 Power | 230 V connected, firmware version | ✅ read |
| 💧 Fresh water tank | level in % (0 / 25 / 50 / 75 / 100) | ✅ read ¹ |
| 📶 Caravan link | BLE connected / out of range | ✅ read ² |
| 🩺 ESP diagnostics | uptime, Wi-Fi signal, firmware version, restart | ✅ |

> ¹ The `WATER_LEVEL` scale is **coarse, in 25 % steps** (verified while filling, 2026-07-01),
> not litres. The panel measures **on demand** — a fresh value is sent when the water screen is
> opened on the panel, so the ESP re-requests all variables every 10 min (`net-BT_VARS`).
> ² The true BLE link state (`binary_sensor`), **not** `power` — that one is the 230 V mains.
> Out of range the measured sensors go *Unavailable* instead of showing a stale last value.
> ³ The panel reports the same set of keys regardless of floor plan, so a key that is a dead
> relay in one caravan is a real light in another (`LIGHT_WASCH` is the toilet light in a
> Hobby 460 UFE). Just add `key_name: <KEY>` under `switch:` — no C++ change needed.

## ⚡ What you need

- 🏠 **Home Assistant** — a running instance
- 🧩 **ESPHome** — easiest as a HA add-on
- 📡 **ESP32-C3** (with BLE; ⚠️ 2.4 GHz Wi-Fi only) + USB-C cable and power
- 📶 **2.4 GHz Wi-Fi** within range of the caravan

👉 **Step-by-step from scratch:** [`docs/getting-started.en.md`](docs/getting-started.en.md)

## 🧰 Hardware

- **ESP32-C3** (SuperMini) — note: **2.4 GHz Wi-Fi only** (5 GHz won't connect).
- Caravan **Hobby De Luxe 495 UL** with *HobbyConnect*.
- BLE device `HobbyConnect Data`, MAC `XX:XX:XX:XX:XX:XX` (find yours via a scan).
- Details: [`docs/hardware.md`](docs/hardware.md).

## 🚀 Quick install

1. ESPHome (here running as a Docker container on a Synology NAS, dashboard on port 6052).
2. Copy `esphome/secrets.example.yaml` → `secrets.yaml` and fill in Wi-Fi
   (**do not commit `secrets.yaml`**).
3. First flash via [web.esphome.io](https://web.esphome.io) (*Prepare for first use*, USB),
   then OTA: `esphome run esphome/hobby-caravan.yaml`.
4. The extended component lives locally in `esphome/my_components/fendt_caravan/`.
5. Step by step: [`docs/flashing.md`](docs/flashing.md).

## 📡 BLE protocol

The full key map (`TEMP_IN`, `LIGHT_*`, `FRIDGE_*`, `IBS0_*` …) and the **control command
formats** (`cmd-tgl:KEY` for on/off, `net-KEY-N` for dimming) are documented in
[`docs/ble-protocol.md`](docs/ble-protocol.md) — reverse-engineered from a real unit.
A complete key catalog is in [`docs/protocol-keys-full.md`](docs/protocol-keys-full.md).

## 🏠 Home Assistant

A sample dashboard (the "Karavan" tab) is in [`ha/`](ha/), described in
[`docs/home-assistant.md`](docs/home-assistant.md).

## 🙏 Credits

This project builds on the work of others — thanks to:
- **[@rawsludge](https://github.com/esphome/esphome/pull/13327)** (Ahmet) — original ESPHome
  `fendt_caravan` component (PR #13327) and reverse engineering of the Fendt/HobbyConnect protocol.
- **tomjeppis** — logged the heating/Truma variables (`HEATER_*`, `AC_TRUMA_*`) via nRF Connect.
- The HA community thread *"Hobby Connect for Hobby Caravans"* (#889146) and its contributors.

## 📄 License

- **Our original content** (docs, dashboard, configuration) — **MIT** (see `LICENSE`).
- The **`fendt_caravan` component** is derived from ESPHome and retains its **dual license**
  (C++ `GPLv3`, Python `MIT`) — see
  [`esphome/my_components/fendt_caravan/NOTICE.md`](esphome/my_components/fendt_caravan/NOTICE.md).
