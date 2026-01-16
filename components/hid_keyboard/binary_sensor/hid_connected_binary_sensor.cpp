#include "hid_connected_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_keyboard {

static const char *const TAG = "hid_keyboard.binary_sensor";

void HIDConnectedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Keyboard Connected", this);
}

void HIDConnectedBinarySensor::update() {
  bool connected = this->parent_->is_connected();
  this->publish_state(connected);
}

}  // namespace hid_keyboard
}  // namespace esphome
