#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

#include <string>

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2)
#define HID_KEYBOARD_SUPPORTED
#endif

namespace esphome {
namespace hid_keyboard {

// Modifier keys
enum Modifier : uint8_t {
  MOD_NONE = 0x00,
  MOD_LEFT_CTRL = 0x01,
  MOD_LEFT_SHIFT = 0x02,
  MOD_LEFT_ALT = 0x04,
  MOD_LEFT_GUI = 0x08,
  MOD_RIGHT_CTRL = 0x10,
  MOD_RIGHT_SHIFT = 0x20,
  MOD_RIGHT_ALT = 0x40,
  MOD_RIGHT_GUI = 0x80,
};

class HIDKeyboard : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  void press(const std::string &key, uint8_t modifier = MOD_NONE);
  void release();
  void release_all();
  void tap(const std::string &key, uint8_t modifier = MOD_NONE);
  void type(const std::string &text, uint32_t speed_ms = 50, uint32_t jitter_ms = 0);
  
  // Keep awake
  void start_keep_awake(const std::string &key, uint32_t interval_ms, uint32_t jitter_ms = 0);
  void stop_keep_awake();
  
  // Connection status
  bool is_connected();
  bool is_ready();

  bool is_initialized() const { return this->initialized_; }

 protected:
  bool initialized_{false};
  void char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier);
  uint8_t key_name_to_keycode(const std::string &key);
  void send_report(uint8_t modifier, uint8_t keycode);
  
  // Keep awake state
  bool keep_awake_enabled_{false};
  std::string keep_awake_key_;
  uint32_t keep_awake_interval_{60000};
  uint32_t keep_awake_jitter_{0};
  uint32_t keep_awake_last_time_{0};
  uint32_t keep_awake_next_interval_{0};
};

template<typename... Ts>
class PressAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  TEMPLATABLE_VALUE(std::string, key)
  void set_modifier(uint8_t modifier) { this->modifier_ = modifier; }
  void play(Ts... x) override { this->parent_->press(this->key_.value(x...), this->modifier_); }
 protected:
  uint8_t modifier_{MOD_NONE};
};

template<typename... Ts>
class ReleaseAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  void play(Ts... x) override { this->parent_->release(); }
};

template<typename... Ts>
class TapAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  TEMPLATABLE_VALUE(std::string, key)
  void set_modifier(uint8_t modifier) { this->modifier_ = modifier; }
  void play(Ts... x) override { this->parent_->tap(this->key_.value(x...), this->modifier_); }
 protected:
  uint8_t modifier_{MOD_NONE};
};

template<typename... Ts>
class ReleaseAllAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  void play(Ts... x) override { this->parent_->release_all(); }
};

template<typename... Ts>
class TypeAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  TEMPLATABLE_VALUE(std::string, text)
  TEMPLATABLE_VALUE(uint32_t, speed)
  TEMPLATABLE_VALUE(uint32_t, jitter)
  void play(Ts... x) override {
    this->parent_->type(this->text_.value(x...), this->speed_.value(x...), this->jitter_.value(x...));
  }
};

template<typename... Ts>
class StartKeepAwakeAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  TEMPLATABLE_VALUE(std::string, key)
  TEMPLATABLE_VALUE(uint32_t, interval)
  TEMPLATABLE_VALUE(uint32_t, jitter)
  void play(Ts... x) override {
    this->parent_->start_keep_awake(this->key_.value(x...), this->interval_.value(x...), this->jitter_.value(x...));
  }
};

template<typename... Ts>
class StopKeepAwakeAction : public Action<Ts...>, public Parented<HIDKeyboard> {
 public:
  void play(Ts... x) override {
    this->parent_->stop_keep_awake();
  }
};

}  // namespace hid_keyboard
}  // namespace esphome

#endif  // USE_ESP32
