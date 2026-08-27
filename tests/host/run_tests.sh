#!/usr/bin/env bash
# Builds and runs the host tests for the pure-logic parts of the fendt_caravan fork.
#
# Every test runs in its own process on purpose: the bugs these tests cover end in
# abort() (exceptions are disabled on ESP-IDF, so a throwing decoder reboots the ESP),
# and a crashed test should be reported, not take the whole suite down with it.
#
# Usage: ./run_tests.sh [test_name]      CXX=g++ ./run_tests.sh   to pick a compiler
set -u

here="$(cd "$(dirname "$0")" && pwd)"
component="$here/../../esphome/my_components/fendt_caravan"

if [ -z "${CXX:-}" ]; then
  for candidate in clang++ g++ /c/msys64/clang64/bin/clang++ /c/msys64/mingw64/bin/g++; do
    if command -v "$candidate" >/dev/null 2>&1; then
      CXX="$candidate"
      break
    fi
  done
fi
if [ -z "${CXX:-}" ]; then
  echo "No C++ compiler found. Install clang or gcc, or set CXX." >&2
  exit 2
fi

bin="$here/test_fendt_caravan"
[ "$(uname -o 2>/dev/null)" = "Msys" ] && bin="$bin.exe"

echo "Building with $CXX ..."
"$CXX" -std=c++17 -fno-exceptions -fstack-protector-all -Wall -Wno-unused-function \
  -DUSE_ESP32 -I "$here/stubs" -I "$component" \
  "$here/test_fendt_caravan.cpp" "$component/sensor/device_decoders.cpp" -o "$bin" || exit 2

pass=0
failed=0
crashed=0
for name in $("$bin" --list | tr -d '\r'); do
  [ $# -gt 0 ] && [ "$1" != "$name" ] && continue
  output="$("$bin" "$name" 2>&1)"
  code=$?
  case $code in
    0) pass=$((pass + 1)); echo "  ok      $name" ;;
    1) failed=$((failed + 1)); echo "  FAIL    $name"; echo "$output" | sed 's/^/          /' ;;
    *) crashed=$((crashed + 1)); echo "  CRASH   $name (exit $code)" ;;
  esac
done

echo
echo "passed: $pass  failed: $failed  crashed: $crashed"
[ $((failed + crashed)) -eq 0 ]
