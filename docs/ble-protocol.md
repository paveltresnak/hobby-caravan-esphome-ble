# 📡 BLE protokol HobbyConnect — reverzní inženýrství

Zachyceno z reálného provozu karavanu **Hobby De Luxe 495 UL** (panel *HobbyConnect /
Hobby BT Masterpanel MoCa 6121980022*, čip BLUENRG-M0L). Stejný modul je i ve Fendt.

## Připojení

| Položka | Hodnota |
|---------|---------|
| BLE název | `HobbyConnect Data` |
| MAC | `XX:XX:XX:XX:XX:XX` (najdi si svůj — sken; náš měl příznak random/private, přesto stabilní) |
| Service UUID | `C7841029-FE7C-4894-8532-F97908EF1AE4` |
| Characteristic | `0x0001` — **NOTIFY + WRITE** |
| Po připojení | komponenta pošle `net-BT_ID-…` a `net-BT_VARS` → jednotka začne notifikovat |

## Formát rámce

Textové dvojice **`KLÍČ: hodnota`** (např. `TEMP_OUT: 25,5^C`). Desetinná čárka je
**čárka**, jednotky bývají přilepené (`13,9V`, `0,0A`, `100%`, `77,5h`, `25,5^C`).
Delší zprávy jsou **chunkované po 17 bajtech** s koncovým `@` (most je skládá zpět).

## 🗺️ Mapa klíčů (čtení)

### Teploty a napájení
| Klíč | Význam | Příklad |
|------|--------|---------|
| `TEMP_IN` | vnitřní teplota | `25,5^C` |
| `TEMP_OUT` | venkovní teplota | `23,2^C` |
| `LINE_EN` | 230 V připojeno (bool) | `01` |
| `SOFTWARE_VERSION` | verze firmware panelu | `V011100` |
| `HS_KEY_STATE` | stav centrálního tlačítka (0/1/2) | `2` = vše vč. světel |

### 💡 Světla — ✅ kompletně zmapováno (495 UL, SW V011100)
> **✅ Ovládání z HA ověřeno živě 2026-06-19:** stmívání postele (`DIM0`/`DIM1`) i koupelna (`LIGHT_DUSCHE`, on/off) z HA **reagují**.

**Stmívatelná** (`net-LIGHT_DIMx-N`, N = 0–15):
| Klíč | Světlo |
|------|--------|
| `LIGHT_DIM0` | postel vpravo |
| `LIGHT_DIM1` | postel vlevo |
| `LIGHT_DIM2` | obývák |
| `LIGHT_DIM3` | ambient zadní okno |
| `LIGHT_DIM4` | — (nezapojeno) |

**On/off** (`cmd-tgl:KLÍČ`):
| Klíč | Význam |
|------|--------|
| `LIGHT_KUECHE` | kuchyň |
| `LIGHT_AUSSEN` | venkovní |
| `LIGHT_AMB1` | ambient **ložnice** — kolem horních skříněk |
| `LIGHT_AMB2` | ambient **obývák** — kolem horních skříněk |
| `LIGHT_AMB3` | **hlavní světlo** — skleněná obruč nad konferenčním stolem |
| `LIGHT_DUSCHE` | koupelna |
| `LIGHT_WASCH` / `LIGHT_ZUSATZL` / `LIGHT_ZUSATZR` | jen relé, bez svítidla (neosazené vývody) |
| `LIGHT_KUECHE2` | — (nic) |

> **Rozložení (495 UL):** vepředu (směrem k oji) **ložnice** → uprostřed **kuchyň + koupelna** → vzadu **obývák**.
> `LIGHT_DIM3` „ambient zadní okno" = okno **vzadu v obýváku**. Postelová světla (`DIM0`/`DIM1`) jsou v přední ložnici.

