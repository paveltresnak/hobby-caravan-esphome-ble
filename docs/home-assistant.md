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
| další spínaná světla podle půdorysu — typy `wasch`, `kueche2`, `zusatzl`, `zusatzr`, `decke` (stropní světlo 540 KMFE, od 1.5.0), nebo **vlastní `key_name`** | `cmd-tgl:<KLÍČ>` (proměnná se pro neznámý klíč založí sama) |
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
| `switch.…_ble_povoleno` (od 1.3.0, `entity_category: config`) | **vypnuto = ESP se od karavanu odpojí a znovu se nepřipojí**, dokud se zase nezapne. HobbyConnect drží jen jedno BLE spojení → takhle se uvolní pro **oficiální aplikaci** v telefonu. Hodí se i k vyzkoušení chování při výpadku spojení. Po restartu ESP je zase zapnuto. |
| `sensor.…_ble_signal` | RSSI se čte z advertisingu, a připojený HobbyConnect inzerovat přestane → **za běžného provozu je trvale `unknown`**. Když ale spojení nejede, hodnoty chodí (2026-08-28 jich přišly desítky, -95 až -100 dBm) — je to jediné, z čeho se pozná, **že jednotka vůbec vysílá**. Co ta čísla znamenají pro schopnost se připojit, ověřené nemáme. |

> Stmívače jsou `light` (monochromatic, optimistický stav) — jas 0–255 v HA se mapuje
> na 0–15 panelu. Po restartu ESP se stav světel může lišit, než přijde první notify.

### Komunikace s panelem — testování z mobilu (od 1.4.0)

`text_sensor` typu `traffic` ukazuje **každý příkaz poslaný panelu a každou zprávu,
kterou panel pošle zpátky**, jeden řádek = jeden stav entity:

```
#41 → cmd-tgl:LIGHT_DECKE
#42 ← LIGHT_DECKE:1
#43 ✗ cmd-tgl:LIGHT_DECKE (not connected)
```

Historie té entity v HA (*kliknout na entitu → Historie*, nebo *Deník*) se pak čte
jako DEBUG log. Hodí se, když se zkouší nový příkaz přes `text.…_raw_cmd` a po ruce
je **jen mobil** — log ESP z telefonu jinak rozumně dostat nejde. Pořadové číslo je
tam proto, že HA uloží nový stav jen při změně textu, takže by se druhé stejné
`LIGHT_DECKE:1` jinak ztratilo. `→` = odesláno, `←` = přišlo z panelu, `✗` = příkaz
zahozen, protože ESP nebylo s panelem spojené.

```yaml
text_sensor:
  - platform: fendt_caravan
    type: traffic
    fendt_caravan_id: hobby_caravan_hub   # id hubu `fendt_caravan:`
    name: "Karavan komunikace s panelem"
    # ignore_keys: [TEMP_IN, TEMP_OUT]    # výchozí - teploty chodí několikrát za sekundu
```

Řádky odcházejí po jednom **každých 250 ms** — API ESPHome by jinak z rychlé dávky
poslal jen poslední. Celý výpis panelu tak do HA dobíhá ~25 s.

⚠️ Panel při každém spojení a při každém `net-BT_VARS` (v našem configu každých
10 min) pošle **celý výpis proměnných** — u 495 UL 95 zpráv, u 540 KMFE 75. Kdo entitu nechá
trvale zapnutou, plní si databázi HA. V našem configu je proto `disabled_by_default: true`
a zapíná se v *Nastavení → Zařízení a služby → Entity* jen na dobu testování.
Bez téhle entity v configu komponenta řádky ani nesestavuje.

### Klima Dometic FreshJet (od 1.5.0 i ovládání)

Pro karavany s vestavěnou klimou Dometic FreshJet na sběrnici (viděno v Hobby 540 KMFE,
SW `V011900`; v 495 UL není). Entita `climate` ukazuje, co panel hlásí — zapnuto/vypnuto,
mód, ventilátor, cílovou teplotu — a jako aktuální teplotu vnitřní čidlo panelu (`TEMP_IN`),
protože klima vlastní teplotu nehlásí.

