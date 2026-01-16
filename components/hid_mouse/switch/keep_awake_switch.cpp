#include "keep_awake_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_mouse {

static const char *const TAG = "hid_mouse.switch";

void KeepAwakeSwitch::dump_config() {
  LOG_SWITCH("", "HID Mouse Keep Awake Switch", this);
  ESP_LOGCONFIG(TAG, "  Interval: %dms", this->interval_);
  ESP_LOGCONFIG(TAG, "  Jitter: %dms", this->jitter_);
}

void KeepAwakeSwitch::write_state(bool state) {
  if (state) {
    this->parent_->start_keep_awake(this->interval_, this->jitter_);
  } else {
    this->parent_->stop_keep_awake();
  }
  this->publish_state(state);
}

}  // namespace hid_mouse
}  // namespace esphome
