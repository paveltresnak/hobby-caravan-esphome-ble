#pragma once

#include "esphome/core/defines.h"

#if defined(USE_ESP32) && defined(USE_CLIMATE)
#include "esphome/components/climate/climate.h"
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/fendt_caravan/climate/dometic_commands.h"
#include "esphome/core/component.h"
#include <optional>
#include <string>

namespace esphome::fendt_caravan {

// Dometic FreshJet roof A/C as the panel reports it (AC_DOM_FJ_*), seen on a Hobby 540 KMFE
// with software V011900. The panel reports every change made on the A/C or its remote;
// control() sends what differs from that (formats in dometic_commands.h).
class FendtDometicClimate : public climate::Climate, public Component, public Parented<CaravanDeviceComponent> {
 public:
  void setup() override;
  void dump_config() override;
  // After the device component (DATA), which registers the variables this one listens to.
  float get_setup_priority() const override { return setup_priority::DATA - 1.0f; }

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

  Variable<std::string> *string_variable_(const char *key);
  void schedule_publish_();
  void publish_from_panel_();
  DometicState panel_state_();

  // ENABLE is a toggle: until the panel confirms a switch on/off, a second request must be
  // compared with what was asked for, not with the stale report - or it toggles back.
  std::optional<bool> pending_on_;

  Variable<std::string> *enable_{nullptr};
  Variable<std::string> *mode_{nullptr};
  Variable<std::string> *fan_speed_{nullptr};
  Variable<std::string> *target_temp_{nullptr};
};

}  // namespace esphome::fendt_caravan
#endif
