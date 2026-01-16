#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"

#ifdef USE_ESP32

#include "esp_log.h"

// Check for ESP32-S2, ESP32-S3, or ESP32-P4 (chips with USB OTG)
#if defined(USE_ESP32_VARIANT_ESP32S2) || defined(USE_ESP32_VARIANT_ESP32S3) || defined(USE_ESP32_VARIANT_ESP32P4)
#define HID_MOUSE_SUPPORTED
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#endif

namespace esphome {
namespace hid_mouse {

enum MouseButton : uint8_t {
  MOUSE_BUTTON_LEFT = 0x01,
  MOUSE_BUTTON_RIGHT = 0x02,
  MOUSE_BUTTON_MIDDLE = 0x04,
};

class HIDMouse : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  // Mouse actions
  void move(int8_t x, int8_t y);
  void click(MouseButton button);
  void press(MouseButton button);
  void release(MouseButton button);
  void scroll(int8_t amount);

 protected:
  void send_report_();
  
  uint8_t buttons_{0};
  int8_t x_{0};
  int8_t y_{0};
  int8_t wheel_{0};
  bool report_pending_{false};
  bool initialized_{false};
};

// Action: Move
template<typename... Ts> class MoveAction : public Action<Ts...>, public Parented<HIDMouse> {
 public:
  TEMPLATABLE_VALUE(int, x)
  TEMPLATABLE_VALUE(int, y)

  void play(Ts... x) override {
    this->parent_->move(this->x_.value(x...), this->y_.value(x...));
  }
};

// Action: Click
template<typename... Ts> class ClickAction : public Action<Ts...>, public Parented<HIDMouse> {
 public:
  void set_button(MouseButton button) { this->button_ = button; }

  void play(Ts... x) override {
    this->parent_->click(this->button_);
  }

 protected:
  MouseButton button_{MOUSE_BUTTON_LEFT};
};

// Action: Press
template<typename... Ts> class PressAction : public Action<Ts...>, public Parented<HIDMouse> {
 public:
  void set_button(MouseButton button) { this->button_ = button; }

  void play(Ts... x) override {
    this->parent_->press(this->button_);
  }

 protected:
  MouseButton button_{MOUSE_BUTTON_LEFT};
};

// Action: Release
template<typename... Ts> class ReleaseAction : public Action<Ts...>, public Parented<HIDMouse> {
 public:
  void set_button(MouseButton button) { this->button_ = button; }

  void play(Ts... x) override {
    this->parent_->release(this->button_);
  }

 protected:
  MouseButton button_{MOUSE_BUTTON_LEFT};
};

// Action: Scroll
template<typename... Ts> class ScrollAction : public Action<Ts...>, public Parented<HIDMouse> {
 public:
  TEMPLATABLE_VALUE(int, amount)

  void play(Ts... x) override {
    this->parent_->scroll(this->amount_.value(x...));
  }
};

}  // namespace hid_mouse
}  // namespace esphome

#endif  // USE_ESP32
