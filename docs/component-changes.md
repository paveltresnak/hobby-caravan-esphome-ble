# 🧩 Rozšíření komponenty `fendt_caravan` (fork vs PR #13327)

Vycházíme z [ESPHome PR #13327](https://github.com/esphome/esphome/pull/13327)
(autor *rawsludge*), který vystavoval jen výřez (2 teploty, centrální spínače
main/all_lights/floor_heater, napájení, verze SW) a sám uváděl *„missing
implementations like Water Level, Battery Level"* + neměl WRITE příkazy.

Lokální fork je v `esphome/my_components/fendt_caravan/`. Změny:

## 1. Dekodér `decode_number`
`sensor/device_decoders.{h,cpp}` — nový float dekodér: čárka→tečka + `std::stof`,
zvládá mezery i přilepené jednotky (`13,9V`, `0,0 A`, `100%`, `77,5h`, `14.0^C`).

## 2. Baterie (`IBS0_*`) ⭐ klíčový objev
`sensor/control_unit_device_sensor.{h,cpp}` — Hobby posílá baterii pod prefixem
`IBS0_`, ne pod `UBAT`/`IBAT_BAL`, které čekal originál (proto se nedekódovala).
Přidány proměnné + senzory: `IBS0_UBAT` (napětí), `IBS0_IBAT` (proud),
`IBS0_SOC2` (nabití %), `IBS0_REMAINING_TIME` (h), `IBS0_TEMPERATURE` (°C).

## 3. Voda
`WATER_LEVEL` jako senzor (`WATER_MEASUREMENT` zatím nepoužito).

## 4. Spínaná světla a relé (WRITE = `cmd-tgl`)
Nové nativní `switch` typy (`switch/__init__.py` + `SUB_SWITCH` v device sensoru):
`kueche`, `aussen`, `amb1`, `amb2`, `amb3`, `fridge` (`FRIDGE_ON_OFF`),
`therme` (`THERME_ON`). Dekódují stav (zpětná vazba) a togglují přes `cmd-tgl:KEY`.

## 5. Stmívače a lednice (WRITE = `net-KEY-N`) — řešeno v YAML
Stmívače (`LIGHT_DIM2/DIM3`) jako HA `light` (monochromatic + template `output`),
lednice režim/teplota jako `select`/`number` — vše přes `id(hub).on_command_send(...)`
(`net-KEY-N`), bez nutnosti psát nové C++ platformy `light/select/number`
(ty v PR chybí, jen jsou v `AUTO_LOAD`).

## 6. RAW příkazový tester
Template `text` entita volající `on_command_send(x)` — pošle libovolný BLE příkaz
z HA bez nového flashe (klíčové pro reverzní inženýrství WRITE formátů).

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
