#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../hid_keyboard.h"

namespace esphome {
namespace hid_keyboard {

class KeepAwakeSwitch : public switch_::Switch, public Component {
 public:
  void setup() override {}
  void dump_config() override;
  
  void set_parent(HIDKeyboard *parent) { this->parent_ = parent; }
  void set_key(const std::string &key) { this->key_ = key; }
  void set_interval(uint32_t interval) { this->interval_ = interval; }
  void set_jitter(uint32_t jitter) { this->jitter_ = jitter; }

 protected:
  void write_state(bool state) override;
  
  HIDKeyboard *parent_{nullptr};
  std::string key_;
  uint32_t interval_{60000};
  uint32_t jitter_{0};
};

}  // namespace hid_keyboard
}  // namespace esphome
