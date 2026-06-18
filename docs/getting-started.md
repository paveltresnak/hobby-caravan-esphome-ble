# 🚀 Start od nuly — co koupit, nainstalovat a nastavit

Krok za krokem od prázdných rukou k běžícímu karavanu v Home Assistantu.
Žádné pájení ani zásahy do elektriky karavanu — komunikace s panelem je
**bezdrátová přes Bluetooth LE**.

## 1. Co budeš potřebovat (prerekvizity)

- **Home Assistant** — běžící instance (HA OS / Container / Supervised). → https://www.home-assistant.io/
- **ESPHome** — nejsnáz jako **add-on v HA** (*Nastavení → Doplňky → ESPHome*),
  případně Docker nebo CLI. → https://esphome.io/
- **Počítač s prohlížečem Chrome/Edge** — jen pro úplně první nahrání (přes USB/WebSerial).
- **2,4GHz Wi-Fi** v dosahu karavanu — ESP32-C3 **neumí 5 GHz**.

## 2. Nákupní seznam

| Položka | Poznámka |
|---------|----------|
| **ESP32-C3** (doporučeno typ „SuperMini") | jakýkoli ESP32-C3 s BLE; levné (jednotky €). ⚠️ **jen 2,4 GHz Wi-Fi**. |
| **USB-C kabel** | **datový** (ne jen nabíjecí) — kvůli prvnímu flashi. |
| **USB napájení** | ESP má **USB-C** → napájej čímkoli s USB (zásuvka v karavanu, powerbanka, adaptér…). Způsob napájení je na tobě. |

## 3. Nainstaluj ESPHome

Nejjednodušší cesta = **HA add-on**: *Nastavení → Doplňky → Obchod s doplňky →
ESPHome → Instalovat → Spustit*. Otevři webové rozhraní ESPHome (dashboard).
(Alternativy: `docker run … ghcr.io/esphome/esphome`, nebo `pip install esphome`.)

## 4. První nahrání firmwaru (přes USB)

1. Připoj ESP32-C3 USB-C kabelem k počítači.
2. Otevři **https://web.esphome.io** v Chrome/Edge → **Connect** → vyber port →
   **Prepare for first use** → zadej své **2,4GHz Wi-Fi** (SSID + heslo).
3. ESP se připojí k Wi-Fi a objeví se v ESPHome dashboardu k „adopci".

> Custom komponentu nejde nahrát hned přes web — proto napřed tahle základní instalace,
> a teprve pak náš config přes ESPHome (krok 5–6).

## 5. Nahraj tento projekt

1. Stáhni/naklonuj repo:
   `git clone https://github.com/paveltresnak/hobby-caravan-esphome-ble`
2. V `esphome/` zkopíruj `secrets.example.yaml` → `secrets.yaml` a doplň Wi-Fi
   (**`secrets.yaml` nikam necommituj**).
3. Soubor `esphome/hobby-caravan.yaml` (i složku `my_components/fendt_caravan/`)
   dej do konfigurace ESPHome (do složky, kterou ESPHome dashboard vidí).

## 6. Najdi MAC svého panelu a nahraj přes OTA

1. **Zjisti BLE MAC** svého panelu — buď:
   - z **logu ESPHome** (s aktivním `esp32_ble_tracker` uvidíš nalezené zařízení
     `HobbyConnect Data` a jeho adresu), nebo
   - mobilní appkou **nRF Connect** / BLE scanner (hledej `HobbyConnect Data`).
2. Vlož MAC do `hobby-caravan.yaml` (`ble_client: → mac_address:` místo `XX:XX:…`).
3. Nahraj: v ESPHome dashboardu **Install → Wirelessly** (OTA), nebo
   `esphome run esphome/hobby-caravan.yaml`.

## 7. Hotovo — v Home Assistantu

- Integrace **ESPHome** si zařízení najde sama (autodiscovery) → potvrď.
- Entity (teploty, baterie, světla, lednice…) naběhnou s prefixem podle názvu zařízení.
- Naimportuj připravený dashboard z [`../ha/dashboard-karavan.yaml`](../ha/dashboard-karavan.yaml)
  (*Dashboard → Upravit → Raw konfigurace → přidat jako nový view*; uprav prefix entit).

## Když něco nejde

- Boot-loop / safe-mode po flashi, zahlcení API, reload po OTA → [`flashing.md`](flashing.md) (Gotchas).
- Hardware, BLE spojení, umístění ESP → [`hardware.md`](hardware.md).
- Entity, přesnost, dashboard → [`home-assistant.md`](home-assistant.md).
- Co která BLE zpráva znamená → [`ble-protocol.md`](ble-protocol.md) + [`protocol-keys-full.md`](protocol-keys-full.md).