```yaml
climate:
  - platform: fendt_caravan
    parent_id: hobby_unit        # id senzoru typu mcu_device
    type: dometic_freshjet
    name: "Karavan klima"
```

| Panel (`AC_DOM_FJ_*`) | Home Assistant |
|---|---|
| `ENABLE` `Off` | mód *Off* |
| `MODE` `Cool` / `Heat` / `Fan` / `Auto` | *Cool* / *Heat* / *Fan only* / *Auto* |
| `FAN_SPEED` `Auto` / `Slow` / `Med` / `Fast` | *Auto* / *Low* / *Medium* / *High* |
| `TARGETTEMP` | cílová teplota (celé °C, rozsah v HA 16–31, jde zúžit přes `visual:`) |

**Ovládání z HA (od 1.5.0)** posílá příkazy, které majitel 540 KMFE ověřil 10. 10. 2026
přes RAW entitu a 12. 10. 2026 přímo z karty klimy (panel na každý odpověděl do sekundy
změněným klíčem):

| Změna v HA | Příkaz |
|---|---|
| zapnout / vypnout | `cmd-tgl:AC_DOM_FJ_ENABLE` |
| mód | `net-AC_DOM_FJ_MODE-Cool` (`Heat` / `Fan`) |
| ventilátor | `net-AC_DOM_FJ_FAN_SPEED-Slow` (`Auto` / `Med` / `Fast`) |
| cílová teplota | `cmd-set:AC_DOM_FJ_TARGETTEMP=21` |

Posílá se jen to, co se liší od stavu z panelu. U zapnutí/vypnutí je to nutnost:
`ENABLE` je **přepínač**, takže „vypnout" u vypnuté klimy by ji zapnulo. Dokud panel
neřekl, jestli je klima zapnutá, zapnutí ani vypnutí se nepošle (varování v logu).
Požadavek se v HA ukáže hned; když ho panel do 5 s nepotvrdí, HA se vrátí na stav
z panelu. Ověřené jsou módy `Cool`, `Heat`, `Fan` a rychlosti `Slow`, `Med`, `Fast`;
`Auto` u ventilátoru je slovo, které panel sám hlásí.

**Mód Auto v HA chybí záměrně (od 1.5.1).** Panel ho přes BLE nepřijme: na
`net-AC_DOM_FJ_MODE-Auto` odpoví `AC_DOM_FJ_MODE:Fan` a klima jede jen jako ventilátor.
Kdyby panel někdy Auto sám nahlásil (nastavený jinak než přes BLE), karta ho zobrazí.
Rychlost `Max` (zná ji sériová strana sběrnice, přes BLE ji zatím nikdo
neviděl) se zobrazí jako prázdná + varování v logu.

## Logování — jak moc je log ESP upovídaný

