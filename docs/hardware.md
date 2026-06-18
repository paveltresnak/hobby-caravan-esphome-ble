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

## BLE spojení
| Položka | Hodnota |
|---------|---------|
| Název | `HobbyConnect Data` |
| MAC | `DE:00:32:00:61:44` (najdi si svou — sken v ESPHome logu) |
| Service | `C7841029-FE7C-4894-8532-F97908EF1AE4` |
| Characteristic | `0x0001` (NOTIFY + WRITE) |

> ESP32-C3 drží **jediné** BLE spojení na panel. Pokud je připojená i mobilní appka,
> může si „přebírat" spojení — pro mapování/provoz měj appku odpojenou.

## Umístění
ESP umísti tak, aby mělo signál na panel (uvnitř karavanu, do pár metrů) i na Wi-Fi AP.
RSSI je vidět v ESPHome logu / jako diagnostické čidlo.
