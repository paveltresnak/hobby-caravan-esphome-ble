#pragma once
// Host-test stub: ESPHome logging reduced to stderr, so component sources compile
// outside the ESP-IDF toolchain.
#include <cstdio>

// Tests read this to check that a value is rejected without (or with) an error line.
inline int &esphome_test_error_count() {
  static int count = 0;
  return count;
}

#define ESPHOME_LOG_(level, tag, fmt, ...) fprintf(stderr, "[" level "][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, ...) (esphome_test_error_count()++, ESPHOME_LOG_("E", tag, __VA_ARGS__))
#define ESP_LOGW(tag, ...) ESPHOME_LOG_("W", tag, __VA_ARGS__)
#define ESP_LOGI(tag, ...) ESPHOME_LOG_("I", tag, __VA_ARGS__)
#define ESP_LOGD(tag, ...) ESPHOME_LOG_("D", tag, __VA_ARGS__)
#define ESP_LOGV(tag, ...) ESPHOME_LOG_("V", tag, __VA_ARGS__)
#define ESP_LOGCONFIG(tag, ...) ESPHOME_LOG_("C", tag, __VA_ARGS__)
