#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../hid_telephony.h"

namespace esphome {
namespace hid_telephony {

// Connected status (polling)
class TelephonyConnectedBinarySensor : public binary_sensor::BinarySensor, public PollingComponent {
 public:
  void setup() override {}
  void update() override;
  void dump_config() override;
  
  void set_parent(HIDTelephony *parent) { this->parent_ = parent; }

 protected:
  HIDTelephony *parent_{nullptr};
};

// Muted status (callback-based)
class TelephonyMutedBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void setup() override;
  void dump_config() override;
  
  void set_parent(HIDTelephony *parent) { this->parent_ = parent; }

 protected:
  HIDTelephony *parent_{nullptr};
};

// In-call status (callback-based)
class TelephonyInCallBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void setup() override;
  void dump_config() override;
  
  void set_parent(HIDTelephony *parent) { this->parent_ = parent; }

 protected:
  HIDTelephony *parent_{nullptr};
};

// Ringing status (callback-based)
class TelephonyRingingBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void setup() override;
  void dump_config() override;
  
  void set_parent(HIDTelephony *parent) { this->parent_ = parent; }

 protected:
  HIDTelephony *parent_{nullptr};
};

}  // namespace hid_telephony
}  // namespace esphome
