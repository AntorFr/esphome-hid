#include "hid_mouse.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

namespace esphome {
namespace hid_mouse {

static const char *const TAG = "hid_mouse";

#ifdef HID_MOUSE_SUPPORTED

// HID Mouse Report structure (4 bytes)
typedef struct __attribute__((packed)) {
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t wheel;
} hid_mouse_report_t;

// HID Report Descriptor for Mouse (Boot protocol compatible)
static const uint8_t hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

// TinyUSB HID callbacks
extern "C" {

// Return the HID report descriptor
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  (void) instance;
  return hid_report_descriptor;
}

// Return the size of HID report descriptor
uint16_t tud_hid_descriptor_report_cb_len(uint8_t instance) {
  (void) instance;
  return sizeof(hid_report_descriptor);
}

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, 
                                hid_report_type_t report_type, uint8_t *buffer, 
                                uint16_t reqlen) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}

// Invoked when received SET_REPORT control request
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, 
                           hid_report_type_t report_type, uint8_t const *buffer, 
                           uint16_t bufsize) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

}  // extern "C"

void HIDMouse::setup() {
  ESP_LOGI(TAG, "Setting up HID Mouse...");
  
  // Wait a bit for USB to initialize
  delay(100);
  
  if (tud_mounted()) {
    this->initialized_ = true;
    ESP_LOGI(TAG, "HID Mouse initialized - USB mounted");
  } else {
    ESP_LOGW(TAG, "USB not mounted yet, will retry in loop");
  }
}

void HIDMouse::loop() {
  // Check if USB got mounted
  if (!this->initialized_ && tud_mounted()) {
    this->initialized_ = true;
    ESP_LOGI(TAG, "HID Mouse: USB now mounted");
  }

  if (!this->initialized_) {
    return;
  }

  // Process pending reports when HID is ready
  if (this->report_pending_ && tud_hid_ready()) {
    this->send_report_();
  }
}

void HIDMouse::dump_config() {
  ESP_LOGCONFIG(TAG, "HID Mouse:");
  ESP_LOGCONFIG(TAG, "  USB Mounted: %s", tud_mounted() ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  HID Ready: %s", tud_hid_ready() ? "YES" : "NO");
}

void HIDMouse::move(int8_t x, int8_t y) {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "HID Mouse not initialized");
    return;
  }

  this->x_ = x;
  this->y_ = y;
  this->report_pending_ = true;
  
  ESP_LOGD(TAG, "Move: x=%d, y=%d", x, y);
}

void HIDMouse::click(MouseButton button) {
  this->press(button);
  // Send the press report immediately if possible
  if (tud_hid_ready()) {
    this->send_report_();
  }
  // Small delay for click
  delay(10);
  this->release(button);
  
  ESP_LOGD(TAG, "Click: button=0x%02X", button);
}

void HIDMouse::press(MouseButton button) {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "HID Mouse not initialized");
    return;
  }

  this->buttons_ |= button;
  this->report_pending_ = true;
  
  ESP_LOGD(TAG, "Press: button=0x%02X, buttons=0x%02X", button, this->buttons_);
}

void HIDMouse::release(MouseButton button) {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "HID Mouse not initialized");
    return;
  }

  this->buttons_ &= ~button;
  this->report_pending_ = true;
  
  ESP_LOGD(TAG, "Release: button=0x%02X, buttons=0x%02X", button, this->buttons_);
}

void HIDMouse::scroll(int8_t amount) {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "HID Mouse not initialized");
    return;
  }

  this->wheel_ = amount;
  this->report_pending_ = true;
  
  ESP_LOGD(TAG, "Scroll: amount=%d", amount);
}

void HIDMouse::send_report_() {
  hid_mouse_report_t report;
  report.buttons = this->buttons_;
  report.x = this->x_;
  report.y = this->y_;
  report.wheel = this->wheel_;

  // Send report using generic HID report function (no report ID for boot protocol)
  if (tud_hid_report(0, &report, sizeof(report))) {
    ESP_LOGV(TAG, "Report sent: buttons=0x%02X, x=%d, y=%d, wheel=%d", 
             this->buttons_, this->x_, this->y_, this->wheel_);
  } else {
    ESP_LOGW(TAG, "Failed to send HID report");
  }

  // Reset movement and scroll after sending (keep button state)
  this->x_ = 0;
  this->y_ = 0;
  this->wheel_ = 0;
  this->report_pending_ = false;
}

#else  // HID_MOUSE_SUPPORTED

void HIDMouse::setup() {
  ESP_LOGE(TAG, "HID Mouse is only supported on ESP32-S2, ESP32-S3, and ESP32-P4");
  this->mark_failed();
}

void HIDMouse::loop() {}
void HIDMouse::dump_config() {
  ESP_LOGCONFIG(TAG, "HID Mouse: NOT SUPPORTED on this chip variant");
}
void HIDMouse::move(int8_t x, int8_t y) {}
void HIDMouse::click(MouseButton button) {}
void HIDMouse::press(MouseButton button) {}
void HIDMouse::release(MouseButton button) {}
void HIDMouse::scroll(int8_t amount) {}
void HIDMouse::send_report_() {}

#endif  // HID_MOUSE_SUPPORTED

}  // namespace hid_mouse
}  // namespace esphome

#endif  // USE_ESP32
