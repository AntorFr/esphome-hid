#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"

#ifdef USE_ESP32

#include <functional>

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2)
#define HID_TELEPHONY_SUPPORTED
#endif

namespace esphome {
namespace hid_telephony {

class HIDTelephony : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  // Mute control - envoie les deux rapports (Telephony + Consumer)
  void mute();
  void unmute();
  void toggle_mute();
  
  // Mute control séparé pour test
  void mute_telephony();   // Envoie uniquement le rapport Telephony (0x0B)
  void mute_consumer();    // Envoie uniquement le rapport Consumer (0x0C)
  
  // Call control
  void hook_switch();  // Toggle off-hook/on-hook
  void answer();       // Go off-hook (answer call)
  void hang_up();      // Go on-hook (end call)
  
  // State getters
  bool is_muted() const { return this->muted_; }
  bool is_off_hook() const { return this->off_hook_; }
  bool is_ringing() const { return this->ringing_; }
  bool is_connected();
  bool is_ready();
  
  // Callbacks for state changes
  void add_on_mute_callback(std::function<void(bool)> &&callback) {
    this->mute_callbacks_.add(std::move(callback));
  }
  void add_on_off_hook_callback(std::function<void(bool)> &&callback) {
    this->off_hook_callbacks_.add(std::move(callback));
  }
  void add_on_ring_callback(std::function<void(bool)> &&callback) {
    this->ring_callbacks_.add(std::move(callback));
  }

  // Called from TinyUSB callback
  void process_host_report(uint8_t const *buffer, uint16_t bufsize);

 protected:
  void send_report_();
  void send_consumer_mute_();
  
  bool initialized_{false};
  
  // Button states (what we send to host)
  bool mute_button_{false};
  bool hook_button_{false};
  
  // LED states (what host tells us)
  bool muted_{false};
  bool off_hook_{false};
  bool ringing_{false};
  bool hold_{false};
  
  // Callbacks
  CallbackManager<void(bool)> mute_callbacks_;
  CallbackManager<void(bool)> off_hook_callbacks_;
  CallbackManager<void(bool)> ring_callbacks_;
  
  friend void telephony_set_report_callback(uint8_t const *buffer, uint16_t bufsize);
};

// Global instance for callbacks
extern HIDTelephony *g_hid_telephony_instance;

// Action Templates
template<typename... Ts>
class MuteAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->mute(); }
};

template<typename... Ts>
class UnmuteAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->unmute(); }
};

template<typename... Ts>
class ToggleMuteAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->toggle_mute(); }
};

template<typename... Ts>
class MuteTelephonyAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->mute_telephony(); }
};

template<typename... Ts>
class MuteConsumerAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->mute_consumer(); }
};

template<typename... Ts>
class HookSwitchAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->hook_switch(); }
};

template<typename... Ts>
class AnswerAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->answer(); }
};

template<typename... Ts>
class HangUpAction : public Action<Ts...>, public Parented<HIDTelephony> {
 public:
  void play(Ts... x) override { this->parent_->hang_up(); }
};

}  // namespace hid_telephony
}  // namespace esphome

#endif  // USE_ESP32
