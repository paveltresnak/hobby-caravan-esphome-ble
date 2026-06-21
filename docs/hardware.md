# 🧰 Hardware

## ESP32-C3
- Doporučeno **ESP32-C3 SuperMini** (levné, malé, vestavěná anténa).
- ⚠️ **Pouze 2,4 GHz Wi-Fi** — 5GHz SSID se nepřipojí. Dej ESP do 2,4GHz sítě.
- Napájení: 5 V / USB-C. V karavanu lze napájet z USB zásuvky nebo z 12 V přes
  step-down na 5 V. Hlídej stabilní napájení — při podpětí hrozí boot-loop
  (viz „Boot crash" ve [flashing.md](flashing.md)).

## Karavan / panel / sběrnice
- **Hobby De Luxe 495 UL** (sezóna 2023). Výbava potvrzená fakturou (schaffer-mobil/Verendus):
  **HOBBY CONNECT** — „*Steuerung für Bordtechnik über Hobby-Bedienpanel und Bluetooth per App*"
  + „*Bedienpanel für Beleuchtungssystem und Tankanzeige*".

### Architektura (dle oficiálního Hobby manuálu 2023, kap. 7 Elektroinstallationen)
- Komponenty (světla, lednice, klima, topení, nabíječ, IBS) jsou propojené přes
  **CI-Bus** — oborová karavanová sběrnice (běžně CAN-based; používá ji víc výrobců).
  Manuál: „*HobbyConnect … die meisten CI-Bus-fähigen Komponenten … bedienen*" (s.68).
- Světla řídí **Lichtsteuermodul** (modul řízení světel + pojistky vnitřních okruhů),
  uzel na CI-Busu (Kontaktplan Lichtsteuersystem, s.93/97 — svorka „CI-Bus").
- **HobbyConnect = brána CI-Bus ↔ Bluetooth LE + appka.** Náš kus má variantu **bez „+"**
  (jen BT). „HobbyConnect+" = Connect+-Box se SIM = vzdálené ovládání přes mobilní síť.
  → **Náš ESP32-C3 supluje funkci HobbyConnect+ (vzdálený přístup) zadarmo přes HA/WiFi.**

### BLE rozhraní panelu (zachyceno z provozu)
- Hlásí se jako `HobbyConnect Data`; Service UUID `C7841029-FE7C-4894-8532-F97908EF1AE4`,
  charakteristika `0x0001` (NOTIFY+WRITE). Kompatibilní i s **Fendt** (sdílí `fendt_caravan`).
- ⚠️ Konkrétní výrobní číslo Lichtsteuermodulu / LED driveru ani typ BLE čipu **manuál
  ani faktura neuvádí** (OEM díly) — zjistitelné jen rozebráním. Dřívější doc měla smyšlené údaje.

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
