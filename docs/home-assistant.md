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
| `sensor.…_nadrz_vody` | hladina pitné vody v **%** (`WATER_LEVEL` 0–4 × 25 → 0/25/50/75/100), měřeno on-demand |

### Ovládání
| Entita | Příkaz BLE |
|--------|-----------|
| `switch.…_svetlo_kuchyn` / `_svetlo_koupelna` / `_svetlo_venku` / `_ambient_1..3` | `cmd-tgl:LIGHT_*` |
| `light.…_svetlo_obyvak` / `_ambient_zadni_okno` / `_postel_vpravo` / `_postel_vlevo` (s jasem) | `net-LIGHT_DIM2/3/0/1-N` |
| `switch.…_hlavni_vypinac` / `_svetla_vse` | centrální (`HS_KEY*`) |
| `switch.…_podlahove_topeni` / `_bojler` | `cmd-tgl:FLOOR_HEATER_ON` / `THERME_ON` |
| `switch.…_lednice` + `select.…_lednice_rezim` (Auto/Plyn/12V/230V) + `number.…_lednice_teplota` (1–5) | `cmd-tgl:FRIDGE_ON_OFF` / `net-FRIDGE_SOURCE-N` / `net-FRIDGE_TEMP-N` ✅ ověřeno (Dometic 10) |
| `text.…_raw_cmd` | **diagnostika** — pošle libovolný BLE příkaz |

### Diagnostika ESP (od 2026-07-24)

Entity s `entity_category: diagnostic` — v HA sedí zvlášť, nepletou se s daty z karavanu.

| Entita | K čemu je |
|--------|-----------|
| `sensor.…_esp_uptime` | **pozná restart ESP** (výpadek proudu, watchdog, OTA). Roste-li plynule, ESP nerebootuje — vyloučí to boot loop. |
| `sensor.…_esp_wifi_signal` | WiFi ESP ↔ AP. **Jiná vrstva než BLE** — bývá v pořádku (−60 dBm), i když BLE nefunguje. |
| `sensor.…_esp_verze` | verze ESPHome + config hash + čas buildu — pozná se, jestli běží to, co je v gitu |
| `button.…_esp_restart` | restart ESP na dálku |
| `sensor.…_ble_signal` | ⚠️ **v praxi k ničemu** — RSSI se čte z advertisingu, jenže HobbyConnect po připojení inzerovat přestane, takže hodnota je trvale `unknown`. Ponecháno jen pro případ dlouhého výpadku. |

> Stmívače jsou `light` (monochromatic, optimistický stav) — jas 0–255 v HA se mapuje
> na 0–15 panelu. Po restartu ESP se stav světel může lišit, než přijde první notify.

## Dostupnost & „mimo dosah"
`binary_sensor.…_spojeni` (**device_class `connectivity`**) ukazuje **pravdivý stav
BLE spojení** ESP ↔ HobbyConnect. ⚠️ Nezaměňovat se `sensor.…_napajeni`, což je
`LINE_EN` = **230 V síť**, nikoli stav spojení!

Když karavan **není v dosahu** (BLE odpojeno déle než 20 s — debounce `delayed_off`):
- `binary_sensor.…_spojeni` → **Disconnected**;
- měřené senzory (teploty, baterie, voda) → **Unavailable** (ESP na `on_disconnect`
  publikuje `NaN`), `napajeni` se vyprázdní → v HA není vidět zastaralá „poslední"
  hodnota;
- ovládací entity sice v UI zůstanou, ale **příkazy se zahodí** (komponenta zapisuje
  jen ve stavu `ESTABLISHED`);
- dashboard ukáže **badge „Karavan BLE"** + podmíněnou kartu *„Karavan není v dosahu…"*.

Po návratu do dosahu se vše **doplní automaticky** (jednotka po připojení vysype
všechny klíče; `on_connect` navíc vyžádá `net-BT_VARS`).

> Řešeno čistě standardními prostředky ESPHome/HA: template `binary_sensor` řízený
> `ble_client` triggery `on_connect`/`on_disconnect`, `delayed_off` debounce, publikace
> `NaN` → *Unavailable*, a Lovelace *conditional* karta.

### ⚠️ Zaseklý HobbyConnect: „spojení = on, ale žádná data"

**Jiná porucha než „mimo dosah" a snadno se splete.** Pozorováno 2026-07-24.

| | mimo dosah | **zaseklý HobbyConnect** |
|---|---|---|
| `binary_sensor.…_spojeni` | `off` | **`on`** ← proto to mate |
| měřené senzory | Unavailable | **Unavailable / prázdné** |
| co pomůže | přijet blíž | **restart BT modulu v karavanu** |

BLE spojení se navazuje a hlásí `ESTABLISHED`, ale **GATT notifikace nechodí** —
jednotka mlčí. Předchází tomu často fáze rychlého kolísání (`on`/`off` á ~20–30 s),
pak se spojení „ustálí" na `on` a data už nedorazí vůbec.

**Jak to poznat:** `sensor.…_baterie_napeti` je `unknown`, přestože `…_spojeni` je `on`.
Baterie je spolehlivější ukazatel než vnitřní teplota — ta chodila i v porouchaném stavu.

**Řešení (ověřeno):**
1. **Restart BT modulu v karavanu** — kompletně off → ~10 s → on. Nic v HA dělat
   netřeba, ESP má `auto_connect: true` a připojí se samo; data byla zpět do ~15 s.
2. Restart ESP (`button.…_esp_restart`) **nestačí** — zkoušen jako první, obnovil
   jen jednu hodnotu a spojení zůstalo hluché. Příčina je na straně jednotky.

> ⚠️ **Po restartu ESP nevěř stavu switchů!** Nemají `restore_mode`, takže spadnou
> na výchozí `off` bez ohledu na realitu v karavanu (`hlavni_vypinac` hlásil `off`,
> zatímco reálně byl `on`). Skutečná hodnota se objeví až s první notifikací
> z jednotky. Totéž platí pro světla — viz poznámka u stmívačů výše.

## Přesnost zobrazení (display precision)
ESP posílá floaty (např. 13,90 V je v 32bit floatu `13.8999996…`). **Zobrazenou
přesnost** řídí HA — buď `accuracy_decimals` v ESPHome (→ *suggested display
precision*), nebo uživatelský override v HA (Nastavení → Zařízení → entita →
⚙ → *Zobrazená přesnost*). Doporučené hodnoty:

| Senzor | Des. míst |
|--------|-----------|
| Teploty (vnitřní/venkovní/baterie) | 1 |
| Napětí baterie | 2 |
| Proud baterie | 1 |
| Nabití (%) | 0 |
| Zbývající čas (h) | 1 |
| Nádrž vody | 0 |

> Tip přes WS API: `config/entity_registry/update` s
> `options_domain: "sensor"`, `options: {display_precision: N}`.

## Dashboard
Ukázková záložka **„Karavan"** (styl *sections*) je v
[`../ha/dashboard-karavan.yaml`](../ha/dashboard-karavan.yaml). Lze ji vložit přes
*Upravit dashboard → Raw konfigurace* jako nový view, nebo jednotlivé karty.

Obsahuje: **badge se stavem BLE spojení** + podmíněné upozornění „mimo dosah",
stav (klima/napájení/baterie/voda), světla (spínaná + stmívače + centrální),
topení & lednici, grafy teplot a baterie, a termostat klimatizace Sinclair
(`climate.karavan_klimatizace`, řešeno mimo BLE přes Gree/Wi-Fi).
