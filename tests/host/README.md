# 🧪 Host testy komponenty `fendt_caravan`

Testy toho, co v komponentě je čistá logika a jde přeložit na PC bez ESP-IDF:
**BLE framing** (chunkování odchozích příkazů, skládání příchozích notifikací)
a **dekodéry hodnot**.

Proč zrovna tyhle části: panel je cizí zařízení a posílá, co uzná za vhodné.
ESPHome se na ESP-IDF překládá **s vypnutými výjimkami**, takže dekodér, který
„jen" hodí výjimku, ve skutečnosti volá `abort()` — tedy reboot ESP uprostřed
provozu. Přesně tohle našel `esphbot review` na [PR #13327](https://github.com/esphome/esphome/pull/13327)
a náš fork ty chyby zdědil (viz [`docs/component-changes.md` §8](../../docs/component-changes.md)).

## Spuštění

```bash
cd tests/host
./run_tests.sh                 # postaví a pustí všechny testy
./run_tests.sh nazev_testu     # jeden test
CXX=g++ ./run_tests.sh         # jiný překladač (default: clang++, pak g++)
```

Každý test běží jako **vlastní proces**, takže se pád (`abort()`) vypíše jako
`CRASH`, místo aby vzal celou sadu s sebou:

```
  ok      number_tolerates_padding_and_unit_suffix
  FAIL    command_of_three_chunks_advances_offset
            FAIL (line 86): expected "_NAME_FOR_TEST-12@", got "net-VERY_LONG_KEY@"
  CRASH   temperature_of_empty_value_does_not_crash (exit 127)
```

Překládá se s `-fno-exceptions` (jako na ESP-IDF) a `-fstack-protector-all`.

## Co je čím pokryté

| Soubor | Co se testuje |
|--------|---------------|
| `../../esphome/my_components/fendt_caravan/protocol_framing.h` | dělení příkazu po 17 B s `@`, skládání víceRámcových notifikací, rámec delší než MTU 23 |
| `.../sensor/device_decoders.cpp` | `decode_temperature`, `decode_number` (`" 13,9 V"`, `"25,5^C"`), `decode_int_str` mimo rozsah, prázdné a nesmyslné hodnoty |
| `.../variable.h` | `decode()` u klíče registrovaného bez dekodéru (`HS_KEY`) |

`stubs/esphome/core/` obsahuje minimální náhrady hlaviček ESPHome (log, `CallbackManager`),
aby šly zdrojáky komponenty přeložit mimo ESP-IDF. Zbytek komponenty (BLE klient,
entity) se na hostu netestuje — ten ověřuje až `esphome compile` a běžící ESP.
