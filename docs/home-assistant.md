# 🏠 Home Assistant

Po nahrání firmwaru se ESP32-C3 objeví v HA jako **ESPHome** integrace
(autodiscovery). Entity mají prefix podle názvu zařízení (`hobby_495ul_karavan_…`).

## Entity

### Senzory (čtení)
| Entita | Popis |
|--------|-------|
| `sensor.…_teplota_vnitrni` / `_venkovni` | teploty (°C) |
| `sensor.…_napajeni` | 230 V připojeno |
| `sensor.…_sw_verze` | verze SW panelu |
| `sensor.…_baterie_nabiti` / `_napeti` / `_proud` / `_zbyva` / `_teplota` | baterie (`IBS0_*`) |
| `sensor.…_nadrz_vody` | hladina pitné vody (stupnice TBD) |

### Ovládání
| Entita | Příkaz BLE |
|--------|-----------|
| `switch.…_svetlo_kuchyn` / `_svetlo_venku` / `_ambient_1..3` | `cmd-tgl:LIGHT_*` |
| `light.…_svetlo_obyvak` / `_ambient_zadni_okno` (s jasem) | `net-LIGHT_DIM2/3-N` |
| `switch.…_hlavni_vypinac` / `_svetla_vse` | centrální (`HS_KEY*`) |
| `switch.…_podlahove_topeni` / `_bojler` | `cmd-tgl:FLOOR_HEATER_ON` / `THERME_ON` |
| `switch.…_lednice` + `select.…_lednice_rezim` + `number.…_lednice_teplota` | `cmd-tgl`/`net-FRIDGE_*` (🧪 ověřit) |
| `text.…_raw_cmd` | **diagnostika** — pošle libovolný BLE příkaz |

> Stmívače jsou `light` (monochromatic, optimistický stav) — jas 0–255 v HA se mapuje
> na 0–15 panelu. Po restartu ESP se stav světel může lišit, než přijde první notify.

## Dashboard
Ukázková záložka **„Karavan"** (styl *sections*) je v
[`../ha/dashboard-karavan.yaml`](../ha/dashboard-karavan.yaml). Lze ji vložit přes
*Upravit dashboard → Raw konfigurace* jako nový view, nebo jednotlivé karty.

Obsahuje: stav (klima/napájení/baterie/voda), světla (spínaná + stmívače + centrální),
topení & lednici, grafy teplot a baterie, a termostat klimatizace Sinclair
(`climate.karavan_klimatizace`, řešeno mimo BLE přes Gree/Wi-Fi).
