#include "fendt_caravan.h"
#include "protocol_framing.h"
#include "esphome/core/application.h"

#ifdef USE_ESP32

namespace esphome::fendt_caravan {

namespace espbt = esphome::esp32_ble_tracker;

using namespace std;
const uint8_t WAIT_COMMAND = 200;
static const char *const TAG = "fendt_caravan";

void FendtCaravan::dump_config() {
  ESP_LOGCONFIG(TAG,
                "Fendt Caravan\n"
                "  Fendt Address: %s",
                this->parent()->address_str());
}

void FendtCaravan::loop() {
  if (!this->command_enabled_)
    return;

  if (this->parent()->state() != espbt::ClientState::ESTABLISHED)
    return;

  uint32_t time_stamp = App.get_loop_component_start_time();
  if (!this->commands_.empty() && (time_stamp - this->last_command_time_) >= WAIT_COMMAND && !this->wait_buffer_) {
    std::string cmd = this->commands_.at(0);
    auto status = esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(),
                                           this->char_handle_, cmd.length(), (uint8_t *) cmd.c_str(),
                                           ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    if (status) {
      ESP_LOGE(TAG, "Writing command failed (%d)", status);
    }

    ESP_LOGD(TAG, "Command sent: %s, %d", cmd.c_str(), cmd.length());
    this->last_command_time_ = time_stamp;
    // drop the command we just sent, not every identical one: two toggles of the same
    // switch in a row are two separate commands and both have to go out
    this->commands_.erase(this->commands_.begin());
  }
}

void FendtCaravan::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGV(TAG, "Service discovery complete");
      auto *service = this->parent()->get_service(espbt::ESPBTUUID::from_raw(service_uuid_));
      if (service == nullptr) {
        ESP_LOGW(TAG, "control service not found at device, not a Fendt Caravan..?");
        break;
      }
      auto *chr = service->get_characteristic(characteristic_uuid_);
      if (chr == nullptr) {
        ESP_LOGW(TAG, "Control characteristic not found");
        break;
      }
      this->char_handle_ = chr->handle;
      auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                                      chr->handle);
      if (status) {
        ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
      }
      auto *descr = chr->get_descriptor(0x2902);
      if (descr == nullptr) {
        ESP_LOGW(TAG, "No CCC descriptor found at service");
        break;
      }
      uint8_t cccd_value[2] = {1, 0};
      status =
          esp_ble_gattc_write_char_descr(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), descr->handle,
                                         2, cccd_value, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
      if (status) {
        ESP_LOGW(TAG, "Error sending CCC descriptor write request, status=%d", status);
        break;
      }

      this->add_command_("net-BT_ID-c0:ee:fb:90:b0:a7");
      this->add_command_("net-BT_VARS");
      this->command_enabled_ = true;
      break;
    }
    case ESP_GATTC_NOTIFY_EVT:
      if (param->notify.handle == this->char_handle_) {
        auto message = feed_notification_chunk(this->last_response_, param->notify.value, param->notify.value_len);
        this->wait_buffer_ = !message.has_value();
        if (message.has_value()) {
          ESP_LOGD(TAG, "Notified value: %s", message->c_str());
          this->on_data_received_(*message);
        }
      }
      break;
    default:
      break;
  }
}
void FendtCaravan::add_command_(const std::string &cmd) {
  for (const auto &chunk : split_command_chunks(cmd))
    this->commands_.push_back(chunk);
}

void FendtCaravan::on_data_received_(const std::string &data) {
  std::string key, value;

  size_t start = 0;
  size_t end = data.find(':');
  key = data.substr(start, end);
  value = data.substr(end + 1);
  if (this->mcu_device_) {
    bool result = this->mcu_device_->decode(key, value);
    if (result)
      return;
  }
}

void FendtCaravan::on_command_send(const std::string &command) {
  ESP_LOGV(TAG, "on_command_send called. Command: %s", command.c_str());
  this->add_command_(command);
}

}  // namespace esphome::fendt_caravan
#endif
