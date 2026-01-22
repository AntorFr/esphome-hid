#include "hid_telephony.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

#ifdef HID_TELEPHONY_SUPPORTED

#include "tinyusb.h"
#include "tusb.h"
#include "class/hid/hid_device.h"

namespace esphome {
namespace hid_telephony {

static const char *const TAG = "hid_telephony";

// Global instance for callbacks
HIDTelephony *g_hid_telephony_instance = nullptr;

// Report IDs
#define REPORT_ID_TELEPHONY 1
#define REPORT_ID_CONSUMER 2

// Telephony HID Report Descriptor
// Includes BOTH Telephony Page (0x0B) AND Consumer Control Page (0x0C)
// to test which one Teams recognizes
static const uint8_t hid_report_descriptor[] = {
    // ========== TELEPHONY DEVICE (Headset) ==========
    0x05, 0x0B,        // Usage Page (Telephony Devices)
    0x09, 0x05,        // Usage (Headset)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_TELEPHONY,  // Report ID (1)
    
    // ===== INPUT REPORT (Buttons we send to host) =====
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    
    // Hook Switch (Answer/End call) - bit 0
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x20,        //   Usage (Hook Switch)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Phone Mute - bit 1
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x2F,        //   Usage (Phone Mute)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Padding to make 1 byte - bits 2-7
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x03,        //   Input (Constant)
    
    // ===== OUTPUT REPORT (LEDs host sends to us) =====
    0x75, 0x01,        //   Report Size (1)
    
    // Mute LED - bit 0
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x9E,        //   Usage (Mute)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    
    // Off Hook LED - bit 1
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x17,        //   Usage (Off Hook)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    
    // Ring LED - bit 2
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x18,        //   Usage (Ring)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    
    // Padding - bits 3-7
    0x95, 0x05,        //   Report Count (5)
    0x91, 0x03,        //   Output (Constant)
    
    0xC0,              // End Collection
    
    // ========== CONSUMER CONTROL (for Teams/Zoom/etc) ==========
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_CONSUMER,  // Report ID (2)
    
    // Mute button (Consumer style)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x01,        //   Report Count (1)
    0x09, 0xE2,        //   Usage (Mute) - Consumer Mute
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Volume Up
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Volume Down
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Padding - bits 3-7
    0x95, 0x05,        //   Report Count (5)
    0x81, 0x03,        //   Input (Constant)
    
    0xC0,              // End Collection
};

// USB Device Descriptor
static const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,  // Espressif VID
    .idProduct = 0x4005, // Custom PID for Telephony
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

// String descriptors
static const char *string_descriptors[] = {
    (const char[]){0x09, 0x04},  // 0: Language - English
    "ESPHome",                    // 1: Manufacturer
    "HID Telephony",              // 2: Product
    "123456",                     // 3: Serial
};

// Configuration descriptor
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#define EPNUM_HID_IN  0x81
#define EPNUM_HID_OUT 0x01

static const uint8_t configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_INOUT_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_descriptor), 
                             EPNUM_HID_OUT, EPNUM_HID_IN, CFG_TUD_HID_EP_BUFSIZE, 10),
};

// TinyUSB callbacks
extern "C" {

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  return hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type, uint8_t *buffer,
                                uint16_t reqlen) {
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // Host is sending us LED states
  if (report_type == HID_REPORT_TYPE_OUTPUT && g_hid_telephony_instance != nullptr) {
    g_hid_telephony_instance->process_host_report(buffer, bufsize);
  }
}

}  // extern "C"

void HIDTelephony::setup() {
  ESP_LOGI(TAG, "Setting up HID Telephony...");
  
  g_hid_telephony_instance = this;

  // Configure TinyUSB with our custom descriptors
  tinyusb_config_t tusb_cfg = {
    .port = TINYUSB_PORT_FULL_SPEED_0,
    .phy = {
      .skip_setup = false,
      .self_powered = false,
      .vbus_monitor_io = -1,
    },
    .task = {
      .size = 4096,
      .priority = 5,
      .xCoreID = 0,
    },
    .descriptor = {
      .device = &device_descriptor,
      .qualifier = nullptr,
      .string = string_descriptors,
      .string_count = sizeof(string_descriptors) / sizeof(string_descriptors[0]),
      .full_speed_config = configuration_descriptor,
      .high_speed_config = nullptr,
    },
    .event_cb = nullptr,
    .event_arg = nullptr,
  };

  esp_err_t ret = tinyusb_driver_install(&tusb_cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "TinyUSB driver install failed: %s", esp_err_to_name(ret));
    return;
  }

  ESP_LOGI(TAG, "HID Telephony initialized successfully");
  this->initialized_ = true;
}

void HIDTelephony::loop() {
  // Nothing to do in loop for now
}

void HIDTelephony::dump_config() {
  ESP_LOGCONFIG(TAG, "HID Telephony:");
  ESP_LOGCONFIG(TAG, "  Initialized: %s", this->initialized_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Telephony Page (0x0B) + Consumer Page (0x0C) enabled");
}

void HIDTelephony::send_report_() {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) {
    return;
  }

  // Build Telephony report: [Hook Switch, Mute, padding...]
  uint8_t telephony_report = 0;
  if (this->hook_button_) telephony_report |= 0x01;  // Hook Switch - bit 0
  if (this->mute_button_) telephony_report |= 0x02;  // Phone Mute - bit 1
  
  tud_hid_report(REPORT_ID_TELEPHONY, &telephony_report, 1);
  ESP_LOGI(TAG, "Sent TELEPHONY report (ID=%d): hook=%d, mute=%d", 
           REPORT_ID_TELEPHONY, this->hook_button_, this->mute_button_);
}

