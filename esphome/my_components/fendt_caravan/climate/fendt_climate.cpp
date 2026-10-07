#include "fendt_climate.h"

#if defined(USE_ESP32) && defined(USE_CLIMATE)
#include "esphome/components/fendt_caravan/sensor/device_decoders.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome::fendt_caravan {
static const char *const TAG = "FC.AC";

// One change on the A/C arrives as a burst of keys (a mode change also re-sends fan speed
// and target temperature), so publish once the burst is over instead of once per key.
static const uint32_t PUBLISH_DELAY_MS = 100;

void FendtDometicClimate::setup() {
  this->enable_ = this->string_variable_("AC_DOM_FJ_ENABLE");
  this->mode_ = this->string_variable_("AC_DOM_FJ_MODE");
  this->fan_speed_ = this->string_variable_("AC_DOM_FJ_FAN_SPEED");
  this->target_temp_ = this->string_variable_("AC_DOM_FJ_TARGETTEMP");

  // The A/C does not report a room temperature of its own; the panel's indoor sensor is
  // the closest thing to it.
  auto *temp_in = static_cast<Variable<float> *>(this->get_parent()->get_variable("TEMP_IN"));
  if (temp_in != nullptr) {
    temp_in->set_on_decode_callback([this](const float &value) {
      this->current_temperature = value;
      this->schedule_publish_();
    });
  }
}

Variable<std::string> *FendtDometicClimate::string_variable_(const char *key) {
  auto *variable = static_cast<Variable<std::string> *>(this->get_parent()->get_variable(key));
  if (variable == nullptr) {
    variable = new Variable<std::string>(key, DeviceDecoders::decode_str);
    this->get_parent()->add_variable(variable);
  }
  variable->set_on_decode_callback([this](const std::string &) { this->schedule_publish_(); });
  return variable;
}

void FendtDometicClimate::schedule_publish_() {
  this->set_timeout("publish", PUBLISH_DELAY_MS, [this]() { this->publish_from_panel_(); });
}

void FendtDometicClimate::publish_from_panel_() {
  // Until the panel has said whether the A/C is on, any mode shown would be made up.
  if (!this->enable_->is_active())
    return;

  if (!DeviceDecoders::decode_bool(this->enable_->get_value())) {
    this->mode = climate::CLIMATE_MODE_OFF;
  } else if (this->mode_->is_active()) {
    const std::string mode = this->mode_->get_value();
    if (mode == "Cool") {
      this->mode = climate::CLIMATE_MODE_COOL;
    } else if (mode == "Heat") {
      this->mode = climate::CLIMATE_MODE_HEAT;
    } else if (mode == "Fan") {
      this->mode = climate::CLIMATE_MODE_FAN_ONLY;
    } else if (mode == "Auto") {
      this->mode = climate::CLIMATE_MODE_AUTO;
    } else {
      ESP_LOGW(TAG, "Unknown AC_DOM_FJ_MODE '%s', mode left unchanged", mode.c_str());
    }
  }

  if (this->fan_speed_->is_active()) {
    const std::string fan = this->fan_speed_->get_value();
    if (fan == "Auto") {
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
    } else if (fan == "Slow") {
      this->fan_mode = climate::CLIMATE_FAN_LOW;
    } else if (fan == "Med") {
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
    } else if (fan == "Fast") {
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
    } else {
      // "Max" exists according to the RS232 side of the bus, but has not been seen over BLE.
      ESP_LOGW(TAG, "Unknown AC_DOM_FJ_FAN_SPEED '%s'", fan.c_str());
      this->fan_mode.reset();
    }
  }

  if (this->target_temp_->is_active()) {
    float target = DeviceDecoders::decode_number(this->target_temp_->get_value());
    if (!std::isnan(target))
      this->target_temperature = target;
  }

  this->publish_state();
}

climate::ClimateTraits FendtDometicClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.add_supported_mode(climate::CLIMATE_MODE_OFF);
  traits.add_supported_mode(climate::CLIMATE_MODE_COOL);
  traits.add_supported_mode(climate::CLIMATE_MODE_HEAT);
  traits.add_supported_mode(climate::CLIMATE_MODE_FAN_ONLY);
  traits.add_supported_mode(climate::CLIMATE_MODE_AUTO);
  traits.add_supported_fan_mode(climate::CLIMATE_FAN_AUTO);
  traits.add_supported_fan_mode(climate::CLIMATE_FAN_LOW);
  traits.add_supported_fan_mode(climate::CLIMATE_FAN_MEDIUM);
  traits.add_supported_fan_mode(climate::CLIMATE_FAN_HIGH);
  // Whole degrees, as the panel reports them. The range can be narrowed with `visual:`.
  traits.set_visual_min_temperature(16);
  traits.set_visual_max_temperature(31);
  traits.set_visual_target_temperature_step(1);
  traits.set_visual_current_temperature_step(0.5);
  return traits;
}

void FendtDometicClimate::control(const climate::ClimateCall &call) {
  ESP_LOGW(TAG, "Controlling the Dometic A/C over BLE is not supported yet, request ignored");
  // Snap Home Assistant back to the state the panel reported.
  this->publish_state();
}

void FendtDometicClimate::dump_config() {
  LOG_CLIMATE("", "Dometic FreshJet (read-only)", this);
}

}  // namespace esphome::fendt_caravan
#endif
