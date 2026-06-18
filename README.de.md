# 🚐 Hobby Wohnwagen → Home Assistant (ESP32-C3 / ESPHome / BLE)

Eine Brücke zwischen dem Wohnwagen **Hobby De Luxe 495 UL** (Bedienpanel *HobbyConnect /
Hobby BT Masterpanel*) und **Home Assistant**: Ein günstiger **ESP32-C3** verbindet sich
über **Bluetooth LE** mit der Steuereinheit des Wohnwagens und stellt deren Daten und
Bedienung als native HA-Entitäten bereit — Licht, Dimmer, Kühlschrank, Heizung, Boiler,
Batterie und Temperaturen.

Es ist eine **Home-Assistant-Integration über WLAN und Bluetooth LE mittels ESP32**:

```
Wohnwagen (HobbyConnect-Panel)  ──BLE──►  ESP32-C3  ──WLAN──►  Home Assistant
```

Dasselbe HobbyConnect-Modul verwenden auch **Fendt**-Wohnwagen, daher baut dieses Projekt
auf der ESPHome-Komponente [`fendt_caravan`](https://github.com/esphome/esphome/pull/13327)
(PR #13327 von *rawsludge*) auf, die wir auf volle Parität mit der HobbyConnect-App
**erweitert** und um **Steuerbefehle (WRITE)** ergänzt haben.

> 🌍 Sprachen: [Česky](README.md) · [English](README.en.md) · **Deutsch** (diese Datei)

---

## ✨ Funktionen

| Bereich | HA-Entitäten | Status |
|---------|--------------|--------|
| 🌡️ Temperaturen | innen, außen (°C) | ✅ lesen |
| 🔋 Batterie | Spannung, Strom, Ladung %, Restzeit, Temp. | ✅ lesen (`IBS0_*`) |
| 💡 Schaltbare Lichter | Küche, außen, Ambiente 1–3 | ✅ Steuerung + Status |
| 🎚️ Dimmer | Wohnraum, Ambiente Heckfenster (0–15) | ✅ Steuerung |
| 🔆 Zentral | Hauptschalter, alle Lichter | ✅ Steuerung + Status |
| 🔥 Heizung | Boiler (Fußbodenheizung je nach Modell) | ✅ Steuerung + Status |
| 🧊 Kühlschrank | Ein/Aus · Quelle (12V/230V/Gas) · Temperatur | 🧪 Steuerung (Format zu prüfen) |
| 🔌 Stromversorgung | 230 V verbunden, Firmware-Version | ✅ lesen |
| 💧 Frischwassertank | `WATER_LEVEL` | ⏳ Skala offen (beim Befüllen erfassen) |

## ⚡ Was du brauchst

- 🏠 **Home Assistant** — eine laufende Instanz
- 🧩 **ESPHome** — am einfachsten als HA-Add-on
- 📡 **ESP32-C3** (mit BLE; ⚠️ nur 2,4-GHz-WLAN) + USB-C-Kabel und Stromversorgung
- 📶 **2,4-GHz-WLAN** in Reichweite des Wohnwagens

👉 **Schritt für Schritt von Null:** [`docs/getting-started.md`](docs/getting-started.md)

## 🧰 Hardware

- **ESP32-C3** (SuperMini) — Achtung: **nur 2,4-GHz-WLAN** (5 GHz verbindet nicht).
- Wohnwagen **Hobby De Luxe 495 UL** mit *HobbyConnect / Hobby BT Masterpanel*.
- BLE-Gerät `HobbyConnect Data`, MAC `XX:XX:XX:XX:XX:XX` (eigene per Scan ermitteln).
- Details: [`docs/hardware.md`](docs/hardware.md).

## 🚀 Schnellinstallation

1. ESPHome (hier als Docker-Container auf einem Synology NAS, Dashboard auf Port 6052).
2. `esphome/secrets.example.yaml` → `secrets.yaml` kopieren und WLAN eintragen
   (**`secrets.yaml` nicht committen**).
3. Erstes Flashen über [web.esphome.io](https://web.esphome.io) (*Prepare for first use*, USB),
   danach OTA: `esphome run esphome/hobby-caravan.yaml`.
4. Die erweiterte Komponente liegt lokal unter `esphome/my_components/fendt_caravan/`.
5. Schritt für Schritt: [`docs/flashing.md`](docs/flashing.md).

## 📡 BLE-Protokoll

Die vollständige Schlüsselzuordnung (`TEMP_IN`, `LIGHT_*`, `FRIDGE_*`, `IBS0_*` …) und die
**Steuerbefehl-Formate** (`cmd-tgl:KEY` für Ein/Aus, `net-KEY-N` zum Dimmen) sind in
[`docs/ble-protocol.md`](docs/ble-protocol.md) dokumentiert — per Reverse Engineering an
einem realen Gerät ermittelt. Ein vollständiger Katalog: [`docs/protocol-keys-full.md`](docs/protocol-keys-full.md).

## 🏠 Home Assistant

Ein Beispiel-Dashboard (der Reiter „Karavan") liegt in [`ha/`](ha/), beschrieben in
[`docs/home-assistant.md`](docs/home-assistant.md).

## 🙏 Danksagung

Dieses Projekt baut auf der Arbeit anderer auf — Dank an:
- **[@rawsludge](https://github.com/esphome/esphome/pull/13327)** (Ahmet) — ursprüngliche ESPHome-
  Komponente `fendt_caravan` (PR #13327) und Reverse Engineering des Fendt/HobbyConnect-Protokolls.
- **tomjeppis** — Protokollierung der Heizungs-/Truma-Variablen (`HEATER_*`, `AC_TRUMA_*`) via nRF Connect.
- Der HA-Community-Thread *„Hobby Connect for Hobby Caravans"* (#889146) und seine Mitwirkenden.

## 📄 Lizenz

- **Unsere eigenen Inhalte** (Dokumentation, Dashboard, Konfiguration) — **MIT** (siehe `LICENSE`).
- Die **Komponente `fendt_caravan`** ist von ESPHome abgeleitet und behält dessen **Dual-Lizenz**
  (C++ `GPLv3`, Python `MIT`) — siehe
  [`esphome/my_components/fendt_caravan/NOTICE.md`](esphome/my_components/fendt_caravan/NOTICE.md).
