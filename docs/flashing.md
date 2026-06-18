# ⚡ Flashování a provoz

## 1. Příprava
- ESPHome (zde jako Docker kontejner na Synology NAS, dashboard `http://NAS:6052`).
- `esphome/secrets.example.yaml` → zkopíruj na `esphome/secrets.yaml`, doplň Wi-Fi
  (2,4 GHz!). **`secrets.yaml` necommituj.**

## 2. První flash (USB)
Custom (externí) komponenta nejde nahrát úplně přes prohlížeč při úplně prvním
flashi, proto:
1. [web.esphome.io](https://web.esphome.io) → *Prepare for first use* → nahraj přes
   USB základní ESPHome firmware a zadej Wi-Fi (2,4 GHz).
2. V ESPHome dashboardu zařízení **adoptuj** jako `hobby-caravan` a nahraj
   `esphome/hobby-caravan.yaml`.

## 3. Další aktualizace (OTA)
```bash
esphome run esphome/hobby-caravan.yaml --device <IP_ESP>
```
Na NASu přes kontejner:
```bash
docker exec esphome esphome run hobby-caravan.yaml --device <IP_ESP> --no-logs
```

## 4. Komponenta `fendt_caravan`
YAML používá **lokální** rozšířenou kopii:
```yaml
external_components:
  - source: { type: local, path: my_components }
    components: [fendt_caravan]
```
(Origin je [ESPHome PR #13327](https://github.com/esphome/esphome/pull/13327),
ale ten zatím nemá WRITE příkazy ani plnou paritu — proto fork v `my_components/`.)

## 🐞 Gotchas (z praxe)
- **Boot crash / safe-mode rollback**: po prvním OTA s velkým fw může C3 10× rebootovat
  do 60 s a vrátit se na esphome.web. Většinou přechodné (slabé napájení) — zopakuj OTA
  u PC se stabilním 5 V.
- **`VERY_VERBOSE` nepoužívat** — zahltí API log stream C3 přes Wi-Fi a způsobí reset.
  Na mapování stačí `DEBUG` (most loguje každý notify `Notified value:`).
- **Uklízej `esphome logs` procesy** — víc než ~5 souběžných klientů přetíží API C3
  („Connection reset by peer", data v HA zamrznou). Na NASu: `docker restart esphome`.
- **Po OTA HA někdy zamrzne ESPHome spojení** (staré hodnoty) → *Reload* integrace
  ESPHome (Nastavení → Zařízení a služby → ESPHome → ⋮ → Načíst znovu).
