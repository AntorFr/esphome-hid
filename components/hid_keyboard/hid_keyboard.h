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
  void type(const std::string &text);

  bool is_initialized() const { return this->initialized_; }

 protected:
  bool initialized_{false};
  void char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier);
  uint8_t key_name_to_keycode(const std::string &key);
  void send_report(uint8_t modifier, uint8_t keycode);
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
  void play(Ts... x) override { this->parent_->type(this->text_.value(x...)); }
};

}  // namespace hid_keyboard
}  // namespace esphome

#endif  // USE_ESP32
