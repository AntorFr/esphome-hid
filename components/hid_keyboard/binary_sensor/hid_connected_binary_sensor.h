#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../hid_keyboard.h"

namespace esphome {
namespace hid_keyboard {

class HIDConnectedBinarySensor : public binary_sensor::BinarySensor, public PollingComponent {
 public:
  void setup() override {}
  void update() override;
  void dump_config() override;
  
  void set_parent(HIDKeyboard *parent) { this->parent_ = parent; }

 protected:
  HIDKeyboard *parent_{nullptr};
};

}  // namespace hid_keyboard
}  // namespace esphome
