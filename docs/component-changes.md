# 🧩 Rozšíření komponenty `fendt_caravan` (fork vs PR #13327)

Vycházíme z [ESPHome PR #13327](https://github.com/esphome/esphome/pull/13327)
(autor *rawsludge*), který vystavoval jen výřez (2 teploty, centrální spínače
main/all_lights/floor_heater, napájení, verze SW) a sám uváděl *„missing
implementations like Water Level, Battery Level"* + neměl WRITE příkazy.

Lokální fork je v `esphome/my_components/fendt_caravan/`. Změny:

## 1. Dekodér `decode_number`
`sensor/device_decoders.{h,cpp}` — nový float dekodér: čárka→tečka + parsování čísla
na začátku hodnoty, zvládá mezery i přilepené jednotky (`13,9V`, `0,0 A`, `100%`,
`77,5h`, `14.0^C`). Původně přes `std::stof`, dnes přes `strtof` — viz [§8](#8-robustnost--opravy-pádů-z-review-pr-13327).

## 2. Baterie (`IBS0_*`) ⭐ klíčový objev
`sensor/control_unit_device_sensor.{h,cpp}` — Hobby posílá baterii pod prefixem
`IBS0_`, ne pod `UBAT`/`IBAT_BAL`, které čekal originál (proto se nedekódovala).
Přidány proměnné + senzory: `IBS0_UBAT` (napětí), `IBS0_IBAT` (proud),
`IBS0_SOC2` (nabití %), `IBS0_REMAINING_TIME` (h), `IBS0_TEMPERATURE` (°C).

## 3. Voda
`WATER_LEVEL` jako senzor (`WATER_MEASUREMENT` zatím nepoužito).

## 4. Spínaná světla a relé (WRITE = `cmd-tgl`)
Nové nativní `switch` typy (`switch/__init__.py` + `SUB_SWITCH` v device sensoru):
`kueche`, `aussen`, `amb1`, `amb2`, `amb3`, `dusche` (`LIGHT_DUSCHE`),
`fridge` (`FRIDGE_ON_OFF`), `therme` (`THERME_ON`). Dekódují stav (zpětná vazba)
a togglují přes `cmd-tgl:KEY`.

Plus `wasch`, `kueche2`, `zusatzl`, `zusatzr` — v našem 495 UL neosazené vývody, ale
jiný půdorys je používá (v 460 UFE je `LIGHT_WASCH` kandidát na světlo WC, viz issue #1).
A pro klíč, který katalog nezná, **stačí `key_name` v YAML** — `FendtSwitch::setup()`
si proměnnou v takovém případě založí sám (`decode_bool` + `cmd-tgl`), takže nový
model karavanu už nevyžaduje zásah do C++:

```yaml
switch:
  - platform: fendt_caravan
    parent_id: hobby_unit
    type: wasch
    key_name: LIGHT_COKOLIV_PANEL_POSILA
    name: "Svetlo WC"
```

## 5. Stmívače a lednice (WRITE = `net-KEY-N`) — řešeno v YAML
Stmívače (`LIGHT_DIM2/DIM3`) jako HA `light` (monochromatic + template `output`),
lednice režim/teplota jako `select`/`number` — vše přes `id(hub).on_command_send(...)`
(`net-KEY-N`), bez nutnosti psát nové C++ platformy `light/select/number`
(ty v PR chybí, jen jsou v `AUTO_LOAD`).

## 6. RAW příkazový tester
Template `text` entita volající `on_command_send(x)` — pošle libovolný BLE příkaz
z HA bez nového flashe (klíčové pro reverzní inženýrství WRITE formátů).

## 7. Kompletní katalog protokolu (decode-only, se status anotací)
`control_unit_device_sensor.cpp` má v `setup()` přes `ref()` zaregistrované **všechny
potvrzené klíče** napříč výbavou — vč. Alde `HEATER_*`, Truma `AC_TRUMA_*`, Dometic
`AC_DOM_FJ_*`, `ULTRAHEAT_*`, termostat `TH_*`/`TT_`, satelit `SAT_*`, doplňky
baterie/lednice/světel. Bez HA entity (jen se dekódují → připraveny k navázání),
komentované statusem: `[OK]` odzkoušeno (naše 495 UL) · `[?]` máme/neodzkoušeno ·
`[--]` jiná výbava (nemáme; z dumpu/komunity). Z komponenty je tak nejúplnější
referenční dekodér HobbyConnect — užitečné pro upstream i další karavany.

## 8. Robustnost — opravy pádů z review PR #13327

Maintainer ESPHome nechal 16. 8. 2026 na PR proběhnout `esphbot review`; ten našel
v komponentě **tři cesty, kterými data z panelu shodí ESP** (ESP-IDF překládá ESPHome
s vypnutými výjimkami, takže `throw` = `abort()` = reboot). Autor PR 26. 8. zavřel a
pokračuje v přepsané podobě, náš fork ty chyby ale zdědil — opraveny jsou tady:

| Bylo | Je |
|------|-----|
| `char buffer[25]` v notify handleru plněný `memset`/`memcpy` délkou `value_len` z protistrany (rámec je omezený až MTU, ne 24 B) | žádný pevný buffer — rámec se rovnou skládá do `std::string` |
| skládání víc než dvou rámců zahodilo všechny kromě posledních dvou | `pending` se plní postupně, zpráva je hotová až rámcem bez `@` |
| `std::stof` v `decode_temperature`/`decode_number` → `abort()` na neparsovatelné hodnotě | `strtof` + kontrola, že se něco přečetlo; jinak `NAN` a `ESP_LOGE` (v HA *Unavailable*, ne falešná 0) |
| `list.at(val)` v `decode_int_str` → `abort()` mimo rozsah | kontrola rozsahu + `""` |
| `Variable::decode()` volal `decode_funct_` bez kontroly, ale `HS_KEY`/`HS_KEY_LONG` jsou registrované s `nullptr` | prázdný dekodér se přeskočí (klíč je jen pro zápis); `value_` navíc inicializovaná |
| chunkování příkazů brala každý kus od indexu 0 (a indexy byly `int8_t`) | `split_command_chunks()` posouvá offset, funguje i přes 127 B |
| odeslaný příkaz se z fronty mazal přes `remove()` = **všechny** shodné najednou | maže se jen odeslaná položka, takže dva stejné toggly za sebou projdou oba |
| `decode_int`/`decode_int_str`/`decode_voltage` četly hodnotu přes `from_chars`, který **vyžaduje shodu celého řetězce** → panel posílá `TEMP_IN_OFFSET:  0` nebo `13,9 V` a hodnota tiše spadla na 0 (+ `Data parse error` v logu při každém připojení) | stejné shovívavé parsování jako u čísel (`strtol`/`strtof` + kontrola, že se něco přečetlo) |
| prázdná hodnota se logovala jako `Data parse error` — panel ale takhle běžně posílá klíče, ke kterým nemá co hlásit (`IBS0_UBAT:`, `IBS0_IBAT:` u nečinného čidla baterie, hlášeno z 540 KMFE) | výsledek stejný (`NAN` → v HA *Unavailable*, u celých čísel `0` / `""`), ale bez `E` v logu; chybu hlásí jen nesmyslná hodnota |

Framing (chunkování příkazů + skládání notifikací) je kvůli testovatelnosti vytažený
do `protocol_framing.h` — je to jediná část komponenty bez ESP-IDF typů, takže se dá
přeložit a otestovat na PC: [`tests/host/`](../tests/host/) (25 testů, `./run_tests.sh`).
Proti kódu před opravou jich 7 padalo na chybný výsledek a 6 skončilo pádem procesu.

## 9. Výpadek spojení a odmítnutý zápis (1.3.0)

Druhé `esphbot review` — tentokrát na novém PR [esphome#19091](https://github.com/esphome/esphome/pull/19091)
(23. 9. 2026) — našlo tři věci, které měl i náš fork:

| Bylo | Je |
|------|-----|
| při odpojení se nic nenulovalo: spadlo-li spojení uprostřed vícerámcové zprávy, zůstal nastavený příznak „zbytek zprávy ještě přijde" a **po reconnectu se už nic neodeslalo — ani handshake — až do restartu ESP** | `ESP_GATTC_DISCONNECT_EVT`/`CLOSE_EVT` zahodí rozpracovaný rámec, frontu i stav; nové spojení začíná handshakem |
| příkaz zadaný mimo dosah zůstal ve frontě a **odešel po návratu do dosahu** (třeba přepnutí světla o hodinu později) — a dokonce **před** handshakem | bez spojení se příkaz zahodí s `W` v logu: „Not connected to the caravan, command dropped" |
| odmítnutý zápis (`esp_ble_gattc_write_char` ≠ `ESP_OK`) se zalogoval a příkaz zmizel | zkusí se znovu (max. 3×); pak se zahodí **celý** příkaz, ne jen jeden kus — druhá půlka rozděleného příkazu by byla pro panel nesmysl |
| handshake natvrdo `net-BT_ID-c0:ee:fb:90:b0:a7` (adresa telefonu, ze kterého byl protokol odposlechnut) | adresa jde nastavit volbou `bt_id:` u `fendt_caravan:`; **výchozí zůstává `c0:ee:fb:90:b0:a7`** |

> ⚠️ **Vlastní BT adresu ESP panel nepřijme** (ověřeno 5. 10. 2026): po `net-BT_ID-90:70:69:c2:9f:8a`
> shodí spojení do půl sekundy, ESP se připojuje dokola (35× za pár minut) a data nechodí. Panel
> nejspíš bere jen spárované adresy — proto výchozí hodnota zůstala; `bt_id:` je pro případ,
> že si někdo spáruje vlastní.

Stav jednoho spojení (fronta, skládání rámců, pokusy o zápis) je teď třída `PanelLink`
v `protocol_framing.h`, takže i tohle kryjí [host testy](../tests/host/) — 11 nových,
celkem 36. Ověřeno i živě na 495 UL (12/12): ovládání světla před i po reconnectu, příkaz
mimo spojení zahozen a po návratu nepřehrán, jeden handshake, kompletní výpis `BT_START`…`BT_STOP`. Do configu přibyl spínač **„Karavan BLE povoleno"** (`ble_client` switch):
vypnutím se ESP odpojí a uvolní jediné BLE spojení HobbyConnectu pro oficiální aplikaci.

## 10. Klima Dometic FreshJet — platforma `climate` (jen čtení)

Nová platforma `climate/` (`FendtDometicClimate`, `type: dometic_freshjet`). Pověsí se na
proměnné `AC_DOM_FJ_ENABLE`/`_MODE`/`_FAN_SPEED`/`_TARGETTEMP` (dosud jen decode-only
v katalogu) a na `TEMP_IN` jako aktuální teplotu. Jedna změna na klimě přijde jako dávka
klíčů (změna módu pošle i ventilátor a teplotu) → stav se publikuje jednou, 100 ms po
posledním klíči. Dokud panel neřekl, jestli je klima zapnutá, nepublikuje se nic.
`control()` nic neposílá — formát příkazů není ověřený — jen zaloguje varování a vrátí
v HA poslední stav z panelu. Kód je celý pod `#ifdef USE_CLIMATE`, takže konfigurace bez
`climate:` (náš 495 UL) se překládá beze změny. Ověřeno překladem obou variant v ESPHome
2026.9.1; živě neověřeno (Dometic nemáme) — data z logu Hobby 540 KMFE, viz
[`protocol-keys-full.md`](protocol-keys-full.md).

## 11. Komunikace s panelem — `text_sensor` typu `traffic` (1.4.0)

Testeři s jiným půdorysem (Markus, 540 KMFE) zkoušejí příkazy přes RAW entitu, ale
log ESP z mobilu nepošlou. Nový typ `traffic` publikuje každý příkaz (`→`), zahozený
příkaz (`✗`) a každou zprávu z panelu (`←`) jako stav textové entity, takže odpověď
panelu je vidět v historii entity v HA. Formátování řádků je třída `TrafficLog`
v `protocol_framing.h` (pořadové číslo 1–999, aby HA uložil i dva stejné řádky po sobě;
klíče z `ignore_keys` vynechá, výchozí `TEMP_IN`/`TEMP_OUT`; řádek zkrátí na 255 B,
limit HA, a nikdy uprostřed znaku UTF-8). Kryje ji 9 nových [host testů](../tests/host/),
celkem 47. Hub (`FendtCaravan`) ji volá z `on_command_send`, z příjmu notifikace
a po handshaku; kód kolem entity je pod `#ifdef USE_TEXT_SENSOR` a bez entity v configu
se nic nesestavuje. Ověřeno překladem v ESPHome 2026.9.1.

## Zjištěné WRITE formáty (viz [`ble-protocol.md`](ble-protocol.md))
- on/off (bool): **`cmd-tgl:KEY`** (toggle), `cmd-set:KEY=1` NEfunguje
- stmívače (0–15): **`net-KEY-N`** (univerzální), `cmd-set` jen na DIM2

## Přesnost
`accuracy_decimals` v `sensor/__init__.py` (teploty 1, napětí 1, proud 1, % 0, h 1).
Pozn.: float nese IEEE balast — zobrazenou přesnost dolaďuje HA *display precision*
(viz [`home-assistant.md`](home-assistant.md)); na živé instanci napětí=2.

## Možný příspěvek zpět do PR #13327
Hodnota pro upstream: `IBS0_*` baterie, `WATER_LEVEL`, WRITE formáty (`cmd-tgl`/`net-`),
`decode_number`. Doplnit chybějící C++ platformy `light/select/number` by umožnilo
plně nativní stmívače/lednici bez YAML obezliček.
