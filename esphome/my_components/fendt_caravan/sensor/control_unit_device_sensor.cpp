#include "control_unit_device_sensor.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "FC.CU";

void ControlUnitDeviceSensor::setup() {
  auto *network = new Variable<std::string>("LINE_EN", [](const std::string &value) {
    const char *tmp[] = {"Connected", "Disconnected"};
    return DeviceDecoders::decode_bool_str(value, tmp);
  });
  this->add_variable(network);

  auto *main_switch = new Variable<bool>("HS_EN", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(main_switch);

  auto *hs_key = new Variable<bool>("HS_KEY", nullptr, Commands::update_run);
  this->add_variable(hs_key);

  auto *hs_key_long = new Variable<bool>("HS_KEY_LONG", nullptr, Commands::update_run);
  this->add_variable(hs_key_long);

  auto *d_plus = new Variable<bool>("D_PLUS", DeviceDecoders::decode_bool);
  this->add_variable(d_plus);

  auto *battery_loading_status = new Variable<int>("IBAT_BAL", DeviceDecoders::decode_int);
  this->add_variable(battery_loading_status);

  auto *ac_active = new Variable<std::string>("AC_EN", [](const std::string &value) {
    const char *tmp[] = {"Enable", "Disable"};
    return DeviceDecoders::decode_bool_str(value, tmp);
  });
  this->add_variable(ac_active);

  auto *alarm_clock_active = new Variable<bool>("WAKE_EN", DeviceDecoders::decode_bool);
  this->add_variable(alarm_clock_active);

  auto *temp_in = new Variable<float>("TEMP_IN", DeviceDecoders::decode_temperature);
  this->add_variable(temp_in);

  auto *temp_out = new Variable<float>("TEMP_OUT", DeviceDecoders::decode_temperature);
  this->add_variable(temp_out);

  auto *battery_voltage = new Variable<float>("UBAT", DeviceDecoders::decode_voltage);
  this->add_variable(battery_voltage);

  auto *battery_voltage2 = new Variable<float>("UBATM", DeviceDecoders::decode_voltage);
  this->add_variable(battery_voltage2);

  auto *date = new Variable<time_t>("DATE", DeviceDecoders::decode_date);
  this->add_variable(date);

  auto *time = new Variable<time_t>("TIME", DeviceDecoders::decode_time);
  this->add_variable(time);

  auto *floor_heater =
      new Variable<bool>("FLOOR_HEATER_ON", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(floor_heater);

  auto *temp_in_offset = new Variable<int>("TEMP_IN_OFFSET", DeviceDecoders::decode_int);
  this->add_variable(temp_in_offset);

  auto *temp_out_offset = new Variable<int>("TEMP_OUT_OFFSET", DeviceDecoders::decode_int);
  this->add_variable(temp_out_offset);

  auto *software_version = new Variable<std::string>("SOFTWARE_VERSION", DeviceDecoders::decode_str);
  this->add_variable(software_version);

  auto *hs_key_state = new Variable<int>("HS_KEY_STATE", DeviceDecoders::decode_int);
  this->add_variable(hs_key_state);

  auto *th_error = new Variable<int>("TH_ERROR", DeviceDecoders::decode_int);
  this->add_variable(th_error);

  auto *trade_show = new Variable<int>("TRADE_SHOW", DeviceDecoders::decode_int);
  this->add_variable(trade_show);

  auto *therme_config = new Variable<int>("THERME_CONFIG", DeviceDecoders::decode_int);
  this->add_variable(therme_config);

  auto *floor_heater_config =
      new Variable<bool>("FLOOR_HEATER_CONFIG", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(floor_heater_config);

  auto *waste_water_heater_config = new Variable<int>("WASTE_WATER_HEATER_CONFIG", DeviceDecoders::decode_int);
  this->add_variable(waste_water_heater_config);

  auto *light_kueche = new Variable<bool>("LIGHT_KUECHE", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(light_kueche);

  auto *radio_config = new Variable<bool>("RADIO_CONFIG", DeviceDecoders::decode_bool);
  this->add_variable(radio_config);

  auto *light_aussen = new Variable<bool>("LIGHT_AUSSEN", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(light_aussen);
  auto *light_amb1 = new Variable<bool>("LIGHT_AMB1", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(light_amb1);
  auto *light_amb2 = new Variable<bool>("LIGHT_AMB2", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(light_amb2);
  auto *light_amb3 = new Variable<bool>("LIGHT_AMB3", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(light_amb3);
  auto *light_dusche = new Variable<bool>("LIGHT_DUSCHE", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(light_dusche);
  auto *fridge_on = new Variable<bool>("FRIDGE_ON_OFF", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(fridge_on);
  auto *therme_on = new Variable<bool>("THERME_ON", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(therme_on);
  auto *batt_voltage = new Variable<float>("IBS0_UBAT", DeviceDecoders::decode_number);
  this->add_variable(batt_voltage);
  auto *batt_current = new Variable<float>("IBS0_IBAT", DeviceDecoders::decode_number);
  this->add_variable(batt_current);
  auto *batt_soc = new Variable<float>("IBS0_SOC2", DeviceDecoders::decode_number);
  this->add_variable(batt_soc);
  auto *batt_remaining = new Variable<float>("IBS0_REMAINING_TIME", DeviceDecoders::decode_number);
  this->add_variable(batt_remaining);
  auto *batt_temp = new Variable<float>("IBS0_TEMPERATURE", DeviceDecoders::decode_number);
  this->add_variable(batt_temp);
  auto *water_level = new Variable<float>("WATER_LEVEL", DeviceDecoders::decode_number);
  this->add_variable(water_level);

  // ==========================================================================
  // KOMPLETNI KATALOG PROTOKOLU HobbyConnect (reference, decode-only).
  //   [OK] = odzkouseno na nasi Hobby 495 UL (ma HA entitu vyse)
  //   [?]  = klic mame/ctemе, ale chovani/ovladani NEODZKOUSENO
  //   [--] = jina vybava (Alde/Truma/Dometic/UltraHeat/SAT) - NEMAME (z dumpu/komunity)
  // Klice nize nemaji HA entitu, jen se dekoduji (raw) -> pripraveny k navazani.
  // ==========================================================================
  auto ref = [this](const char *name) {
    this->add_variable(new Variable<std::string>(name, DeviceDecoders::decode_str));
  };
  // --- System / identifikace [?] ---
  ref("PANEL_VERSION"); ref("GSM_SIGNAL"); ref("VEHICLE_TYPE");
  ref("LATITUDE"); ref("LONGITUDE"); ref("PI_PAIRING");
  // --- Baterie IBS0_ doplnkove [?] ---
  ref("IBS0_RECALIBRATED"); ref("IBS0_CAPACITY"); ref("IBS0_TYPE"); ref("IBS0_AVAILABLE");
  // --- Voda [?] (stupnice neoverena) ---
  ref("WATER_MEASUREMENT");
  // --- Svetla nezapojena v 495 UL / readback stmivacu [?] ---
  // (LIGHT_DUSCHE = koupelna -> ma vlastni switch entitu vyse, neni v ref katalogu)
  ref("LIGHT_WASCH"); ref("LIGHT_KUECHE2");
  ref("LIGHT_ZUSATZL"); ref("LIGHT_ZUSATZR");
  ref("LIGHT_DIM0"); ref("LIGHT_DIM1"); ref("LIGHT_DIM2"); ref("LIGHT_DIM3"); ref("LIGHT_DIM4");
  ref("LIGHT_BUSY");
  // --- Lednice doplnkove [?] (ovladani neovereno) ---
  ref("FRIDGE_AVAILABLE"); ref("FRIDGE_MODE"); ref("FRIDGE_SOURCE");
  ref("FRIDGE_TEMP"); ref("FRIDGE_TYPE"); ref("FRIDGE_ERROR");
  // --- Topeni Alde (HEATER_*) [--] NEMAME (manualni topeni) ---
  ref("HEATER_AVAILABLE"); ref("HEATER_ONOFF"); ref("HEATER_TEMP");
  ref("HEATER_WATER"); ref("HEATER_WATER_TEMP"); ref("HEATER_EL"); ref("HEATER_GAS");
  // --- UltraHeat [--] NEMAME ---
  ref("ULTRAHEAT_AVAILABLE"); ref("ULTRAHEAT_ONOFF"); ref("ULTRAHEAT_POWER"); ref("ULTRAHEAT_TEMP");
  // --- Klima Dometic FreshJet (AC_DOM_FJ_*) [--] NEMAME ---
  ref("AC_DOM_FJ_AVAILABLE"); ref("AC_DOM_FJ_ENABLE"); ref("AC_DOM_FJ_MODE");
  ref("AC_DOM_FJ_TARGETTEMP"); ref("AC_DOM_FJ_FAN_SPEED");
  // --- Klima Truma (AC_TRUMA_*) [--] NEMAME ---
  ref("AC_TRUMA_AVAILABLE"); ref("AC_TRUMA_TYPE"); ref("AC_TRUMA_ENABLE");
  ref("AC_TRUMA_TEMP"); ref("AC_TRUMA_MODE"); ref("AC_TRUMA_FAN_LEVEL");
  ref("AC_TRUMA_LIGHT_ON_OFF"); ref("AC_TRUMA_LIGHT_DIMMER"); ref("AC_TRUMA_MM");
  // --- Termostat topeni Truma/Alde (TH_*/TT_) [--] NEMAME (manualni topeni) ---
  ref("TH_AVAILABLE"); ref("TT_AVAILABLE"); ref("TH_TYPE");
  ref("TH_A_EN"); ref("TH_W_EN"); ref("TH_A_T"); ref("TH_W_T"); ref("TH_ES"); ref("TH_MM");
  // --- Satelit (SAT_*) [--] NEMAME ---
  ref("SAT_AVAILABLE"); ref("SAT_TYPE"); ref("SAT_STATUS"); ref("SAT_ADVANCED_STATUS");
  ref("SAT_COMMAND"); ref("SAT_LAT"); ref("SAT_LON"); ref("SAT_ORBITAL_POSTION");
}

void ControlUnitDeviceSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Fendt Control Unit");
  LOG_SWITCH(TAG, "  Main Switch", this->main_switch_switch_);
  LOG_SWITCH(TAG, "  All Lights Status", this->all_lights_switch_);
  LOG_SENSOR(TAG, "  Temp In", this->temp_in_sensor_);
  LOG_SENSOR(TAG, "  Temp Out", this->temp_out_sensor_);
  LOG_TEXT_SENSOR(TAG, "  Power Status", this->power_status_text_sensor_);
  LOG_TEXT_SENSOR(TAG, "  Software Version", this->software_version_text_sensor_);
  LOG_SWITCH(TAG, "  Floor Heater", this->floor_heater_switch_);
  LOG_SWITCH(TAG, "  Koupelna", this->dusche_switch_);
}

void ControlUnitDeviceSensor::on_data_decoded(IVariable *variable) {
  if (this->main_switch_switch_ && variable->get_name() == "HS_KEY_STATE") {
    auto *hs_key_state = static_cast<Variable<int> *>(variable);
    if (hs_key_state->is_active()) {
      if (this->main_switch_switch_)
        this->main_switch_switch_->publish_state(hs_key_state->get_value() > 0);
      if (this->all_lights_switch_)
        this->all_lights_switch_->publish_state(hs_key_state->get_value() == 2);
    }
  }
}

void ControlUnitDeviceSensor::on_state_change_command(const std::string &tag, const std::string &command) {
  std::string cmd = command;
  if (tag == "MAIN_SWITCH") {
    auto *hs_key_long = GET_VARIABLE(bool, "HS_KEY_LONG");
    auto *hs_key_state = GET_VARIABLE(int, "HS_KEY_STATE");
    bool current_state = hs_key_state->get_value() > 0;

    ESP_LOGV(TAG, "Main switch state changed. cs: %s", ONOFF(current_state));
    if (!(hs_key_long && hs_key_state))
      return;
    if (current_state) {
      hs_key_long->set_value(true);
      cmd = hs_key_long->get_command();
    } else {
      auto *hs_key = GET_VARIABLE(bool, "HS_KEY");
      hs_key->set_value(true);
      cmd = hs_key->get_command();
    }
  } else if (tag == "ALL_LIGHTS_SWITCH") {
    auto *hs_key = GET_VARIABLE(bool, "HS_KEY");
    auto *hs_key_state = GET_VARIABLE(int, "HS_KEY_STATE");
    bool current_state = hs_key_state->get_value() == 2;
    ESP_LOGV(TAG, "Light switch state changed. cs: %s", ONOFF(current_state));
    if (hs_key && hs_key_state) {
      cmd = hs_key->get_command();
    }
  }
  if (!cmd.empty()) {
    ESP_LOGV(TAG, "Switch state changed command:%s", cmd.c_str());
    this->command_callback_.call(cmd);
  }
}
}  // namespace esphome::fendt_caravan
#endif
