#include "telephony_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hid_telephony {

static const char *const TAG = "hid_telephony.switch";

void MuteSwitch::setup() {
  // Initialize with current state from parent
  this->publish_state(this->parent_->is_muted());
  
  // Register callback to update switch state when PC changes mute
  this->parent_->add_on_mute_callback([this](bool muted) {
    this->publish_state(muted);
  });
}

void MuteSwitch::dump_config() {
  LOG_SWITCH("", "HID Telephony Mute Switch", this);
}

void MuteSwitch::write_state(bool state) {
  // Send toggle mute command to PC
  // The actual state update will come from the callback when PC confirms
  this->parent_->toggle_mute();
  
  // Note: We don't publish_state here because the PC will send us the actual state
  // via the callback. This ensures the switch reflects the real PC state.
}

}  // namespace hid_telephony
}  // namespace esphome
