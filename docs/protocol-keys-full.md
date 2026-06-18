# 📚 Úplný katalog klíčů HobbyConnect (`BT_START` dump)

Při navázání BLE spojení jednotka vysype **kompletní stav** (`BT_START` … `BT_STOP`).
Tohle je úplný výpis z **Hobby De Luxe 495 UL** (panel `PANEL_VERSION:V011902`,
`SOFTWARE_VERSION:V011100`), zachycený 2026-06-18.

> ⚠️ Jednotka hlásí **všechny** klíče, které firmware zná — i pro výbavu, kterou
> konkrétní karavan **nemá** (hodnota bývá `0` / `Off` / `*_AVAILABLE:0`). V 495 UL
> jsou reálně zapojené jen některé (viz [`ble-protocol.md`](ble-protocol.md)).

## Identifikace / systém
| Klíč | Hodnota v dumpu | Pozn. |
|------|-----------------|-------|
| `PANEL_VERSION` | `V011902` | verze panelu |
| `SOFTWARE_VERSION` | `V011100` | verze SW |
| `VEHICLE_TYPE` | `10` | typ vozidla |
| `GSM_SIGNAL` | `17` | síla GSM |
| `LATITUDE` / `LONGITUDE` | (prázdné) | GPS |
| `BT_START` / `BT_STOP` | — | hranice dumpu |

## 🌡️ Teploty / napájení
`TEMP_IN` `21,5^C`, `TEMP_OUT` `17,5^C`, `TEMP_IN_OFFSET` `0`, `TEMP_OUT_OFFSET` `0`,
`LINE_EN` `1` (230 V), `HS_EN` `0`, `AC_EN` `0`, `HS_KEY_STATE` `2`, `TRADE_SHOW` `0`.

> 🐞 `TEMP_*_OFFSET` má hodnotu `"  0"` (mezery) → `decode_int` (`from_chars`)
> hlásí `Data parse error` (`FC.COD:036`). Pre-existující drobnost, neškodí.

## 🔋 Baterie (`IBS0_` — Intelligent Battery Sensor)
`IBS0_UBAT` `0,0 V`, `IBS0_IBAT` `0,0 A`, `IBS0_SOC2` `0%`, `IBS0_REMAINING_TIME` `0,0 h`,
`IBS0_TEMPERATURE` `0.0^C`, `IBS0_RECALIBRATED` `0`, `IBS0_CAPACITY` `95`, `IBS0_TYPE` `3`,
`IBS0_AVAILABLE` `0`.

> Při provozu (po probuzení IBS) jdou reálné hodnoty (`13,9 V` / `100%` / `77,5 h`).
> V dumpu výše je `*_AVAILABLE:0` → IBS zrovna spal → samé nuly.

## 💧 Voda
`WATER_LEVEL` `0`, `WATER_MEASUREMENT` `0`.

## 💡 Světla (v 495 UL zapojené jen některé)
| Klíč | Pravděpodobně | Zapojeno v 495 UL? |
|------|---------------|--------------------|
| `LIGHT_KUECHE` | kuchyň | ✅ |
| `LIGHT_KUECHE2` | kuchyň 2 | ? |
| `LIGHT_AUSSEN` | venkovní | ✅ |
| `LIGHT_AMB1` / `_AMB2` / `_AMB3` | ambient 1–3 | ✅ |
| `LIGHT_DUSCHE` | sprcha | ? |
| `LIGHT_WASCH` | umývárna | ? |
| `LIGHT_ZUSATZL` / `_ZUSATZR` | přídavné L/P | ? |
| `LIGHT_DIM0`–`LIGHT_DIM4` | stmívače 0–4 | ✅ jen DIM2 (obývák), DIM3 (zadní okno) |
| `LIGHT_BUSY` | příznak „zaneprázdněno" | — |

