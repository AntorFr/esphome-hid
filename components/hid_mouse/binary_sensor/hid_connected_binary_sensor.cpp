#include "hid_connected_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_mouse {

static const char *const TAG = "hid_mouse.binary_sensor";

void HIDConnectedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "HID Mouse Connected", this);
}

void HIDConnectedBinarySensor::update() {
  bool connected = this->parent_->is_connected();
  this->publish_state(connected);
}

}  // namespace hid_mouse
}  // namespace esphome
