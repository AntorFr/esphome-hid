#include "hid_connected_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_composite {

static const char *const TAG = "hid_composite.binary_sensor";

void HIDConnectedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Composite Connected", this);
}

void HIDConnectedBinarySensor::update() {
  bool connected = this->parent_->is_connected();
  this->publish_state(connected);
}

// ============ Muted Binary Sensor ============

void HIDMutedBinarySensor::setup() {
  this->publish_state(this->parent_->is_muted());
  this->parent_->add_on_mute_callback([this](bool muted) {
    this->publish_state(muted);
  });
}

void HIDMutedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Composite Muted", this);
}

// ============ In Call Binary Sensor ============

void HIDInCallBinarySensor::setup() {
  this->publish_state(this->parent_->is_off_hook());
  this->parent_->add_on_off_hook_callback([this](bool off_hook) {
    this->publish_state(off_hook);
  });
}

void HIDInCallBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Composite In Call", this);
}

// ============ Ringing Binary Sensor ============

void HIDRingingBinarySensor::setup() {
  this->publish_state(this->parent_->is_ringing());
  this->parent_->add_on_ring_callback([this](bool ringing) {
    this->publish_state(ringing);
  });
}

void HIDRingingBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Composite Ringing", this);
}

}  // namespace hid_composite
}  // namespace esphome
