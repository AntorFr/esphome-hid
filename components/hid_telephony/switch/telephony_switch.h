#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../hid_telephony.h"

namespace esphome {
namespace hid_telephony {

class MuteSwitch : public switch_::Switch, public Component {
 public:
  void setup() override;
  void dump_config() override;
  
  void set_parent(HIDTelephony *parent) { this->parent_ = parent; }

 protected:
  void write_state(bool state) override;
  
  HIDTelephony *parent_{nullptr};
};

}  // namespace hid_telephony
}  // namespace esphome
