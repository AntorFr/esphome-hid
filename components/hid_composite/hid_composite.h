#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/automation.h"

#ifdef USE_ESP32
#include <soc/soc_caps.h>
#if SOC_USB_OTG_SUPPORTED
#define HID_COMPOSITE_SUPPORTED
#endif
#endif

namespace esphome {
namespace hid_composite {

// Keyboard layouts
enum KeyboardLayout : uint8_t {
  LAYOUT_QWERTY_US = 0,
  LAYOUT_AZERTY_FR = 1,
  LAYOUT_QWERTZ_DE = 2,
};

enum MouseButton : uint8_t {
  BUTTON_LEFT = 0,
  BUTTON_RIGHT = 1,
  BUTTON_MIDDLE = 2,
};

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

class HIDComposite : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Mouse functions
  void move(int8_t x, int8_t y);
  void scroll(int8_t vertical, int8_t horizontal);
  void click(MouseButton button);
  void mouse_press(MouseButton button);
  void mouse_release(MouseButton button);
  void mouse_release_all();

  // Keyboard functions
  void key_press(const std::string &key, uint8_t modifier = 0);
  void key_release();
  void key_release_all();
  void key_tap(const std::string &key, uint8_t modifier = 0);
  void type(const std::string &text, uint32_t speed_ms = 50, uint32_t jitter_ms = 0);
  
  // Layout
  void set_layout(KeyboardLayout layout) { this->layout_ = layout; }
  KeyboardLayout get_layout() const { return this->layout_; }
  
  // Keep awake (mouse)
  void start_mouse_keep_awake(uint32_t interval_ms, uint32_t jitter_ms = 0);
  void stop_mouse_keep_awake();
  
  // Keep awake (keyboard)
  void start_keyboard_keep_awake(const std::string &key, uint32_t interval_ms, uint32_t jitter_ms = 0);
  void stop_keyboard_keep_awake();
  
  // Connection status
  bool is_connected();
  bool is_ready();
  
  // Telephony functions
  void mute();
  void unmute();
  void toggle_mute();
  void hook_switch(bool state);
  void answer_call();
  void hang_up();
  
  // Telephony state getters
  bool is_muted() { return this->muted_; }
  bool is_off_hook() { return this->off_hook_; }
  bool is_ringing() { return this->ringing_; }
  bool is_hold() { return this->hold_; }
  
  // Telephony callbacks
  void add_on_mute_callback(std::function<void(bool)> &&callback) { this->mute_callbacks_.add(std::move(callback)); }
  void add_on_off_hook_callback(std::function<void(bool)> &&callback) { this->off_hook_callbacks_.add(std::move(callback)); }
  void add_on_ring_callback(std::function<void(bool)> &&callback) { this->ring_callbacks_.add(std::move(callback)); }
  void add_on_hold_callback(std::function<void(bool)> &&callback) { this->hold_callbacks_.add(std::move(callback)); }
  
  // Process host report (for telephony LED states)
  void process_host_report(uint8_t const *buffer, uint16_t bufsize);

 protected:
  bool initialized_{false};
  KeyboardLayout layout_{LAYOUT_QWERTY_US};
  uint8_t mouse_buttons_{0};

  void send_mouse_report();
  void send_keyboard_report(uint8_t modifier, uint8_t keycode);
  void char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier);
  void char_to_keycode_qwerty(char c, uint8_t &keycode, uint8_t &modifier);
  void char_to_keycode_azerty(char c, uint8_t &keycode, uint8_t &modifier);
  void char_to_keycode_qwertz(char c, uint8_t &keycode, uint8_t &modifier);
  uint8_t key_name_to_keycode(const std::string &key);
  
  // Mouse keep awake state
  bool mouse_keep_awake_enabled_{false};
  uint32_t mouse_keep_awake_interval_{60000};
  uint32_t mouse_keep_awake_jitter_{0};
  uint32_t mouse_keep_awake_last_time_{0};
  uint32_t mouse_keep_awake_next_interval_{0};
  
  // Keyboard keep awake state
  bool keyboard_keep_awake_enabled_{false};
  std::string keyboard_keep_awake_key_;
  uint32_t keyboard_keep_awake_interval_{60000};
  uint32_t keyboard_keep_awake_jitter_{0};
  uint32_t keyboard_keep_awake_last_time_{0};
  uint32_t keyboard_keep_awake_next_interval_{0};
  
  // Telephony state
  bool muted_{false};
  bool off_hook_{false};
  bool ringing_{false};
  bool hold_{false};
  bool hook_button_{false};
  bool mute_button_{false};
  
