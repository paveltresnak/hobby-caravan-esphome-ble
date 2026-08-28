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
| `sensor.…_nadrz_vody` | hladina pitné vody v **%** (`WATER_LEVEL` 0–4 × 25 → 0/25/50/75/100), měřeno on-demand; ⚠️ hned po restartu jednotky hlásí 0, než si hladinu změří |

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
| `sensor.…_ble_signal` | RSSI se čte z advertisingu, a připojený HobbyConnect inzerovat přestane → **za normálního provozu je trvale `unknown`**. Zato **když spojení nejede, je to ta nejcennější hodnota, kterou máš** — ukáže, jestli je jednotka slyšet a jak silně (viz [třetí případ](#-třetí-případ-po-ota-se-ble-nechytí-zpátky-status133)). |

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

### ⚠️ Třetí případ: „po OTA se BLE nechytí zpátky" (`status=133`)

Pozorováno 2026-08-28. ESP po OTA normálně naběhne — WiFi, API i `uptime` v pořádku,
žádný boot loop — ale spojení na karavan se **nenaváže skoro hodinu**. V logu:

```
[E][esp32_ble_client]: ESP_GATTC_OPEN_EVT in DISCONNECTING state (status=133)
[W][esp32_ble_client]: Connection open error, status=133
```

Jednotku přitom **vidí** (`Found device` → `Connecting`), takže to není „mimo dosah"
ani zaseklý HobbyConnect (ten by hlásil `spojeni = on` bez dat).

**Jak si ověřit, že jde o slabý signál, a ne o poruchu:**
- `sensor.…_ble_signal` má smysl **právě v odpojeném stavu** (připojené zařízení už
  neinzeruje). Tady ukazoval **-95 až -100 dBm**, přičemž den předtím se spojení
  navazovalo při **-92 až -95 dBm** → link je trvale na hraně a pár dB rozhoduje.
- ⚠️ **Když ESP zrovna zkouší připojení, vzorky RSSI přestanou chodit** — tracker během
  connect scanu ten senzor neplní. Ticho tedy neznamená, že jednotka zmizela.
- Historie v HA (*Nastavení → Historie*) ukáže, kdy data reálně přestala téct. Pokud
  jela až do restartu, není to postupné odumírání linky.

**Co s tím:**
1. **Rozšířit skenovací okno** — ESP ve výchozím stavu poslouchá jen 30 ms z každých
   320 ms (9 % času), což u slabého signálu propustí většinu inzerátů:
   ```yaml
   esp32_ble_tracker:
     scan_parameters:
       active: true
       interval: 320ms
       window: 200ms
       connection_scan_window: 200ms
   ```
2. **Posunout ESP blíž k panelu.** U -98 dBm udělá pár desítek centimetrů větší rozdíl
   než jakákoli konfigurace. (Tip: porovnej v UniFi sílu WiFi signálu ESP a jiného
   zařízení v karavanu — když je ESP o 20 dB lepší, sedí u kraje směrem k AP, tedy co
   nejdál od panelu.)
3. Vypnout appku HobbyConnect na telefonu — **jednotka přijímá jediné BLE spojení**,
   takže když ho drží telefon, ESP se nedostane dovnitř.

V našem případě se spojení nakonec obnovilo **samo, ~15 min po nasazení širšího okna**;
tvrdý restart ESP pak navazoval do ~10 s. Které z toho rozhodlo, prokázat nelze.

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
