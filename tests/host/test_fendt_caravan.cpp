// Host tests for the parts of the fendt_caravan fork that are pure logic:
// BLE framing (command chunking, notification reassembly) and the value decoders.
//
// They exist because these are the paths that the esphbot review of ESPHome PR #13327
// flagged as remotely triggerable crashes: the panel is a third party device, so every
// decoder has to survive whatever it sends. ESP-IDF builds ESPHome with exceptions
// disabled, hence -fno-exceptions here: a throwing decoder is a reboot on the real device.
//
// Run: ./run_tests.sh   (each test runs as its own process, so a crash is reported
// as a crash instead of taking the whole suite down)

#include "protocol_framing.h"
#include "sensor/device_decoders.h"
#include "variable.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using esphome::fendt_caravan::DeviceDecoders;
using esphome::fendt_caravan::feed_notification_chunk;
using esphome::fendt_caravan::split_command_chunks;
using esphome::fendt_caravan::Variable;

static int g_failures = 0;

static void fail(int line, const std::string &what) {
  fprintf(stderr, "  FAIL (line %d): %s\n", line, what.c_str());
  g_failures++;
}

#define CHECK(cond) \
  do { \
    if (!(cond)) \
      fail(__LINE__, #cond); \
  } while (0)

#define CHECK_STR(actual, expected) \
  do { \
    std::string a_ = (actual), e_ = (expected); \
    if (a_ != e_) \
      fail(__LINE__, "expected \"" + e_ + "\", got \"" + a_ + "\""); \
  } while (0)

#define CHECK_NEAR(actual, expected) \
  do { \
    float a_ = (actual), e_ = (expected); \
    if (!(std::fabs(a_ - e_) < 0.001f)) \
      fail(__LINE__, "expected " + std::to_string(e_) + ", got " + std::to_string(a_)); \
  } while (0)

static std::optional<std::string> feed(std::string &pending, const std::string &frame) {
  return feed_notification_chunk(pending, reinterpret_cast<const uint8_t *>(frame.data()), frame.size());
}

// ---------------------------------------------------------------- command chunking

// "net-BT_VARS" and friends fit into a single write and must go out untouched.
static void command_short_is_not_chunked() {
  auto chunks = split_command_chunks("net-BT_VARS");
  CHECK(chunks.size() == 1);
  CHECK_STR(chunks[0], "net-BT_VARS");
}

static void command_of_exactly_17_bytes_is_not_chunked() {
  auto chunks = split_command_chunks("cmd-tgl:LIGHT_AMB");  // 17 bytes
  CHECK(chunks.size() == 1);
  CHECK_STR(chunks[0], "cmd-tgl:LIGHT_AMB");
}

static void command_of_two_chunks_is_split() {
  auto chunks = split_command_chunks("net-BT_ID-0123456789ABCDEF");  // 26 bytes
  CHECK(chunks.size() == 2);
  CHECK_STR(chunks[0], "net-BT_ID-0123456@");
  CHECK_STR(chunks[1], "789ABCDEF");
}

// Every chunk after the first has to continue where the previous one stopped.
static void command_of_three_chunks_advances_offset() {
  std::string cmd = "net-VERY_LONG_KEY_NAME_FOR_TEST-123456789";  // 41 bytes
  auto chunks = split_command_chunks(cmd);
  CHECK(chunks.size() == 3);
  CHECK_STR(chunks[0], "net-VERY_LONG_KEY@");
  CHECK_STR(chunks[1], "_NAME_FOR_TEST-12@");
  CHECK_STR(chunks[2], "3456789");
}

// The chunk indices used to be int8_t, so anything past 127 bytes wrapped around.
static void command_longer_than_127_bytes_is_still_chunked() {
  std::string cmd(140, 'x');
  auto chunks = split_command_chunks(cmd);
  CHECK(chunks.size() == 9);  // 8 full chunks of 17 + 4 byte remainder
  std::string reassembled;
  for (size_t i = 0; i < chunks.size(); i++) {
    std::string chunk = chunks[i];
    if (i + 1 < chunks.size()) {
      CHECK(chunk.size() == 18);
      CHECK(chunk.back() == '@');
      chunk.pop_back();
    }
    reassembled += chunk;
  }
  CHECK_STR(reassembled, cmd);
}

static void empty_command_produces_no_chunk() {
  auto chunks = split_command_chunks("");
  CHECK(chunks.empty());
}

// ------------------------------------------------------ notification reassembly

static void notification_without_at_is_complete() {
  std::string pending;
  auto message = feed(pending, "TEMP_IN: 25,5^C");
  CHECK(message.has_value());
  CHECK_STR(message.value_or(""), "TEMP_IN: 25,5^C");
  CHECK(pending.empty());
}

static void notification_of_two_chunks_is_reassembled() {
  std::string pending;
  CHECK(!feed(pending, "IBS0_UBAT: 13,9@").has_value());
  auto message = feed(pending, " V");
  CHECK(message.has_value());
  CHECK_STR(message.value_or(""), "IBS0_UBAT: 13,9 V");
}

static void notification_of_three_chunks_is_reassembled() {
  std::string pending;
  CHECK(!feed(pending, "IBS0_REMAINING_T@").has_value());
  CHECK(!feed(pending, "IME: 77,5 h (ext@").has_value());
  auto message = feed(pending, "ra)");
  CHECK(message.has_value());
  CHECK_STR(message.value_or(""), "IBS0_REMAINING_TIME: 77,5 h (extra)");
}

// A frame may be as long as the negotiated MTU allows, not just 24 bytes.
static void notification_longer_than_the_old_buffer_is_kept_whole() {
  std::string pending;
  std::string frame(200, 'A');
  auto message = feed(pending, frame);
  CHECK(message.has_value());
  CHECK(message.value_or("").size() == 200);
}

static void empty_notification_yields_empty_message() {
  std::string pending;
  auto message = feed(pending, "");
  CHECK(message.has_value());
  CHECK_STR(message.value_or("x"), "");
}

// -------------------------------------------------------------------- decoders

static void temperature_parses_comma_decimal_and_unit() {
  CHECK_NEAR(DeviceDecoders::decode_temperature("25,5^C"), 25.5f);
}

static void temperature_of_empty_value_does_not_crash() {
  CHECK(std::isnan(DeviceDecoders::decode_temperature("")));
}

static void temperature_of_garbage_does_not_crash() {
  CHECK(std::isnan(DeviceDecoders::decode_temperature("n/a")));
}

// The panel pads values and attaches units ("IBS0_UBAT: 13,9 V"), and the battery
// sensors depend on that being parsed as a number, not rejected.
static void number_tolerates_padding_and_unit_suffix() {
  CHECK_NEAR(DeviceDecoders::decode_number(" 13,9 V"), 13.9f);
}

// Same leniency for the upstream voltage decoder: "13,9 V" leaves a trailing space
// once the unit is stripped, which a strict full-string parse rejects (reads as 0 V).
static void voltage_tolerates_unit_suffix() {
  CHECK_NEAR(DeviceDecoders::decode_voltage("13,9 V"), 13.9f);
}

static void number_parses_plain_integer() {
  CHECK_NEAR(DeviceDecoders::decode_number("4"), 4.0f);  // WATER_LEVEL = full tank
}

static void number_of_empty_value_does_not_crash() {
  CHECK(std::isnan(DeviceDecoders::decode_number("")));
}

// The panel pads integers too ("TEMP_IN_OFFSET:  0"), which a strict full-string parse
// rejects - and a rejected value silently reads as 0.
static void int_tolerates_padding() {
  CHECK(DeviceDecoders::decode_int("  4") == 4);
}

static void int_of_garbage_returns_zero() {
  CHECK(DeviceDecoders::decode_int("n/a") == 0);
}

static void int_str_maps_index_to_text() {
  CHECK_STR(DeviceDecoders::decode_int_str("1", {"Auto", "Gas", "12V", "230V"}), "Gas");
}

static void int_str_tolerates_padding() {
  CHECK_STR(DeviceDecoders::decode_int_str(" 2", {"Auto", "Gas", "12V", "230V"}), "12V");
}

static void int_str_out_of_range_returns_empty() {
  CHECK_STR(DeviceDecoders::decode_int_str("7", {"Auto", "Gas", "12V", "230V"}), "");
}

// ------------------------------------------------------------------- variables

// HS_KEY and HS_KEY_LONG are registered without a decoder, but the panel does send them.
static void variable_without_decoder_does_not_crash() {
  Variable<bool> variable("HS_KEY", nullptr);
  variable.decode("1");
  CHECK(!variable.is_active());
}

static void variable_with_decoder_stores_value() {
  Variable<bool> variable("HS_EN", DeviceDecoders::decode_bool);
  variable.decode("1");
  CHECK(variable.is_active());
  CHECK(variable.get_value());
}

// ---------------------------------------------------------------- link session
//
// Everything that has to be forgotten when the BLE link drops. The esphbot review of
// esphome#19091 found that a drop in the middle of a fragmented notification leaves the
// "rest of the message is coming" state set, so after the reconnect nothing is sent ever
// again - not even the handshake - until the ESP reboots.

using esphome::fendt_caravan::PanelLink;
using esphome::fendt_caravan::WriteOutcome;

static const char *const TEST_BT_ID = "01:02:03:04:05:06";

static std::optional<std::string> feed(PanelLink &link, const std::string &frame) {
  return link.feed(reinterpret_cast<const uint8_t *>(frame.data()), frame.size());
}

// What the link wants written right now, reported back as accepted by the BLE stack.
static std::string send_next(PanelLink &link) {
  const std::string *chunk = link.next();
  if (chunk == nullptr)
    return "<nothing>";
  std::string sent = *chunk;
  link.on_write(true);
  return sent;
}

// Same, but the BLE stack refuses the write.
static WriteOutcome fail_next(PanelLink &link) {
  link.next();
  return link.on_write(false);
}

// "net-BT_ID-01:02:03:04:05:06" is 27 bytes, so it goes out in two writes.
static void handshake_announces_the_given_bt_id() {
  PanelLink link;
  link.start(TEST_BT_ID);
  CHECK_STR(send_next(link), "net-BT_ID-01:02:0@");
  CHECK_STR(send_next(link), "3:04:05:06");
  CHECK_STR(send_next(link), "net-BT_VARS");
  CHECK_STR(send_next(link), "<nothing>");
}

static void nothing_is_sent_before_the_link_is_started() {
  PanelLink link;
  CHECK(!link.queue("cmd-tgl:LIGHT_KUECHE"));
  CHECK_STR(send_next(link), "<nothing>");
}

// A light switched in HA while the caravan was out of reach must not toggle an hour later.
static void command_given_while_disconnected_is_not_replayed() {
  PanelLink link;
  link.queue("cmd-tgl:LIGHT_KUECHE");
  link.start(TEST_BT_ID);
  send_next(link);
  send_next(link);
  CHECK_STR(send_next(link), "net-BT_VARS");
  CHECK_STR(send_next(link), "<nothing>");
}

static void queued_commands_are_dropped_on_disconnect() {
  PanelLink link;
  link.start(TEST_BT_ID);
  CHECK(link.queue("cmd-tgl:LIGHT_KUECHE"));
  link.stop();
  link.start(TEST_BT_ID);
  send_next(link);
  send_next(link);
  CHECK_STR(send_next(link), "net-BT_VARS");
  CHECK_STR(send_next(link), "<nothing>");
}

static void nothing_is_sent_while_a_message_is_still_arriving() {
  PanelLink link;
  link.start(TEST_BT_ID);
  feed(link, "TEMP_IN: 21,@");
  CHECK(link.next() == nullptr);
  CHECK_STR(feed(link, "5^C").value_or("<incomplete>"), "TEMP_IN: 21,5^C");
  CHECK(link.next() != nullptr);
}

static void fragment_cut_by_disconnect_does_not_block_the_next_session() {
  PanelLink link;
  link.start(TEST_BT_ID);
  feed(link, "TEMP_IN: 21,@");
  link.stop();
  link.start(TEST_BT_ID);
  CHECK_STR(send_next(link), "net-BT_ID-01:02:0@");
}

static void fragment_cut_by_disconnect_is_not_glued_to_the_next_message() {
  PanelLink link;
  link.start(TEST_BT_ID);
  feed(link, "TEMP_IN: 21,@");
  link.stop();
  link.start(TEST_BT_ID);
  CHECK_STR(feed(link, "LINE_EN:1").value_or("<incomplete>"), "LINE_EN:1");
}

// Two toggles of the same light in a row are two commands; both have to go out, whole.
static void identical_commands_are_both_sent() {
  PanelLink link;
  link.start(TEST_BT_ID);
  send_next(link);
  send_next(link);
  send_next(link);
  link.queue("cmd-tgl:LIGHT_KUECHE");  // 20 bytes, two writes
  link.queue("cmd-tgl:LIGHT_KUECHE");
  CHECK_STR(send_next(link), "cmd-tgl:LIGHT_KUE@");
  CHECK_STR(send_next(link), "CHE");
  CHECK_STR(send_next(link), "cmd-tgl:LIGHT_KUE@");
  CHECK_STR(send_next(link), "CHE");
  CHECK_STR(send_next(link), "<nothing>");
}

static void refused_write_is_retried() {
  PanelLink link;
  link.start(TEST_BT_ID);
  CHECK(fail_next(link) == WriteOutcome::RETRY);
  CHECK_STR(send_next(link), "net-BT_ID-01:02:0@");
}

// A write that keeps failing must not jam the queue, and the second half of BT_ID sent on
// its own would be garbage to the panel - so the whole command goes.
static void command_refused_three_times_is_dropped_whole() {
  PanelLink link;
  link.start(TEST_BT_ID);
  CHECK(fail_next(link) == WriteOutcome::RETRY);
  CHECK(fail_next(link) == WriteOutcome::RETRY);
  CHECK(fail_next(link) == WriteOutcome::GAVE_UP);
  CHECK_STR(send_next(link), "net-BT_VARS");
}

static void attempts_are_counted_per_chunk() {
  PanelLink link;
  link.start(TEST_BT_ID);
  fail_next(link);
  fail_next(link);
  CHECK_STR(send_next(link), "net-BT_ID-01:02:0@");
  CHECK(fail_next(link) == WriteOutcome::RETRY);
  CHECK(fail_next(link) == WriteOutcome::RETRY);
  CHECK_STR(send_next(link), "3:04:05:06");
}

// ------------------------------------------------------------------------ main

struct TestCase {
  const char *name;
  void (*fn)();
};

static const TestCase TESTS[] = {
    {"command_short_is_not_chunked", command_short_is_not_chunked},
    {"command_of_exactly_17_bytes_is_not_chunked", command_of_exactly_17_bytes_is_not_chunked},
    {"command_of_two_chunks_is_split", command_of_two_chunks_is_split},
    {"command_of_three_chunks_advances_offset", command_of_three_chunks_advances_offset},
    {"command_longer_than_127_bytes_is_still_chunked", command_longer_than_127_bytes_is_still_chunked},
    {"empty_command_produces_no_chunk", empty_command_produces_no_chunk},
    {"notification_without_at_is_complete", notification_without_at_is_complete},
    {"notification_of_two_chunks_is_reassembled", notification_of_two_chunks_is_reassembled},
    {"notification_of_three_chunks_is_reassembled", notification_of_three_chunks_is_reassembled},
    {"notification_longer_than_the_old_buffer_is_kept_whole", notification_longer_than_the_old_buffer_is_kept_whole},
    {"empty_notification_yields_empty_message", empty_notification_yields_empty_message},
    {"temperature_parses_comma_decimal_and_unit", temperature_parses_comma_decimal_and_unit},
    {"temperature_of_empty_value_does_not_crash", temperature_of_empty_value_does_not_crash},
    {"temperature_of_garbage_does_not_crash", temperature_of_garbage_does_not_crash},
    {"number_tolerates_padding_and_unit_suffix", number_tolerates_padding_and_unit_suffix},
    {"voltage_tolerates_unit_suffix", voltage_tolerates_unit_suffix},
    {"number_parses_plain_integer", number_parses_plain_integer},
    {"number_of_empty_value_does_not_crash", number_of_empty_value_does_not_crash},
    {"int_tolerates_padding", int_tolerates_padding},
    {"int_of_garbage_returns_zero", int_of_garbage_returns_zero},
    {"int_str_maps_index_to_text", int_str_maps_index_to_text},
    {"int_str_tolerates_padding", int_str_tolerates_padding},
    {"int_str_out_of_range_returns_empty", int_str_out_of_range_returns_empty},
    {"variable_without_decoder_does_not_crash", variable_without_decoder_does_not_crash},
    {"variable_with_decoder_stores_value", variable_with_decoder_stores_value},
    {"handshake_announces_the_given_bt_id", handshake_announces_the_given_bt_id},
    {"nothing_is_sent_before_the_link_is_started", nothing_is_sent_before_the_link_is_started},
    {"command_given_while_disconnected_is_not_replayed", command_given_while_disconnected_is_not_replayed},
    {"queued_commands_are_dropped_on_disconnect", queued_commands_are_dropped_on_disconnect},
    {"nothing_is_sent_while_a_message_is_still_arriving", nothing_is_sent_while_a_message_is_still_arriving},
    {"fragment_cut_by_disconnect_does_not_block_the_next_session",
     fragment_cut_by_disconnect_does_not_block_the_next_session},
    {"fragment_cut_by_disconnect_is_not_glued_to_the_next_message",
     fragment_cut_by_disconnect_is_not_glued_to_the_next_message},
    {"identical_commands_are_both_sent", identical_commands_are_both_sent},
    {"refused_write_is_retried", refused_write_is_retried},
    {"command_refused_three_times_is_dropped_whole", command_refused_three_times_is_dropped_whole},
    {"attempts_are_counted_per_chunk", attempts_are_counted_per_chunk},
};

int main(int argc, char **argv) {
  std::string arg = argc > 1 ? argv[1] : "";
  // the shell wrapper reads the test names from our own stdout, which is CRLF on Windows
  while (!arg.empty() && (arg.back() == '\r' || arg.back() == '\n'))
    arg.pop_back();

  if (arg == "--list") {
    for (const auto &test : TESTS)
      printf("%s\n", test.name);
    return 0;
  }

  int selected = 0;
  for (const auto &test : TESTS) {
    if (!arg.empty() && arg != test.name)
      continue;
    selected++;
    if (arg.empty())
      printf("- %s\n", test.name);
    test.fn();
  }
  if (selected == 0) {
    fprintf(stderr, "unknown test: %s\n", arg.c_str());
    return 2;
  }
  return g_failures == 0 ? 0 : 1;
}
