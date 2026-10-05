
#pragma once

#ifdef USE_ESP32
#include "esphome/core/log.h"
#include "caravan_device_component.h"
#include "esphome/core/component.h"
#include "esphome/core/string_ref.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_client/ble_characteristic.h"
#include "protocol_framing.h"

namespace esphome::fendt_caravan {
using namespace std;

class FendtCaravan : public Component, public ble_client::BLEClientNode {
 public:
  void setup() override{};
  void loop() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void set_address(uint64_t address) { address_ = address; };
  // Address announced in the handshake (`bt_id:`, see __init__.py for the default).
  void set_bt_id(const std::string &bt_id) { this->bt_id_ = bt_id; }
  void set_mcu_device_sensor(CaravanDeviceComponent *mcu_device_sensor) {
    this->mcu_device_ = mcu_device_sensor;
    this->mcu_device_->set_command_send_callback([this](const std::string &cmd) { this->on_command_send(cmd); });
  }
  void dump_config() override;
  void on_command_send(const std::string &command);

 protected:
  void on_data_received_(const std::string &data);

 private:
  const char *service_uuid_ = "C7841029-FE7C-4894-8532-F97908EF1AE4";  // değiştirilmesi gerekebilir
  const uint16_t characteristic_uuid_ = 0x0001;
  uint16_t char_handle_ = 0;
  uint32_t last_command_time_ = 0;
  std::string bt_id_;
  PanelLink link_;
  CaravanDeviceComponent *mcu_device_{nullptr};
};
}  // namespace esphome::fendt_caravan
#endif