## 🧊 Lednice
`FRIDGE_AVAILABLE` `1`, `FRIDGE_ON_OFF` `0`, `FRIDGE_MODE` `0`, `FRIDGE_SOURCE` `1`,
`FRIDGE_TEMP` `2`, `FRIDGE_TYPE` `1`.

## 🔥 Topení / bojler / termostat
> ⚠️ **V tomto 495 UL je topení MANUÁLNÍ (žádná Truma 4/6)** → `TH_*` a `HEATER_*`
> se nevyužívají a hodnoty se **per kus liší** (viz křížové ověření níže).
> Climate entitu z `TH_*` proto nestavíme.

- **Termostat (Truma — jen u vozů s ní)** `TH_AVAILABLE`, `TT_AVAILABLE`, `TH_TYPE`,
  `TH_A_EN` (vzduch), `TH_W_EN` (voda), `TH_A_T` (cílová), `TH_W_T`, `TH_ES`, `TH_MM`.
- **Hořák/ohřev** `HEATER_AVAILABLE` `0`, `HEATER_ONOFF` `0`, `HEATER_TEMP` `5.0^C`,
  `HEATER_WATER` `Off`, `HEATER_WATER_TEMP` `50^C`, `HEATER_EL` `Off`, `HEATER_GAS` `On`.
- **Bojler / podlahovka** `THERME_CONFIG` `1`, `THERME_ON` `0`,
  `FLOOR_HEATER_CONFIG` `1`, `FLOOR_HEATER_ON` `0`.
- **UltraHeat** `ULTRAHEAT_AVAILABLE` `0`, `_ONOFF` `Off`, `_POWER` `2000 W`, `_TEMP` `5`.

## ❄️ Klimatizace (vestavěné typy — v 495 UL NEdostupné)
- **Dometic FreshJet** `AC_DOM_FJ_AVAILABLE` `0`, `_ENABLE` `Off`, `_MODE` `Auto`,
  `_TARGETTEMP` `19`, `_FAN_SPEED` `Slow`.
- **Truma** `AC_TRUMA_AVAILABLE` `0` (+ `_TYPE/_ENABLE/_TEMP/_MODE/_FAN_LEVEL/_LIGHT_*/_MM`).

> ℹ️ Sinclair klima řešíme **mimo BLE** (Gree/WiFi) — proto jsou tyhle `AC_*` u nás `0`.

## 📡 Ostatní (nedostupné)
`SAT_*` (satelit, `SAT_AVAILABLE` `0`), `PI_PAIRING` `0`.

## 🔁 Křížové ověření (PR #13327)

Stejnou sadu klíčů potvrzuje i příkladový výpis z [ESPHome PR #13327]
(https://github.com/esphome/esphome/pull/13327) na **jiné** jednotce — liší se jen
hodnoty/výbava, struktura je shodná:

| Klíč | Naše 495 UL | Jednotka z PR | Závěr |
|------|-------------|---------------|-------|
| `SOFTWARE_VERSION` | `V011100` | `V010700` | per kus |
| `PANEL_VERSION` | `V011902` | `V011902` | shodné |
| `TH_AVAILABLE` | `1`* | `0` | výbava per kus |
| `FRIDGE_TYPE` | `1` | `2` | typ lednice per kus |
| `AC_TRUMA_MODE` | `0` | `5` (aktivní) | výbava per kus |
| `IBS0_TEMPERATURE` | `0.0^C` | `14.0^C` | **dekóduje (tečka!)** |
| `HEATER_WATER_TEMP` | `50^C` | `Off` | hodnota i string! |

\* `TH_AVAILABLE` u nás kolísá — topení je manuální, viz výše.

> ✅ Protokol je tedy **stabilní napříč jednotkami** → komponenta je přenositelná na
> další Hobby/Fendt karavany (zapojí se jen entity dle reálné výbavy).

---
*Zachyceno reverzním inženýrstvím; metodika v [`ble-protocol.md`](ble-protocol.md).*