  void send_telephony_report();
  
  // Telephony callbacks
  CallbackManager<void(bool)> mute_callbacks_;
  CallbackManager<void(bool)> off_hook_callbacks_;
  CallbackManager<void(bool)> ring_callbacks_;
  CallbackManager<void(bool)> hold_callbacks_;
};

// ============ Mouse Action Templates ============

template<typename... Ts>
class MoveAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(int8_t, x)
  TEMPLATABLE_VALUE(int8_t, y)
  void play(Ts... x) override { this->parent_->move(this->x_.value(x...), this->y_.value(x...)); }
};

template<typename... Ts>
class ScrollAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(int8_t, vertical)
  TEMPLATABLE_VALUE(int8_t, horizontal)
  void play(Ts... x) override { this->parent_->scroll(this->vertical_.value(x...), this->horizontal_.value(x...)); }
};

template<typename... Ts>
class ClickAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(uint8_t, button)
  void play(Ts... x) override { this->parent_->click(static_cast<MouseButton>(this->button_.value(x...))); }
};

template<typename... Ts>
class MousePressAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(uint8_t, button)
  void play(Ts... x) override { this->parent_->mouse_press(static_cast<MouseButton>(this->button_.value(x...))); }
};

template<typename... Ts>
class MouseReleaseAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(uint8_t, button)
  void play(Ts... x) override { this->parent_->mouse_release(static_cast<MouseButton>(this->button_.value(x...))); }
};

template<typename... Ts>
class MouseReleaseAllAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->mouse_release_all(); }
};

// ============ Keyboard Action Templates ============

template<typename... Ts>
class KeyPressAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(std::string, key)
  void set_modifier(uint8_t mod) { this->modifier_ = mod; }
  void play(Ts... x) override { this->parent_->key_press(this->key_.value(x...), this->modifier_); }
 protected:
  uint8_t modifier_{0};
};

template<typename... Ts>
class KeyReleaseAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->key_release(); }
};

template<typename... Ts>
class KeyTapAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(std::string, key)
  void set_modifier(uint8_t mod) { this->modifier_ = mod; }
  void play(Ts... x) override { this->parent_->key_tap(this->key_.value(x...), this->modifier_); }
 protected:
  uint8_t modifier_{0};
};

template<typename... Ts>
class KeyReleaseAllAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->key_release_all(); }
};

template<typename... Ts>
class TypeAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(std::string, text)
  TEMPLATABLE_VALUE(uint32_t, speed)
  TEMPLATABLE_VALUE(uint32_t, jitter)
  void play(Ts... x) override {
    this->parent_->type(this->text_.value(x...), this->speed_.value(x...), this->jitter_.value(x...));
  }
};

// ============ Keep Awake Action Templates ============

template<typename... Ts>
class StartMouseKeepAwakeAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(uint32_t, interval)
  TEMPLATABLE_VALUE(uint32_t, jitter)
  void play(Ts... x) override {
    this->parent_->start_mouse_keep_awake(this->interval_.value(x...), this->jitter_.value(x...));
  }
};

template<typename... Ts>
class StopMouseKeepAwakeAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override {
    this->parent_->stop_mouse_keep_awake();
  }
};

template<typename... Ts>
class StartKeyboardKeepAwakeAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(std::string, key)
  TEMPLATABLE_VALUE(uint32_t, interval)
  TEMPLATABLE_VALUE(uint32_t, jitter)
  void play(Ts... x) override {
    this->parent_->start_keyboard_keep_awake(this->key_.value(x...), this->interval_.value(x...), this->jitter_.value(x...));
  }
};

template<typename... Ts>
class StopKeyboardKeepAwakeAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override {
    this->parent_->stop_keyboard_keep_awake();
  }
};

// ============ Telephony Action Templates ============

template<typename... Ts>
class MuteAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->mute(); }
};

template<typename... Ts>
class UnmuteAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->unmute(); }
};

template<typename... Ts>
class ToggleMuteAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->toggle_mute(); }
};

template<typename... Ts>
class HookSwitchAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  TEMPLATABLE_VALUE(bool, state)
  void play(Ts... x) override { this->parent_->hook_switch(this->state_.value(x...)); }
};

template<typename... Ts>
class AnswerCallAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->answer_call(); }
};

template<typename... Ts>
class HangUpAction : public Action<Ts...>, public Parented<HIDComposite> {
 public:
  void play(Ts... x) override { this->parent_->hang_up(); }
};

}  // namespace hid_composite
}  // namespace esphome