Komponenta žádné vlastní nastavení nemá, řídí to standardní blok
[`logger:`](https://esphome.io/components/logger.html) ESPHome. Úrovně od nejtišší:
`NONE < ERROR < WARN < INFO < CONFIG < DEBUG < VERBOSE < VERY_VERBOSE`.

- `level:` je **strop při překladu** — co je nad ním, se do firmwaru vůbec nedostane.
  Prohlížeč logu si pak může zvolit stejnou nebo nižší úroveň, vyšší ne.
- `logs:` umí pro jednotlivé tagy úroveň **jen snížit** (nad `level:` to nejde).

Co komponenta píše:

| úroveň | co | tag |
|---|---|---|
| `[E]` | `Data parse error`, příkaz se nepodařilo zapsat ani na 3. pokus a zahodil se | `FC.COD`, `fendt_caravan` |
| `[W]` | příkaz zahozen, protože ESP nebylo spojené; opakovaný zápis; klima nepotvrdila zapnutí/vypnutí; neznámý mód/rychlost ventilátoru | `fendt_caravan`, `FC.AC` |
| `[C]` | přehled konfigurace (senzory, jednotky, State Class …) | `FC.CU` aj. |
| `[D]` | **`Notified value:`** (každá zpráva z panelu), **`Command sent:`**, `Handshake as` | `fendt_caravan` |
| `[V]` | podrobnosti pro ladění komponenty | různé |

**Chyba je jen `[E]`, varování `[W]`.** Řádky `[C]` ESPHome vypíše pokaždé, když se
k ESP připojí prohlížeč logu; barevně jsou zvýrazněné (fialově), takže je snadné je
splést s chybou — nic neznamenají. S každou další entitou (klima, komunikace
s panelem …) jich přibude.

Náš `esphome/hobby-caravan.yaml` má `level: DEBUG`, protože `Notified value:` /
`Command sent:` jsou hlavní nástroj při mapování protokolu. Pro běžný provoz stačí
ztlumit komponentu a `[D]` zůstane jen pro zbytek:

```yaml
logger:
  level: DEBUG             # strop nechat, ať jde při ladění zase přidat
  logs:
    fendt_caravan: INFO    # ztlumí Notified value / Command sent / Handshake
```

Kdo nepotřebuje ani přehled konfigurace, dá rovnou `level: INFO` — zmizí `[C]` i `[D]`
a zůstanou jen `[I]`, `[W]`, `[E]`. Pro hledání chyby pak stačí vrátit `DEBUG` a nahrát
přes OTA.

- Řádky `[S][sensor] '…' >> hodnota` **nepíše ESP**, přidává je prohlížeč logu ze stavů
  entit, takže `logs:` na ně nemá vliv. V příkazové řádce je vypne `esphome logs … --no-states`
  (nebo proměnná `ESPHOME_LOG_STATES=false`).
- Na zkoušení příkazů z mobilu, kde log nejde rozumně číst, je lepší
  [komunikace s panelem](#komunikace-s-panelem--testování-z-mobilu-od-140) (od 1.4.0) — ukáže
  totéž co `[D]` řádky komponenty, ale přímo v historii entity v HA.
- ⚠️ `VERY_VERBOSE` nepoužívat a nenechávat otevřených víc prohlížečů logu najednou
  (~5 a víc) — ESP32-C3 se zahltí, API spojení se resetuje a HA přestane dostávat data.
  Viz [`flashing.md`](flashing.md).

## Dostupnost & „mimo dosah"
`binary_sensor.…_spojeni` (**device_class `connectivity`**) ukazuje **pravdivý stav
BLE spojení** ESP ↔ HobbyConnect. ⚠️ Nezaměňovat se `sensor.…_napajeni`, což je
`LINE_EN` = **230 V síť**, nikoli stav spojení!

Když karavan **není v dosahu** (BLE odpojeno déle než 20 s — debounce `delayed_off`):
- `binary_sensor.…_spojeni` → **Disconnected**;
- měřené senzory (teploty, baterie, voda) → **Unavailable** (ESP na `on_disconnect`
  publikuje `NaN`), `napajeni` se vyprázdní → v HA není vidět zastaralá „poslední"
  hodnota;
- ovládací entity sice v UI zůstanou, ale **příkazy se zahodí** — v logu
  `Not connected to the caravan, command dropped: …`. Od 1.3.0 se **ani po návratu do
  dosahu nepošlou** (dřív čekaly ve frontě a světlo se mohlo přepnout o hodinu později);
- dashboard ukáže **badge „Karavan BLE"** + podmíněnou kartu *„Karavan není v dosahu…"*.

Po návratu do dosahu se vše **doplní automaticky** (jednotka po připojení vysype
všechny klíče; `on_connect` navíc vyžádá `net-BT_VARS`). Od 1.3.0 se při odpojení zahodí
i napůl přijatá zpráva — dřív mohl výpadek uprostřed ní **zablokovat odesílání až do
restartu ESP** (viz [`component-changes.md` §9](component-changes.md)).

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

**Co se dá zjistit (a co ne):**
- `sensor.…_ble_signal` chodí **právě v odpojeném stavu** (připojené zařízení už
  neinzeruje), takže z něj poznáš, že **jednotka vysílá a je slyšet** — tedy že to není
  vybitá/mrtvá jednotka. Naměřeno bylo **-95 až -100 dBm**; den předtím, kdy se spojení
  navázalo, **-92 až -95 dBm**. 🧪 **Že by tenhle rozdíl byl příčinou, prokázané není** —
  jsou to dva dny měření a spojení se nakonec obnovilo, aniž bychom to doměřili.
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
