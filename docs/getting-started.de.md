# 🚀 Erste Schritte — was kaufen, installieren und einrichten

Schritt für Schritt von leeren Händen zum laufenden Wohnwagen in Home Assistant.
Kein Löten und keine Eingriffe in die Elektrik des Wohnwagens — die Kommunikation
mit dem Panel ist **drahtlos über Bluetooth LE**.

> 🌍 Sprachen: [Česky](getting-started.md) · [English](getting-started.en.md) · **Deutsch**

## 1. Was du brauchst (Voraussetzungen)

- **Home Assistant** — eine laufende Instanz (HA OS / Container / Supervised). → https://www.home-assistant.io/
- **ESPHome** — am einfachsten als **Add-on in HA** (*Einstellungen → Add-ons → ESPHome*), oder Docker/CLI. → https://esphome.io/
- **Ein Computer mit Chrome/Edge** — nur für das allererste Flashen (über USB/WebSerial).
- **2,4-GHz-WLAN** in Reichweite des Wohnwagens — der ESP32-C3 **unterstützt kein 5 GHz**.

## 2. Einkaufsliste

| Artikel | Hinweis |
|---------|---------|
| **ESP32-C3** (Typ „SuperMini" empfohlen) | beliebiger ESP32-C3 mit BLE; günstig (wenige €). ⚠️ **nur 2,4-GHz-WLAN**. |
| **USB-C-Kabel** | ein **Datenkabel** (nicht nur Ladekabel) — für das erste Flashen. |
| **USB-Stromversorgung** | das Board hat **USB-C** → versorge es mit allem, was USB liefert (Steckdose im Wohnwagen, Powerbank, Adapter…). Die Stromquelle ist dir überlassen. |

## 3. ESPHome installieren

Einfachster Weg = **HA-Add-on**: *Einstellungen → Add-ons → Add-on Store → ESPHome →
Installieren → Starten*. Öffne die ESPHome-Weboberfläche (Dashboard).
(Alternativen: `docker run … ghcr.io/esphome/esphome`, oder `pip install esphome`.)

## 4. Erstes Flashen der Firmware (über USB)

1. Verbinde den ESP32-C3 per USB-C-Kabel mit dem Computer.
2. Öffne **https://web.esphome.io** in Chrome/Edge → **Connect** → Port wählen →
   **Prepare for first use** → dein **2,4-GHz-WLAN** eingeben (SSID + Passwort).
3. Der ESP verbindet sich mit dem WLAN und erscheint im ESPHome-Dashboard zum „Adoptieren".

> Eine eigene Komponente lässt sich nicht direkt über die Website flashen — daher zuerst
> diese Basisinstallation und erst dann unsere Konfiguration via ESPHome (Schritte 5–6).

## 5. Dieses Projekt einspielen

1. Repo herunterladen/klonen:
   `git clone https://github.com/paveltresnak/hobby-caravan-esphome-ble`
2. In `esphome/` `secrets.example.yaml` → `secrets.yaml` kopieren und WLAN eintragen
   (**`secrets.yaml` niemals committen**).
3. `esphome/hobby-caravan.yaml` (und den Ordner `my_components/fendt_caravan/`) in deine
   ESPHome-Konfiguration legen (ein Ordner, den das ESPHome-Dashboard sieht).

## 6. MAC des Panels finden und per OTA flashen

1. **BLE-MAC** deines Panels ermitteln — entweder:
   - aus dem **ESPHome-Log** (mit aktivem `esp32_ble_tracker` siehst du das Gerät
     `HobbyConnect Data` und seine Adresse), oder
   - mit der App **nRF Connect** / einem BLE-Scanner (nach `HobbyConnect Data` suchen).
2. MAC in `hobby-caravan.yaml` eintragen (`ble_client: → mac_address:` statt `XX:XX:…`).
3. Flashen: im ESPHome-Dashboard **Install → Wirelessly** (OTA), oder
   `esphome run esphome/hobby-caravan.yaml`.

## 7. Fertig — in Home Assistant

- Die Integration **ESPHome** erkennt das Gerät automatisch → bestätigen.
- Entitäten (Temperaturen, Batterie, Licht, Kühlschrank…) erscheinen mit einem Präfix nach dem Gerätenamen.
- Importiere das fertige Dashboard aus [`../ha/dashboard-karavan.yaml`](../ha/dashboard-karavan.yaml)
  (*Dashboard → Bearbeiten → Raw-Konfigurationseditor → als neue Ansicht hinzufügen*; Entitäts-Präfix anpassen).

## Wenn etwas nicht funktioniert

- Boot-Loop / Safe-Mode nach dem Flashen, API-Überlastung, Reload nach OTA → [`flashing.md`](flashing.md) (Gotchas, auf Tschechisch).
- Hardware, BLE-Verbindung, ESP-Platzierung → [`hardware.md`](hardware.md).
- Entitäten, Genauigkeit, Dashboard → [`home-assistant.md`](home-assistant.md).
- Was jede BLE-Nachricht bedeutet → [`ble-protocol.md`](ble-protocol.md) + [`protocol-keys-full.md`](protocol-keys-full.md).

> ℹ️ Die ausführlichen Referenz-Dokumente (`flashing`, `hardware`, `home-assistant`,
> `ble-protocol`) sind derzeit auf Tschechisch — Übersetzungen willkommen.
