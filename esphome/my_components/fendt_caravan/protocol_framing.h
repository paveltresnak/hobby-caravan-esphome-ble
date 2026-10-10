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
#include <deque>
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

// A write the BLE stack refuses is tried this many times before the command is dropped.
static const uint8_t MAX_WRITE_ATTEMPTS = 3;

enum class WriteOutcome { SENT, RETRY, GAVE_UP };

// One BLE session with the panel: the outgoing command queue and the reassembly of incoming
// notifications. All of it belongs to the connection - stop() on disconnect forgets it, so a
// fragment cut off by the drop cannot block the next session and a command given while the
// caravan was out of reach does not fire after the reconnect.
class PanelLink {
 public:
  // After service discovery: a fresh session that opens with the handshake.
  void start(const std::string &bt_id) {
    this->stop();
    this->started_ = true;
    this->queue("net-BT_ID-" + bt_id);
    this->queue("net-BT_VARS");
  }

  void stop() {
    this->started_ = false;
    this->receiving_ = false;
    this->attempts_ = 0;
    this->pending_.clear();
    this->commands_.clear();
  }

  // Returns false when there is no session: the command is dropped, not kept for later.
  bool queue(const std::string &cmd) {
    if (!this->started_)
      return false;
    auto chunks = split_command_chunks(cmd);
    if (!chunks.empty())
      this->commands_.emplace_back(chunks.begin(), chunks.end());
    return true;
  }

  // The chunk to write now, or nullptr when there is nothing to send or the panel is in the
  // middle of a fragmented message.
  const std::string *next() const {
    if (!this->started_ || this->receiving_ || this->commands_.empty())
      return nullptr;
    return &this->commands_.front().front();
  }

  // Result of writing the chunk returned by next(). A command that keeps being refused is
  // dropped whole: the rest of a split command on its own would be garbage to the panel.
  WriteOutcome on_write(bool accepted) {
    if (this->commands_.empty())
      return WriteOutcome::SENT;
    auto &chunks = this->commands_.front();
    if (accepted) {
      this->attempts_ = 0;
      chunks.pop_front();
      if (chunks.empty())
        this->commands_.pop_front();
      return WriteOutcome::SENT;
    }
    if (++this->attempts_ < MAX_WRITE_ATTEMPTS)
      return WriteOutcome::RETRY;
    this->attempts_ = 0;
    this->commands_.pop_front();
    return WriteOutcome::GAVE_UP;
  }

  // Returns the complete message once its last frame has arrived.
  std::optional<std::string> feed(const uint8_t *data, uint16_t len) {
    auto message = feed_notification_chunk(this->pending_, data, len);
    this->receiving_ = !message.has_value();
    return message;
  }

 private:
  bool started_{false};
  bool receiving_{false};
  uint8_t attempts_{0};
  std::string pending_;
  std::deque<std::deque<std::string>> commands_;
};

// Home Assistant refuses a state longer than 255 characters.
static const size_t MAX_TRAFFIC_LINE = 255;

// Lines of the traffic text sensor (`type: traffic`): every command sent to the panel and
// every message it sends back, one state per line. The history of that entity in Home
// Assistant then reads like the debug log, so somebody with only a phone at hand can test
// a command and show what the panel answered.
//
// The running number keeps two identical lines apart - Home Assistant records a new state
// only when the text changes, so a second "LIGHT_DECKE:1" would otherwise not show up.
class TrafficLog {
 public:
  // Messages of this key are left out (TEMP_IN/TEMP_OUT arrive several times a second).
  void ignore_key(const std::string &key) { this->ignored_.push_back(key); }

  // The line for a message from the panel, std::nullopt for a message of an ignored key.
  std::optional<std::string> received(const std::string &message) {
    std::string key = message.substr(0, message.find(':'));
    for (const auto &ignored : this->ignored_) {
      if (key == ignored)
        return std::nullopt;
    }
    return this->line_("\u2190 ", message);
  }

  std::string sent(const std::string &command) { return this->line_("\u2192 ", command); }

  // A command given while there was no connection to the panel (it is dropped, not queued).
  std::string dropped(const std::string &command) { return this->line_("\u2717 ", command + " (not connected)"); }

 private:
  std::string line_(const char *direction, const std::string &text) {
    this->counter_ = this->counter_ % 999 + 1;
    std::string line = "#" + std::to_string(this->counter_) + " " + direction + text;
    if (line.size() > MAX_TRAFFIC_LINE) {
      line.resize(MAX_TRAFFIC_LINE);
      // do not leave half of a multi-byte UTF-8 character at the end
      while (!line.empty() && (static_cast<uint8_t>(line.back()) & 0xC0) == 0x80)
        line.pop_back();
      if (!line.empty() && static_cast<uint8_t>(line.back()) >= 0xC0)
        line.pop_back();
    }
    return line;
  }

  uint16_t counter_{0};
  std::vector<std::string> ignored_;
};

}  // namespace esphome::fendt_caravan
