# 🧰 Hardware

## ESP32-C3
- Doporučeno **ESP32-C3 SuperMini** (levné, malé, vestavěná anténa).
- ⚠️ **Pouze 2,4 GHz Wi-Fi** — 5GHz SSID se nepřipojí. Dej ESP do 2,4GHz sítě.
- Napájení: 5 V / USB-C. V karavanu lze napájet z USB zásuvky nebo z 12 V přes
  step-down na 5 V. Hlídej stabilní napájení — při podpětí hrozí boot-loop
  (viz „Boot crash" ve [flashing.md](flashing.md)).

## Karavan / panel
- **Hobby De Luxe 495 UL** (sezóna 2023) s ovládacím panelem
  **HobbyConnect / Hobby BT Masterpanel MoCa 6121980022**, čip **BLUENRG-M0L**.
- Panel poskytuje **Bluetooth LE** rozhraní (stejné jako mobilní appka HobbyConnect).
- Stejný modul mají i karavany **Fendt** → komponenta je přenositelná.

## Ověřená výbava (dle dodací faktury, Hobby 495 UL, model 2023)
Co v tomto konkrétním kuse reálně je a jak to mapuje na entity:
- 🧊 **Lednice Dometic, 133 l, řada 10, absorpční** (3-zdrojová: plyn / 12 V / 230 V) → `FRIDGE_*` (zdroj Auto/Plyn/12V/230V, teplota 1–5).
- 🔋 **Autark-paket: AGM baterie 95 Ah + bateriový senzor (IBS)** → `IBS0_*` (proto `IBS0_CAPACITY` = 95).
- 💧 **Nádrž pitné vody — zvětšena na 120 l** (z originálních 25 l dle faktury; odpadní 23,5 l) → `WATER_LEVEL`.
- 🔥 **Topení Truma S-3004** (manuální plynové, **ne** Combi 4/6 → není na panelu/BLE) + **Therme Truma** (bojler) → `THERME_ON`.
- 💡 **Ambientní osvětlení** (placená výbava) → `LIGHT_AMB1-3`; dimmovatelné „Lesespots mit Farbspiel" → `LIGHT_DIM`; čtecí světla v ložnici.
- 🔌 **HOBBY CONNECT** (panel + Bluetooth) — předmět této integrace.
- ❄️ **Žádná vestavěná klimatizace** → Sinclair je dokoupený zvlášť (řešen přes Gree/Wi-Fi, ne BLE).

> Výbava se kus od kusu liší — jiný Hobby/Fendt může mít jiné položky; firmware hlásí
> všechny klíče bez ohledu na reálnou výbavu (viz [`protocol-keys-full.md`](protocol-keys-full.md)).

## BLE spojení
| Položka | Hodnota |
|---------|---------|
| Název | `HobbyConnect Data` |
| MAC | `XX:XX:XX:XX:XX:XX` (najdi si svou — sken v ESPHome logu / nRF Connect) |
| Service | `C7841029-FE7C-4894-8532-F97908EF1AE4` |
| Characteristic | `0x0001` (NOTIFY + WRITE) |

> ESP32-C3 drží **jediné** BLE spojení na panel. Pokud je připojená i mobilní appka,
> může si „přebírat" spojení — pro mapování/provoz měj appku odpojenou.

## Umístění
ESP umísti tak, aby mělo signál na panel (uvnitř karavanu, do pár metrů) i na Wi-Fi AP.
RSSI je vidět v ESPHome logu / jako diagnostické čidlo.
