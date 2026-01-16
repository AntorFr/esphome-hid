#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../hid_mouse.h"

namespace esphome {
namespace hid_mouse {

class HIDConnectedBinarySensor : public binary_sensor::BinarySensor, public PollingComponent {
 public:
  void setup() override {}
  void update() override;
  void dump_config() override;
  
  void set_parent(HIDMouse *parent) { this->parent_ = parent; }

 protected:
  HIDMouse *parent_{nullptr};
};

}  // namespace hid_mouse
}  // namespace esphome
