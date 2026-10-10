# 📚 Úplný katalog klíčů HobbyConnect (`BT_START` dump)

Při navázání BLE spojení jednotka vysype **kompletní stav** (`BT_START` … `BT_STOP`).
Tohle je úplný výpis z **Hobby De Luxe 495 UL** (panel `PANEL_VERSION:V011902`,
`SOFTWARE_VERSION:V011100`), zachycený 2026-06-18.

> ⚠️ Jednotka hlásí **všechny** klíče, které firmware zná — i pro výbavu, kterou
> konkrétní karavan **nemá** (hodnota bývá `0` / `Off` / `*_AVAILABLE:0`). V 495 UL
> jsou reálně zapojené jen některé (viz [`ble-protocol.md`](ble-protocol.md)).
> Jiné modely ale můžou hlásit i klíče, které v tomhle výpisu **vůbec nejsou** — viz
> [Klíče z jiných modelů](#-klíče-z-jiných-modelů-hlášené-uživateli).

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
| `LIGHT_DUSCHE` | koupelna / sprcha | ✅ (ovládání ověřeno živě) |
| `LIGHT_WASCH` | umývárna / **WC** | ✗ neosazeno — ale v **Hobby 460 UFE je to světlo WC**, potvrzeno uživatelem ([issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1)) |
| `LIGHT_ZUSATZL` / `_ZUSATZR` | přídavné L/P | ? |
| `LIGHT_DIM0`–`LIGHT_DIM4` | stmívače 0–4 | ✅ jen DIM2 (obývák), DIM3 (zadní okno) |
| `LIGHT_BUSY` | příznak „zaneprázdněno" | — |

### 🔎 Klíče z jiných modelů (hlášené uživateli)

V dumpu 495 UL **nejsou** — hlásí je panel jiného karavanu (zjištěno z jejich logu při
fyzickém přepnutí vypínače). Komponenta je umí bez zásahu do C++ přes `key_name`.

| Klíč | Model | Co to je | Zdroj |
|------|-------|----------|-------|
| `LIGHT_DECKE` | Hobby 540 KMFE (SW `V011900`) | stropní světlo | [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |
| `LIGHT_THERME` | Hobby 540 KMFE (SW `V011900`) | bojler — **místo** `THERME_ON` z 495 UL | [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |
| `LIGHT_WAND` | (SW `V012000`); Hobby 540 KMFE (SW `V011900`) | světlo, `0`/`1` — tahle verze nevystavuje `LIGHT_DIMx`; v 540 KMFE „Ambiente Eltern“ (ambient ložnice) | [hobby-caravan-pi#1](https://github.com/AndreasKirsten79/hobby-caravan-pi/issues/1), [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |
| `LIGHT_BETTR` | Hobby 540 KMFE (SW `V011900`) | postel vpravo — v 495 UL je to `LIGHT_DIM0` | [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |
| `LIGHT_BETTL` | Hobby 540 KMFE (SW `V011900`) | postel vlevo — v 495 UL je to `LIGHT_DIM1` | [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |
| `LIGHT_ZUSATZM` | Hobby 540 KMFE (SW `V011900`) | přídavné světlo uprostřed (*Zusatzlicht Mitte* dle [BastyJuice](https://github.com/BastyJuice/Hobby_Caravan_Serial_2_CI-BUS_Connectivity)), hlásí `1`/`0` | [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |
| `LIGHT_FUSSB` | Hobby 540 KMFE (SW `V011900`) | **podlahové topení** (potvrdil majitel; v 495 UL je to `FLOOR_HEATER_ON`), hlásí `1`/`0` — [BastyJuice](https://github.com/BastyJuice/Hobby_Caravan_Serial_2_CI-BUS_Connectivity) ho vede jako *Fußbodenlicht*, to neplatí | [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1) |

> 📋 **Dump 540 KMFE** (`PANEL_VERSION` `V011902` — stejný panel jako 495 UL, `SOFTWARE_VERSION`
> `V011900`): světla v pořadí `DECKE`, `WAND`, `BETTL`, `BETTR`, `DUSCHE`, `WASCH`, `AMB1`–`3`,
> `ZUSATZL`, `ZUSATZM`, `ZUSATZR`, `KUECHE`, `AUSSEN`, `THERME`, `FUSSB` — **žádné `LIGHT_DIMx`**
> (stejně jako `V012000`). Stropní světlo, spoty u stolu a obě světla u postele uživatel
> stmívá podržením tlačítka na panelu, ale **stmívání nevyvolá žádný notify**.

> 🧪 **Ovládání v 540 KMFE (SW `V011900`)**: prvních pět klíčů výše uživatel spíná
> `net-<KLÍČ>-15` (zap) / `net-<KLÍČ>-0` (vyp) — **nastavení stavu, ne přepnutí**
> (`cmd-tgl`), ověřeno jen jeho template switchi, bez zpětného čtení stavu.
> `15` je horní mez stmívací škály `LIGHT_DIMx` (0–15) → světla v tomhle firmwaru
> nejspíš mají vlastní jméno místo `DIMx` a jdou i stmívat (neověřeno).
> ✅ **Hlášený stav je `1`/`0`, ne `15`** (log 7. 10. 2026, přepínání na panelu): `DECKE`, `WAND`,
> `ZUSATZL/M/R`, `AMB1–3`, `FUSSB` — switch komponenty tedy stav čte správně (uživatel ho už
> používá pro `ZUSATZR`/`AMB1–3`). Stmívání podržením tlačítka hlásí jen zap/vyp, ne úroveň.
> ❌ **Stmívat přes BLE nejde**: `net-LIGHT_DECKE-7` světlo neztlumí (uživatel 7. 10. 2026) —
> stejně jako u SW `V012000`. Dálkově stmívat jde jen na `V011100` přes `LIGHT_DIMx`.
> 🧪 Otevřené: projde u těchto klíčů `cmd-tgl:<KLÍČ>` (to posílá switch komponenty)?

> ⚠️ Sadu klíčů tedy určuje **model a verze firmwaru**, ne jen výbava. Když ti něco
> chybí, pusť `esphome logs`, přepni to fyzicky v karavanu a hledej řádek
> `Notified value: <KLÍČ>:<hodnota>`.

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

### Dometic FreshJet v Hobby 540 KMFE (SW `V011900`, log 7. 10. 2026, [issue #1](https://github.com/paveltresnak/hobby-caravan-esphome-ble/issues/1))

Klima je na sběrnici: `AC_DOM_FJ_AVAILABLE` `1` (v dumpu 6. 10. bylo `0`; 7. 10. jednou
krátce `0` a za 10 s zase `1`, ještě před zapnutím klimy — na čem to závisí, nevíme).
Každou změnu z panelu/ovladače panel pošle hned:

| Klíč | Hodnoty | Pozn. |
|------|---------|-------|
| `AC_DOM_FJ_ENABLE` | `On` / `Off` | současně `AC_EN` `1` / `0` |
| `AC_DOM_FJ_MODE` | `Auto`, `Cool`, `Heat`, `Fan` | při změně módu přijde i `_FAN_SPEED` a `_TARGETTEMP` |
| `AC_DOM_FJ_TARGETTEMP` | celé °C (v logu 16–22) | |
| `AC_DOM_FJ_FAN_SPEED` | `Auto`, `Slow`, `Med`, `Fast` (+ `Max` dle BastyJuice) | |

Vlastní teplotu klima nehlásí → pro `climate` entitu by byla aktuální teplota `TEMP_IN`.
**Zápis přes BLE ✅ ověřen 10. 10. 2026** (majitel 540 KMFE, entita „komunikace
s panelem"): `cmd-tgl:AC_DOM_FJ_ENABLE` · `net-AC_DOM_FJ_MODE-Cool` ·
`net-AC_DOM_FJ_FAN_SPEED-Slow` · `cmd-set:AC_DOM_FJ_TARGETTEMP=21` — mód a ventilátor
**textem**, ne číslem. Entita `climate` (`type: dometic_freshjet`) je od 1.5.0 ovládá,
viz [`home-assistant.md`](home-assistant.md).
⚠️ `AC_TRUMA_AVAILABLE` jednou přišlo s nečitelným bajtem (`\xc1`), pak `1` / `_TYPE` `10` — Trumu uživatel nemá.

### 🔌 Nezávislý zdroj: CI-Bus přes RS232 ([BastyJuice](https://github.com/BastyJuice/Hobby_Caravan_Serial_2_CI-BUS_Connectivity), od dubna 2026)

Projekt čte tutéž sadu proměnných **přímo z LSG** (Lichtsteuermodul, piny „WLAN") přes RS232
115200 Bd místo BLE — proměnné tam mají čísla (`$8` = `AC_DOM_FJ_FAN_SPEED`, `$73` =
`LIGHT_KUECHE`…), názvy jsou shodné s naším BLE dumpem. Ovládání po sériové lince:
`!tgl:<id>` (přepnout), `!set:<id>=<hodnota>` (nastavit), `!run:30`/`!run:31` (hlavní vypínač
zap/vyp). Klima: `!set:10=Auto|Heat|Cool|Fan` (mód), `!set:8=Auto|Slow|Med|Fast|Max`
(ventilátor), `!set:11=22` (cílová teplota). Že zápis po lince funguje, potvrdil i další
uživatel (issue #1 tamtéž, po výměně kabelu) — ale jen přepínání světel, ne klimu. Pro BLE se nabízí stejná dvojice `cmd-tgl:<KLÍČ>` / `cmd-set:<KLÍČ>=<hodnota>` —
**obdoba, ne ověřený převod** (HobbyConnect může příkazy překládat jinak).
Navíc tam jsou klíče, které náš dump nemá: `CHARGER0_*`/`CHARGER1_*` (nabíječ), `DATE`/`TIME`/
`DATETIME`, `DP_EN` (zapalování), `WAKE_EN`, `IBS0_RECALIBRATED`, `PI_STATE`, a na SW `V011800`
`FRESH_WATER_LEVEL` místo `WATER_LEVEL`. `TH_W_T` (Truma bojler) bere jen `40` / `60`.

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
