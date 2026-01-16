#include "telephony_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_telephony {

static const char *const TAG = "hid_telephony.binary_sensor";

// Connected sensor
void TelephonyConnectedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Telephony Connected", this);
}

void TelephonyConnectedBinarySensor::update() {
  bool connected = this->parent_->is_connected();
  this->publish_state(connected);
}

// Muted sensor
void TelephonyMutedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Telephony Muted", this);
}

void TelephonyMutedBinarySensor::setup() {
  // Register callback with parent
  this->parent_->add_on_mute_callback([this](bool muted) {
    this->publish_state(muted);
  });
  // Publish initial state
  this->publish_state(this->parent_->is_muted());
}

// In-call sensor
void TelephonyInCallBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Telephony In Call", this);
}

void TelephonyInCallBinarySensor::setup() {
  // Register callback with parent
  this->parent_->add_on_off_hook_callback([this](bool off_hook) {
    this->publish_state(off_hook);
  });
  // Publish initial state
  this->publish_state(this->parent_->is_off_hook());
}

// Ringing sensor
void TelephonyRingingBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Telephony Ringing", this);
}

void TelephonyRingingBinarySensor::setup() {
  // Register callback with parent
  this->parent_->add_on_ring_callback([this](bool ringing) {
    this->publish_state(ringing);
  });
  // Publish initial state
  this->publish_state(this->parent_->is_ringing());
}

}  // namespace hid_telephony
}  // namespace esphome