> ⚠️ **Firmware-version gotcha:** `LIGHT_DIMx` (remote stmívání) jsou přes BLE vystavené jen ve **starší SW `V011100`**; novější `V012000` je NEvystavuje (stmívání jen lokálně na panelu) — viz [hobby-caravan-pi#1](https://github.com/AndreasKirsten79/hobby-caravan-pi/issues/1).
> Pozn.: některá světla mohou být **mimo BLE sběrnici** (vlastní vypínač → žádný notify) a nelze je ovládat.

### 🧊 Lednice — ✅ ovládání OVĚŘENO (Dometic 10, 2026-06-19)
| Klíč | Význam |
|------|--------|
| `FRIDGE_ON_OFF` | zap/vyp (0/1) — WRITE `cmd-tgl:FRIDGE_ON_OFF` |
| `FRIDGE_SOURCE` | zdroj **0=Auto / 1=Plyn / 2=12V / 3=230V** — WRITE `net-FRIDGE_SOURCE-N` |
| `FRIDGE_TEMP` | stupeň **1–5** (5=max chlazení, NE °C) — WRITE `net-FRIDGE_TEMP-N` |
| `FRIDGE_MODE` | režim (auto/manual?) |
| `FRIDGE_TYPE` / `FRIDGE_AVAILABLE` / `FRIDGE_ERROR` | typ / dostupnost / chyba |

### 🔥 Topení / voda
| Klíč | Význam |
|------|--------|
| `FLOOR_HEATER_ON` | podlahové topení |
| `THERME_ON` | bojler |
| `THERME_CONFIG`, `WASTE_WATER_HEATER_CONFIG` | konfigurace |

### 🔋 Baterie — prefix `IBS0_` (Intelligent Battery Sensor) ⭐ NÁŠ OBJEV
Posílá se **na změnu** (vyvoláno např. odpojením 230 V). Komponenta `fendt_caravan`
původně čekala `UBAT`/`IBAT_BAL`, ale Hobby reálně posílá `IBS0_*` → proto se
nedekódovalo. **Nikde jinde na netu nepopsáno.**

| Klíč | Význam | Příklad |
|------|--------|---------|
| `IBS0_UBAT` | napětí baterie | `13,9V` |
| `IBS0_IBAT` | proud (±, nabíjení/vybíjení) | `0,0A` |
| `IBS0_SOC2` | nabití | `100%` |
| `IBS0_REMAINING_TIME` | zbývající čas | `77,5h` |
| `IBS0_TEMPERATURE` | teplota baterie | `0.0^C` (pozor: **tečka**, ne čárka) |
| `IBS0_CAPACITY` | kapacita | `95` |
| `IBS0_TYPE` / `IBS0_AVAILABLE` / `IBS0_RECALIBRATED` | typ / dostupnost / rekalibrace | `3` / `0` / `0` |

> Pozn.: v `BT_START` dumpu jsou hodnoty s mezerami a mezerou před jednotkou
> (`IBS0_UBAT:  0,0 V`, `  0%`, `   0,0 h`). Dekodér `decode_number` to zvládá
> (`stof` přeskočí mezery a ignoruje jednotku).

### 💧 Nádrž vody — ✅ ZACHYCENO (BT_START dump 2026-06-18)
| Klíč | Význam | Příklad |
|------|--------|---------|
| `WATER_LEVEL` | hladina pitné vody | `0` (nádrž prázdná) |
| `WATER_MEASUREMENT` | měření / surová hodnota | `0` |

Stupnice zatím neznámá (nádrž byla prázdná; kapacita **120 l** — zvětšeno z originálních 25 l) — **ověřit rozsah při napouštění**.

> 📚 Úplný výpis **všech** klíčů jednotky (vč. nepoužitých v 495 UL) je v
> [`protocol-keys-full.md`](protocol-keys-full.md).

## ✍️ Ovládací (WRITE) příkazy — ověřeno živým testem 2026-06-18

Příkaz se zapisuje do téže charakteristiky. Formáty potvrzené křížovým testem
(jas nahoru jednou metodou, dolů druhou → blikání = obě metody fungují):

| Akce | Formát | Poznámka |
|------|--------|----------|
| **Přepnout on/off** (světla, lednice, bojler, podlahovka) | `cmd-tgl:KLÍČ` | **TOGGLE** — překlopí stav, není to „set". Ověřeno kuchyní. |
| **Nastavit jas stmívače** | `net-KLÍČ-N` (N = 0–15) | **Univerzální** pro `LIGHT_DIM2` i `LIGHT_DIM3`. |
| **Lednice — zdroj** | `net-FRIDGE_SOURCE-N` | 0=Auto / 1=Plyn / 2=12V / 3=230V ✅ |
| **Lednice — teplota** | `net-FRIDGE_TEMP-N` | N = 1–5 (5 = max chlazení) ✅ |
| Centrální tlačítko | `net-HS_KEY` / `net-HS_KEY_LONG` | krátký/dlouhý stisk |

⚠️ **Pozor na pasti:**
- `cmd-set:KLÍČ=1` pro **bool NEFUNGUJE** (musí `cmd-tgl`).
- `cmd-set:KLÍČ=N` funguje **jen na `LIGHT_DIM2`**, na `LIGHT_DIM3` ne → proto pro
  stmívače používej výhradně **`net-`**.
- ✅ Lednice OVĚŘENO živě (Dometic 10): `net-FRIDGE_SOURCE-N` (0=Auto/1=Plyn/2=12V/3=230V),
  `net-FRIDGE_TEMP-N` (1–5, 5=max), `cmd-tgl:FRIDGE_ON_OFF`; bojler `cmd-tgl:THERME_ON`.
- Nezahlcuj BLE — víc příkazů rychle za sebou se může ztratit (chunkování + congestion).

## 🔬 Metodika RE (jak jsme to zmapovali)

1. Most (komponenta `fendt_caravan`) loguje **každý** notify na úrovni `DEBUG`
   (`fendt_caravan.cpp` → `ESP_LOGD("Notified value: %s")`).
2. **Diferenciálně** se přepínají **fyzické** vypínače v karavanu (ne z appky — ESP
   drží jediné BLE spojení) a porovnávají se klíče před/po.
3. WRITE formáty: **RAW tester** = template `text` entita v HA, jejíž `set_action`
   volá `id(hub).on_command_send(x)` → lze poslat libovolný příkazový string bez
   nového flashe. Křížový oscilační test odhalí, který formát skutečně nastavuje hodnotu.

> 💡 `VERY_VERBOSE` nepoužívat — zahltí API log stream ESP32-C3 a způsobí reset.
> Po sobě uklízet procesy `esphome logs` (>5 souběžných přetíží C3 API).
