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
  void type(const std::string &text);

 protected:
  bool initialized_{false};
  uint8_t mouse_buttons_{0};

  void send_mouse_report();
  void send_keyboard_report(uint8_t modifier, uint8_t keycode);
  void char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier);
  uint8_t key_name_to_keycode(const std::string &key);
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
  void play(Ts... x) override { this->parent_->type(this->text_.value(x...)); }
};

}  // namespace hid_composite
}  // namespace esphome
