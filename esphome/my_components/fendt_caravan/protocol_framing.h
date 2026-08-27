#pragma once
// BLE framing of the HobbyConnect/Fendt protocol, kept free of ESP-IDF types on purpose:
// this is the only part of the component that can be exercised on a host compiler
// (see tests/host/ in the project repository).
//
// Outgoing: the panel accepts at most 17 bytes per write; longer commands are split into
// chunks, every chunk but the last one terminated with '@'.
// Incoming: notifications arrive with the same chunking, so a message is complete only
// once a frame without a trailing '@' has been received.
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace esphome::fendt_caravan {

static const uint8_t MAX_COMMAND_CHUNK = 17;

inline std::vector<std::string> split_command_chunks(const std::string &cmd) {
  std::vector<std::string> chunks;
  for (size_t start = 0; start < cmd.size(); start += MAX_COMMAND_CHUNK) {
    std::string chunk = cmd.substr(start, MAX_COMMAND_CHUNK);
    if (start + MAX_COMMAND_CHUNK < cmd.size())
      chunk += '@';
    chunks.push_back(chunk);
  }
  return chunks;
}

// Feeds one notification frame into the reassembly buffer. Returns the complete message
// once the final frame of a message has arrived, std::nullopt while more frames are pending.
inline std::optional<std::string> feed_notification_chunk(std::string &pending, const uint8_t *data, uint16_t len) {
  // no fixed size buffer here: a frame is only bounded by the negotiated MTU, and the
  // panel decides how long it is
  std::string frame(reinterpret_cast<const char *>(data), len);
  bool more_to_come = !frame.empty() && frame.back() == '@';
  if (more_to_come)
    frame.pop_back();

  pending += frame;
  if (more_to_come)
    return std::nullopt;

  std::string message;
  message.swap(pending);
  return message;
}

}  // namespace esphome::fendt_caravan
