#include "device_decoders.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "FC.COD";

float DeviceDecoders::parse_leading_float(const std::string &value) {
  const char *begin = value.c_str();
  char *end = nullptr;
  float result = std::strtof(begin, &end);
  if (end == begin) {
    ESP_LOGE(TAG, "Data parse error. Data: %s", value.c_str());
    return NAN;
  }
  return result;
}

std::optional<int> DeviceDecoders::parse_leading_int(const std::string &value) {
  const char *begin = value.c_str();
  char *end = nullptr;
  long result = std::strtol(begin, &end, 10);
  if (end == begin || result > INT_MAX || result < INT_MIN)
    return std::nullopt;
  return static_cast<int>(result);
}

float DeviceDecoders::decode_temperature(const std::string &data) {
  std::string value = data;
  size_t start = value.find("^C");
  if (start != std::string::npos)
    value.replace(start, 2, "");
  start = value.find(',');
  if (start != std::string::npos)
    value.replace(start, 1, ".");
  return parse_leading_float(value);
}
float DeviceDecoders::decode_voltage(const std::string &data) {
  std::string value = data;
  size_t start = value.find('V');
  if (start != std::string::npos)
    value.replace(start, 1, "");
  start = value.find(',');
  if (start != std::string::npos)
    value.replace(start, 1, ".");
  return parse_leading_float(value);
}

float DeviceDecoders::decode_number(const std::string &data) {
  std::string value = data;
  size_t pos = value.find(',');
  if (pos != std::string::npos)
    value.replace(pos, 1, ".");
  return parse_leading_float(value);
}

int DeviceDecoders::decode_int(const std::string &data) {
  auto result = parse_leading_int(data);
  if (!result) {
    ESP_LOGE(TAG, "Data parse error. Data: %s", data.c_str());
    return 0;
  }
  return result.value();
}

time_t DeviceDecoders::decode_date(const std::string &data) {
  std::istringstream date(data);
  tm tm = {};
  date >> std::get_time(&tm, "%d.%m.%y");
  if (date.fail()) {
    ESP_LOGE(TAG, "Date Parsing failed");
    return 0;
  }
  time_t ret = mktime(&tm);
  return ret;
}
time_t DeviceDecoders::decode_time(const std::string &data) {
  std::istringstream date(data);
  tm tm = {};
  date >> std::get_time(&tm, "%H:%M:%S");
  if (date.fail()) {
    ESP_LOGE(TAG, "Date Parsing failed");
    return 0;
  }
  time_t ret = mktime(&tm);
  return ret;
}
std::string DeviceDecoders::decode_int_str(const std::string &data, const std::vector<std::string> &list) {
  auto result = parse_leading_int(data);
  if (!result) {
    ESP_LOGE(TAG, "Data parse error. Data: %s", data.c_str());
    return "";
  }
  int val = result.value();
  if (val < 0 || static_cast<size_t>(val) >= list.size()) {
    ESP_LOGE(TAG, "Value out of range. Data: %s", data.c_str());
    return "";
  }
  return list[val];
}
}  // namespace esphome::fendt_caravan
#endif
