# Attribution / NOTICE — komponenta `fendt_caravan`

Tento adresář obsahuje **upravenou kopii** ESPHome komponenty `fendt_caravan`,
původně navrženou v **ESPHome PR #13327** autorem **@rawsludge (Ahmet)**.

- Upstream PR: https://github.com/esphome/esphome/pull/13327
- Původní projekt: **ESPHome** — https://github.com/esphome/esphome
- Copyright (c) 2019 ESPHome a přispěvatelé; `fendt_caravan` © @rawsludge

## Licence (zachována z ESPHome — duální)
ESPHome používá duální licencování a to zde **zůstává v platnosti**:
- Soubory **C++** (`.h`, `.cpp`, `.tcc`) → **GPLv3**
- Soubory **Python** a ostatní → **MIT**

`CODEOWNERS = ["@rawsludge"]` je zachováno v `__init__.py`. MIT licence v kořeni
repozitáře se vztahuje na **náš původní obsah** (dokumentace, dashboard, konfigurace),
**ne** na tuto odvozenou komponentu.

## Naše úpravy (detail v [`../../../docs/component-changes.md`](../../../docs/component-changes.md))
- `decode_number`; baterie `IBS0_*`; `WATER_LEVEL`; nativní spínače (`cmd-tgl`);
  WRITE formáty `cmd-tgl`/`net-`; kompletní katalog protokolu (Alde/Truma/Dometic/
  UltraHeat/TH_/SAT klíče, decode-only, se status anotací).

## Poděkování za reverzní inženýrství protokolu
- **@rawsludge** — původní komponenta + RE protokolu Fendt/HobbyConnect
- **tomjeppis** — zalogování proměnných topení/Truma (`HEATER_*`, `AC_TRUMA_*`) přes nRF Connect
- HA community vlákno *„Hobby Connect for Hobby Caravans"* (#889146)
