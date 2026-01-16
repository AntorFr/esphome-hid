#include "keep_awake_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_composite {

static const char *const TAG = "hid_composite.switch";

// Mouse Keep Awake Switch
void MouseKeepAwakeSwitch::dump_config() {
  LOG_SWITCH("", "HID Composite Mouse Keep Awake Switch", this);
  ESP_LOGCONFIG(TAG, "  Interval: %dms", this->interval_);
  ESP_LOGCONFIG(TAG, "  Jitter: %dms", this->jitter_);
}

void MouseKeepAwakeSwitch::write_state(bool state) {
  if (state) {
    this->parent_->start_mouse_keep_awake(this->interval_, this->jitter_);
  } else {
    this->parent_->stop_mouse_keep_awake();
  }
  this->publish_state(state);
}

// Keyboard Keep Awake Switch
void KeyboardKeepAwakeSwitch::dump_config() {
  LOG_SWITCH("", "HID Composite Keyboard Keep Awake Switch", this);
  ESP_LOGCONFIG(TAG, "  Key: %s", this->key_.c_str());
  ESP_LOGCONFIG(TAG, "  Interval: %dms", this->interval_);
  ESP_LOGCONFIG(TAG, "  Jitter: %dms", this->jitter_);
}

void KeyboardKeepAwakeSwitch::write_state(bool state) {
  if (state) {
    this->parent_->start_keyboard_keep_awake(this->key_, this->interval_, this->jitter_);
  } else {
    this->parent_->stop_keyboard_keep_awake();
  }
  this->publish_state(state);
}

// Mute Switch
void MuteSwitch::setup() {
  // Initialize with current state from parent
  this->publish_state(this->parent_->is_muted());
  
  // Register callback to update switch state when PC changes mute
  this->parent_->add_on_mute_callback([this](bool muted) {
    this->publish_state(muted);
  });
}

void MuteSwitch::dump_config() {
  LOG_SWITCH("", "HID Composite Mute Switch", this);
}

void MuteSwitch::write_state(bool state) {
  // Send toggle mute command to PC
  // The actual state update will come from the callback when PC confirms
  this->parent_->toggle_mute();
  
  // Note: We don't publish_state here because the PC will send us the actual state
  // via the callback. This ensures the switch reflects the real PC state.
}

}  // namespace hid_composite
}  // namespace esphome
