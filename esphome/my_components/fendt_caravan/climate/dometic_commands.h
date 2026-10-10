#pragma once
// BLE commands for the Dometic FreshJet A/C (AC_DOM_FJ_*), kept free of ESPHome types so the
// rules can be exercised by the host tests (tests/host/ in the project repository).
//
// Formats verified on a Hobby 540 KMFE (SW V011900) on 2026-10-10, the panel answering each
// one within a second with the key it changed:
//   cmd-tgl:AC_DOM_FJ_ENABLE              -> AC_DOM_FJ_ENABLE:On / Off (+ AC_EN:1 / 0)
//   cmd-set:AC_DOM_FJ_TARGETTEMP=21       -> AC_DOM_FJ_TARGETTEMP:21
//   net-AC_DOM_FJ_FAN_SPEED-Slow          -> AC_DOM_FJ_FAN_SPEED:Slow
//   net-AC_DOM_FJ_MODE-Cool               -> AC_DOM_FJ_MODE:Cool
// Fan speed and mode take the same words the panel reports (Auto/Slow/Med/Fast,
// Auto/Cool/Heat/Fan); only Slow and Cool have been sent so far.
#include <optional>
#include <string>
#include <vector>

namespace esphome::fendt_caravan {

// What the panel last reported (empty / nullopt = not reported yet).
struct DometicState {
  std::optional<bool> on;
  std::string mode;
  std::string fan_speed;
  std::optional<int> target_temp;
};

// What Home Assistant asked for; only the fields set in the request are filled in.
struct DometicRequest {
  std::optional<bool> on;
  std::optional<std::string> mode;
  std::optional<std::string> fan_speed;
  std::optional<int> target_temp;
};

struct DometicCommands {
  std::vector<std::string> commands;
  // The A/C was asked to switch on or off, but the panel has not said yet whether it is on.
  // ENABLE is a toggle, so nothing is sent for it: a blind toggle would just as likely turn
  // the A/C off as on.
  bool enable_unknown{false};
  // The ENABLE toggle is among the commands.
  bool toggles_enable{false};
};

// Only what differs from the panel is sent - for ENABLE that is what makes a toggle safe,
// for the rest it saves BLE writes. Switching on goes first, so mode, fan and temperature
// reach an A/C that is running.
inline DometicCommands dometic_commands(const DometicState &panel, const DometicRequest &want) {
  DometicCommands out;
  if (want.on.has_value()) {
    if (!panel.on.has_value()) {
      out.enable_unknown = true;
    } else if (*panel.on != *want.on) {
      out.commands.push_back("cmd-tgl:AC_DOM_FJ_ENABLE");
      out.toggles_enable = true;
    }
  }
  if (want.mode.has_value() && *want.mode != panel.mode)
    out.commands.push_back("net-AC_DOM_FJ_MODE-" + *want.mode);
  if (want.fan_speed.has_value() && *want.fan_speed != panel.fan_speed)
    out.commands.push_back("net-AC_DOM_FJ_FAN_SPEED-" + *want.fan_speed);
  if (want.target_temp.has_value() && want.target_temp != panel.target_temp)
    out.commands.push_back("cmd-set:AC_DOM_FJ_TARGETTEMP=" + std::to_string(*want.target_temp));
  return out;
}

}  // namespace esphome::fendt_caravan
