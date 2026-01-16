#include "hid_mouse.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

#ifdef HID_MOUSE_SUPPORTED

#include "tinyusb.h"
#include "tusb.h"
#include "class/hid/hid_device.h"

namespace esphome {
namespace hid_mouse {

static const char *const TAG = "hid_mouse";

// Singleton for callbacks
static HIDMouse *g_hid_mouse_instance = nullptr;

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
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x03,        //     Input (Const,Var,Abs) - padding
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel)
    0xC0,              //   End Collection
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
    .idProduct = 0x4002, // Custom PID for HID Mouse
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
    "HID Mouse",                  // 2: Product
    "123456",                     // 3: Serial
};

// Configuration descriptor
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

static const uint8_t configuration_descriptor[] = {
    // Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, 0, 100),
    // HID Descriptor
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_MOUSE, sizeof(hid_report_descriptor), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10),
};

// TinyUSB callbacks
extern "C" {

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type, uint8_t *buffer,
                                uint16_t reqlen) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}

}  // extern "C"

void HIDMouse::setup() {
  ESP_LOGI(TAG, "Setting up HID Mouse...");
  
  g_hid_mouse_instance = this;

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

  ESP_LOGI(TAG, "TinyUSB driver installed successfully");
  this->initialized_ = true;
}

void HIDMouse::loop() {
  if (!this->initialized_) {
    return;
  }

  // Send pending report if device is ready
  if (this->report_pending_ && tud_mounted() && tud_hid_ready()) {
    this->send_report_();
  }
}

void HIDMouse::dump_config() {
  ESP_LOGCONFIG(TAG, "HID Mouse:");
  ESP_LOGCONFIG(TAG, "  Status: %s", this->initialized_ ? "Initialized" : "Not initialized");
}

void HIDMouse::send_report_() {
  if (!tud_mounted() || !tud_hid_ready()) {
    return;
  }

  uint8_t report[4] = {
    this->buttons_,
    (uint8_t)this->x_,
    (uint8_t)this->y_,
    (uint8_t)this->wheel_
  };

  if (tud_hid_report(0, report, sizeof(report))) {
    ESP_LOGD(TAG, "Report sent: buttons=%02X x=%d y=%d wheel=%d", 
             this->buttons_, this->x_, this->y_, this->wheel_);
  } else {
    ESP_LOGW(TAG, "Failed to send HID report");
  }

  // Clear movement values after sending
  this->x_ = 0;
  this->y_ = 0;
  this->wheel_ = 0;
  this->report_pending_ = false;
}

void HIDMouse::move(int8_t x, int8_t y) {
  ESP_LOGD(TAG, "Move: x=%d y=%d", x, y);
  this->x_ = x;
  this->y_ = y;
  this->report_pending_ = true;
  
  // Try to send immediately if ready
  if (this->initialized_ && tud_mounted() && tud_hid_ready()) {
    this->send_report_();
  }
}

void HIDMouse::click(MouseButton button) {
  ESP_LOGD(TAG, "Click: button=%d", button);
  this->press(button);
  delay(50);
  this->release(button);
}

void HIDMouse::press(MouseButton button) {
  ESP_LOGD(TAG, "Press: button=%d", button);
  this->buttons_ |= button;
  this->report_pending_ = true;
  
  if (this->initialized_ && tud_mounted() && tud_hid_ready()) {
    this->send_report_();
  }
}

void HIDMouse::release(MouseButton button) {
  ESP_LOGD(TAG, "Release: button=%d", button);
  this->buttons_ &= ~button;
  this->report_pending_ = true;
  
  if (this->initialized_ && tud_mounted() && tud_hid_ready()) {
    this->send_report_();
  }
}

void HIDMouse::scroll(int8_t amount) {
  ESP_LOGD(TAG, "Scroll: amount=%d", amount);
  this->wheel_ = amount;
  this->report_pending_ = true;
  
  if (this->initialized_ && tud_mounted() && tud_hid_ready()) {
    this->send_report_();
  }
}

}  // namespace hid_mouse
}  // namespace esphome

#endif  // HID_MOUSE_SUPPORTED

#else  // USE_ESP32

// Stubs for non-ESP32 platforms
namespace esphome {
namespace hid_mouse {
void HIDMouse::setup() {}
void HIDMouse::loop() {}
void HIDMouse::dump_config() {}
void HIDMouse::move(int8_t x, int8_t y) {}
void HIDMouse::click(MouseButton button) {}
void HIDMouse::press(MouseButton button) {}
void HIDMouse::release(MouseButton button) {}
void HIDMouse::scroll(int8_t amount) {}
}  // namespace hid_mouse
}  // namespace esphome

#endif  // USE_ESP32
