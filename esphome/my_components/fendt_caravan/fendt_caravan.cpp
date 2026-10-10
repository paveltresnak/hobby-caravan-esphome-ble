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
  if (this->parent()->state() != espbt::ClientState::ESTABLISHED)
    return;

  uint32_t time_stamp = App.get_loop_component_start_time();
  if ((time_stamp - this->last_command_time_) < WAIT_COMMAND)
    return;
  const std::string *next = this->link_.next();
  if (next == nullptr)
    return;

  std::string chunk = *next;
  auto status = esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(),
                                         this->char_handle_, chunk.length(), (uint8_t *) chunk.c_str(),
                                         ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  this->last_command_time_ = time_stamp;
  switch (this->link_.on_write(status == ESP_OK)) {
    case WriteOutcome::SENT:
      ESP_LOGD(TAG, "Command sent: %s, %u", chunk.c_str(), (unsigned) chunk.length());
      break;
    case WriteOutcome::RETRY:
      ESP_LOGW(TAG, "Writing command failed (%d), retrying: %s", status, chunk.c_str());
      break;
    case WriteOutcome::GAVE_UP:
      ESP_LOGE(TAG, "Writing command failed (%d) %u times, dropped: %s", status, (unsigned) MAX_WRITE_ATTEMPTS,
               chunk.c_str());
      break;
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

      ESP_LOGD(TAG, "Handshake as %s", this->bt_id_.c_str());
      this->link_.start(this->bt_id_);
      if (this->traffic_enabled_())
        this->publish_traffic_(this->traffic_.sent("handshake (net-BT_ID, net-BT_VARS)"));
      break;
    }
    case ESP_GATTC_NOTIFY_EVT:
      if (param->notify.handle == this->char_handle_) {
        auto message = this->link_.feed(param->notify.value, param->notify.value_len);
        if (message.has_value()) {
          ESP_LOGD(TAG, "Notified value: %s", message->c_str());
          if (this->traffic_enabled_())
            this->publish_traffic_(this->traffic_.received(*message));
          this->on_data_received_(*message);
        }
      }
      break;
    case ESP_GATTC_DISCONNECT_EVT:
    case ESP_GATTC_CLOSE_EVT:
      // a fragment cut off by the drop or a stale command must not survive into the next
      // session; the next service discovery starts a new one with the handshake
      this->link_.stop();
      this->char_handle_ = 0;
      break;
    default:
      break;
  }
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
  bool queued = this->link_.queue(command);
  if (!queued)
    ESP_LOGW(TAG, "Not connected to the caravan, command dropped: %s", command.c_str());
  if (this->traffic_enabled_())
    this->publish_traffic_(queued ? this->traffic_.sent(command) : this->traffic_.dropped(command));
}

// Without a traffic text sensor in the config none of the lines is even built.
bool FendtCaravan::traffic_enabled_() const {
#ifdef USE_TEXT_SENSOR
  return this->traffic_sensor_ != nullptr;
#else
  return false;
#endif
}

void FendtCaravan::publish_traffic_(const std::optional<std::string> &line) {
#ifdef USE_TEXT_SENSOR
  if (line.has_value())
    this->traffic_sensor_->publish_state(*line);
#endif
}

}  // namespace esphome::fendt_caravan
#endif