void HIDTelephony::send_consumer_mute_() {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) {
    return;
  }

  // Build Consumer report: [Mute, Vol+, Vol-, padding...]
  uint8_t consumer_report = 0;
  if (this->mute_button_) consumer_report |= 0x01;  // Consumer Mute - bit 0
  
  tud_hid_report(REPORT_ID_CONSUMER, &consumer_report, 1);
  ESP_LOGI(TAG, "Sent CONSUMER report (ID=%d): mute=%d", 
           REPORT_ID_CONSUMER, this->mute_button_);
}

void HIDTelephony::process_host_report(uint8_t const *buffer, uint16_t bufsize) {
  if (bufsize < 1) return;
  
  uint8_t leds = buffer[0];
  
  bool new_muted = (leds & 0x01) != 0;     // Mute LED
  bool new_off_hook = (leds & 0x02) != 0;  // Off Hook LED
  bool new_ringing = (leds & 0x04) != 0;   // Ring LED
  bool new_hold = (leds & 0x08) != 0;      // Hold LED
  
  // Check for changes and trigger callbacks
  if (new_muted != this->muted_) {
    this->muted_ = new_muted;
    ESP_LOGI(TAG, "Mute state changed: %s", new_muted ? "MUTED" : "UNMUTED");
    this->mute_callbacks_.call(new_muted);
  }
  
  if (new_off_hook != this->off_hook_) {
    this->off_hook_ = new_off_hook;
    ESP_LOGI(TAG, "Off-hook state changed: %s", new_off_hook ? "IN CALL" : "IDLE");
    this->off_hook_callbacks_.call(new_off_hook);
  }
  
  if (new_ringing != this->ringing_) {
    this->ringing_ = new_ringing;
    ESP_LOGI(TAG, "Ring state changed: %s", new_ringing ? "RINGING" : "NOT RINGING");
    this->ring_callbacks_.call(new_ringing);
  }
  
  this->hold_ = new_hold;
}

void HIDTelephony::mute() {
  ESP_LOGI(TAG, "Sending mute button press (Telephony + Consumer)");
  this->mute_button_ = true;
  
  // Send BOTH reports to test which one Teams recognizes
  this->send_report_();           // Telephony Page (0x0B) - Report ID 1
  delay(10);
  this->send_consumer_mute_();    // Consumer Page (0x0C) - Report ID 2
  
  delay(50);
  
  this->mute_button_ = false;
  this->send_report_();           // Release Telephony
  delay(10);
  this->send_consumer_mute_();    // Release Consumer
}

void HIDTelephony::mute_telephony() {
  ESP_LOGI(TAG, "Sending TELEPHONY mute only (Page 0x0B, Usage 0x2F)");
  this->mute_button_ = true;
  this->send_report_();
  delay(50);
  this->mute_button_ = false;
  this->send_report_();
}

void HIDTelephony::mute_consumer() {
  ESP_LOGI(TAG, "Sending CONSUMER mute only (Page 0x0C, Usage 0xE2)");
  this->mute_button_ = true;
  this->send_consumer_mute_();
  delay(50);
  this->mute_button_ = false;
  this->send_consumer_mute_();
}

void HIDTelephony::unmute() {
  // Same as mute - it's a toggle button
  this->mute();
}

void HIDTelephony::toggle_mute() {
  this->mute();
}

void HIDTelephony::hook_switch() {
  ESP_LOGI(TAG, "Sending hook switch press");
  this->hook_button_ = true;
  this->send_report_();
  delay(50);
  this->hook_button_ = false;
  this->send_report_();
}

void HIDTelephony::answer() {
  if (!this->off_hook_) {
    this->hook_switch();
  }
}

void HIDTelephony::hang_up() {
  if (this->off_hook_) {
    this->hook_switch();
  }
}

bool HIDTelephony::is_connected() {
  if (!this->initialized_) return false;
  // tud_mounted() alone is not enough when behind a hub:
  // the hub may keep the device enumerated even when the PC is disconnected.
  // tud_suspended() detects when the host stops sending SOF frames (~3ms),
  // which happens when the PC is disconnected from the hub.
  return tud_mounted() && !tud_suspended();
}

bool HIDTelephony::is_ready() {
  if (!this->initialized_) return false;
  return tud_mounted() && !tud_suspended() && tud_hid_ready();
}

}  // namespace hid_telephony
}  // namespace esphome

#else  // HID_TELEPHONY_SUPPORTED

namespace esphome {
namespace hid_telephony {

static const char *const TAG = "hid_telephony";
HIDTelephony *g_hid_telephony_instance = nullptr;

void HIDTelephony::setup() { ESP_LOGE(TAG, "Only supported on ESP32-S3/S2"); }
void HIDTelephony::loop() {}
void HIDTelephony::dump_config() {}
void HIDTelephony::mute() {}
void HIDTelephony::unmute() {}
void HIDTelephony::toggle_mute() {}
void HIDTelephony::hook_switch() {}
void HIDTelephony::answer() {}
void HIDTelephony::hang_up() {}
void HIDTelephony::send_report_() {}
void HIDTelephony::process_host_report_(uint8_t const *buffer, uint16_t bufsize) {}
bool HIDTelephony::is_connected() { return false; }
bool HIDTelephony::is_ready() { return false; }

}  // namespace hid_telephony
}  // namespace esphome

#endif  // HID_TELEPHONY_SUPPORTED
#endif  // USE_ESP32
