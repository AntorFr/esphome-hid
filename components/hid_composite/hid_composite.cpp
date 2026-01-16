#include "hid_composite.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

#ifdef HID_COMPOSITE_SUPPORTED

#include "tinyusb.h"
#include "tusb.h"
#include "class/hid/hid_device.h"

namespace esphome {
namespace hid_composite {

static const char *const TAG = "hid_composite";

// Global instance for TinyUSB callback
static HIDComposite *g_hid_composite_instance = nullptr;

// Report IDs
#define REPORT_ID_KEYBOARD   1
#define REPORT_ID_MOUSE      2
#define REPORT_ID_TELEPHONY  3

// Key codes
enum KeyCode : uint8_t {
  KEY_NONE = 0x00,
  KEY_A = 0x04, KEY_B = 0x05, KEY_C = 0x06, KEY_D = 0x07, KEY_E = 0x08, KEY_F = 0x09,
  KEY_G = 0x0A, KEY_H = 0x0B, KEY_I = 0x0C, KEY_J = 0x0D, KEY_K = 0x0E, KEY_L = 0x0F,
  KEY_M = 0x10, KEY_N = 0x11, KEY_O = 0x12, KEY_P = 0x13, KEY_Q = 0x14, KEY_R = 0x15,
  KEY_S = 0x16, KEY_T = 0x17, KEY_U = 0x18, KEY_V = 0x19, KEY_W = 0x1A, KEY_X = 0x1B,
  KEY_Y = 0x1C, KEY_Z = 0x1D,
  KEY_1 = 0x1E, KEY_2 = 0x1F, KEY_3 = 0x20, KEY_4 = 0x21, KEY_5 = 0x22,
  KEY_6 = 0x23, KEY_7 = 0x24, KEY_8 = 0x25, KEY_9 = 0x26, KEY_0 = 0x27,
  KEY_ENTER = 0x28, KEY_ESC = 0x29, KEY_BACKSPACE = 0x2A, KEY_TAB = 0x2B, KEY_SPACE = 0x2C,
  KEY_MINUS = 0x2D, KEY_EQUAL = 0x2E, KEY_LEFT_BRACE = 0x2F, KEY_RIGHT_BRACE = 0x30,
  KEY_BACKSLASH = 0x31, KEY_SEMICOLON = 0x33, KEY_APOSTROPHE = 0x34, KEY_GRAVE = 0x35,
  KEY_COMMA = 0x36, KEY_PERIOD = 0x37, KEY_SLASH = 0x38, KEY_CAPS_LOCK = 0x39,
  KEY_F1 = 0x3A, KEY_F2 = 0x3B, KEY_F3 = 0x3C, KEY_F4 = 0x3D, KEY_F5 = 0x3E, KEY_F6 = 0x3F,
  KEY_F7 = 0x40, KEY_F8 = 0x41, KEY_F9 = 0x42, KEY_F10 = 0x43, KEY_F11 = 0x44, KEY_F12 = 0x45,
  KEY_PRINT_SCREEN = 0x46, KEY_SCROLL_LOCK = 0x47, KEY_PAUSE = 0x48,
  KEY_INSERT = 0x49, KEY_HOME = 0x4A, KEY_PAGE_UP = 0x4B,
  KEY_DELETE = 0x4C, KEY_END = 0x4D, KEY_PAGE_DOWN = 0x4E,
  KEY_RIGHT_ARROW = 0x4F, KEY_LEFT_ARROW = 0x50, KEY_DOWN_ARROW = 0x51, KEY_UP_ARROW = 0x52,
};

// Composite HID Report Descriptor (Keyboard + Mouse)
static const uint8_t hid_report_descriptor[] = {
    // Keyboard
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_KEYBOARD, // Report ID
    0x05, 0x07,        //   Usage Page (Keyboard)
    0x19, 0xE0,        //   Usage Minimum (Left Control)
    0x29, 0xE7,        //   Usage Maximum (Right GUI)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x05,        //   Usage Maximum (Kana)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Constant)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Keyboard)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data, Array)
    0xC0,              // End Collection

    // Mouse
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x85, REPORT_ID_MOUSE, // Report ID
    0x05, 0x09,        //     Usage Page (Buttons)
    0x19, 0x01,        //     Usage Minimum (Button 1)
    0x29, 0x03,        //     Usage Maximum (Button 3)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x01,        //     Input (Constant)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0x05, 0x0C,        //     Usage Page (Consumer)
    0x0A, 0x38, 0x02,  //     Usage (AC Pan)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0xC0,              //   End Collection
    0xC0,              // End Collection

    // Telephony (Headset)
    0x05, 0x0B,        // Usage Page (Telephony Devices)
    0x09, 0x05,        // Usage (Headset)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_TELEPHONY, // Report ID
    
    // Input Report (buttons we send to host)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x09, 0x20,        //   Usage (Hook Switch)
    0x09, 0x2F,        //   Usage (Phone Mute)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x95, 0x06,        //   Report Count (6) - padding
    0x81, 0x03,        //   Input (Constant)
    
    // Output Report (LEDs from host)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x09, 0x9E,        //   Usage (Mute LED)
    0x09, 0x17,        //   Usage (Off Hook LED)
    0x09, 0x18,        //   Usage (Ring LED)
    0x09, 0x2A,        //   Usage (Hold LED)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x04,        //   Report Count (4)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    0x95, 0x04,        //   Report Count (4) - padding
    0x91, 0x03,        //   Output (Constant)
    0xC0,              // End Collection
};

static const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,
    .idProduct = 0x4004,  // Different from mouse-only and keyboard-only
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

static const char *string_descriptors[] = {
    (const char[]){0x09, 0x04},
    "ESPHome",
    "HID Composite",
    "123456",
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

static const uint8_t configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_descriptor), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10),
};

extern "C" {
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { return hid_report_descriptor; }
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) { return 0; }

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  // Host is sending us LED states (for telephony)
  if (report_type == HID_REPORT_TYPE_OUTPUT && report_id == REPORT_ID_TELEPHONY && g_hid_composite_instance != nullptr) {
    g_hid_composite_instance->process_host_report(buffer, bufsize);
  }
}
}

void HIDComposite::setup() {
  ESP_LOGI(TAG, "Setting up HID Composite (Mouse + Keyboard + Telephony)...");
  
  g_hid_composite_instance = this;
  
  tinyusb_config_t tusb_cfg = {
    .port = TINYUSB_PORT_FULL_SPEED_0,
    .phy = { .skip_setup = false, .self_powered = false, .vbus_monitor_io = -1, },
    .task = { .size = 4096, .priority = 5, .xCoreID = 0, },
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

void HIDComposite::loop() {
  if (!this->initialized_) return;
  
  uint32_t now = millis();
  
  // Handle mouse keep awake
  if (this->mouse_keep_awake_enabled_) {
    if (now - this->mouse_keep_awake_last_time_ >= this->mouse_keep_awake_next_interval_) {
      int8_t dx = (rand() % 3) - 1;
      int8_t dy = (rand() % 3) - 1;
      if (dx == 0 && dy == 0) dx = 1;
      this->move(dx, dy);
      ESP_LOGD(TAG, "Mouse keep awake: move(%d, %d)", dx, dy);
      
      this->mouse_keep_awake_next_interval_ = this->mouse_keep_awake_interval_;
      if (this->mouse_keep_awake_jitter_ > 0) {
        int32_t jitter = (rand() % (this->mouse_keep_awake_jitter_ * 2 + 1)) - this->mouse_keep_awake_jitter_;
        this->mouse_keep_awake_next_interval_ = (int32_t)this->mouse_keep_awake_interval_ + jitter > 1000 
                                                 ? this->mouse_keep_awake_interval_ + jitter : 1000;
      }
      this->mouse_keep_awake_last_time_ = now;
    }
  }
  
  // Handle keyboard keep awake
  if (this->keyboard_keep_awake_enabled_) {
    if (now - this->keyboard_keep_awake_last_time_ >= this->keyboard_keep_awake_next_interval_) {
      this->key_tap(this->keyboard_keep_awake_key_);
      ESP_LOGD(TAG, "Keyboard keep awake: tap(%s)", this->keyboard_keep_awake_key_.c_str());
      
      this->keyboard_keep_awake_next_interval_ = this->keyboard_keep_awake_interval_;
      if (this->keyboard_keep_awake_jitter_ > 0) {
        int32_t jitter = (rand() % (this->keyboard_keep_awake_jitter_ * 2 + 1)) - this->keyboard_keep_awake_jitter_;
        this->keyboard_keep_awake_next_interval_ = (int32_t)this->keyboard_keep_awake_interval_ + jitter > 1000 
                                                   ? this->keyboard_keep_awake_interval_ + jitter : 1000;
      }
      this->keyboard_keep_awake_last_time_ = now;
    }
  }
}

void HIDComposite::dump_config() {
  ESP_LOGCONFIG(TAG, "HID Composite (Mouse + Keyboard):");
  ESP_LOGCONFIG(TAG, "  Status: %s", this->initialized_ ? "Initialized" : "Not initialized");
}

// ============ Mouse Functions ============

void HIDComposite::send_mouse_report() {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) return;
  uint8_t report[5] = {this->mouse_buttons_, 0, 0, 0, 0};
  tud_hid_report(REPORT_ID_MOUSE, report, sizeof(report));
}

void HIDComposite::move(int8_t x, int8_t y) {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) return;
  uint8_t report[5] = {this->mouse_buttons_, (uint8_t)x, (uint8_t)y, 0, 0};
  tud_hid_report(REPORT_ID_MOUSE, report, sizeof(report));
  ESP_LOGD(TAG, "Mouse move: x=%d, y=%d", x, y);
}

void HIDComposite::scroll(int8_t vertical, int8_t horizontal) {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) return;
  uint8_t report[5] = {this->mouse_buttons_, 0, 0, (uint8_t)vertical, (uint8_t)horizontal};
  tud_hid_report(REPORT_ID_MOUSE, report, sizeof(report));
  ESP_LOGD(TAG, "Mouse scroll: v=%d, h=%d", vertical, horizontal);
}

void HIDComposite::click(MouseButton button) {
  this->mouse_press(button);
  delay(10);
  this->mouse_release(button);
}

void HIDComposite::mouse_press(MouseButton button) {
  this->mouse_buttons_ |= (1 << button);
  this->send_mouse_report();
  ESP_LOGD(TAG, "Mouse press: button=%d", button);
}

void HIDComposite::mouse_release(MouseButton button) {
  this->mouse_buttons_ &= ~(1 << button);
  this->send_mouse_report();
  ESP_LOGD(TAG, "Mouse release: button=%d", button);
}

void HIDComposite::mouse_release_all() {
  this->mouse_buttons_ = 0;
  this->send_mouse_report();
  ESP_LOGD(TAG, "Mouse release all");
}

// ============ Keyboard Functions ============

void HIDComposite::send_keyboard_report(uint8_t modifier, uint8_t keycode) {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) return;
  uint8_t report[8] = {modifier, 0, keycode, 0, 0, 0, 0, 0};
  tud_hid_report(REPORT_ID_KEYBOARD, report, sizeof(report));
  ESP_LOGD(TAG, "Keyboard report: mod=0x%02X key=0x%02X", modifier, keycode);
}

void HIDComposite::key_press(const std::string &key, uint8_t modifier) {
  uint8_t keycode, char_mod;
  if (key.length() == 1) {
    this->char_to_keycode(key[0], keycode, char_mod);
  } else {
    keycode = this->key_name_to_keycode(key);
    char_mod = 0;
  }
  ESP_LOGD(TAG, "Key press: %s", key.c_str());
  this->send_keyboard_report(modifier | char_mod, keycode);
}

void HIDComposite::key_release() { this->key_release_all(); }
void HIDComposite::key_release_all() {
  ESP_LOGD(TAG, "Key release all");
  this->send_keyboard_report(0, 0);
}

void HIDComposite::key_tap(const std::string &key, uint8_t modifier) {
  this->key_press(key, modifier);
  delay(10);
  this->key_release();
}

void HIDComposite::type(const std::string &text, uint32_t speed_ms, uint32_t jitter_ms) {
  ESP_LOGI(TAG, "Type: %s (speed=%dms, jitter=%dms)", text.c_str(), speed_ms, jitter_ms);
  for (char c : text) {
    uint8_t keycode, mod;
    this->char_to_keycode(c, keycode, mod);
    this->send_keyboard_report(mod, keycode);
    delay(10);
    this->send_keyboard_report(0, 0);
    
    // Calculate delay with jitter
    uint32_t delay_ms = speed_ms;
    if (jitter_ms > 0) {
      int32_t jitter = (rand() % (jitter_ms * 2 + 1)) - jitter_ms;
      delay_ms = (int32_t)speed_ms + jitter > 10 ? speed_ms + jitter : 10;
    }
    delay(delay_ms);
  }
}

void HIDComposite::char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier) {
  modifier = 0;
  if (c >= 'a' && c <= 'z') { keycode = KEY_A + (c - 'a'); return; }
  if (c >= 'A' && c <= 'Z') { keycode = KEY_A + (c - 'A'); modifier = MOD_LEFT_SHIFT; return; }
  if (c >= '1' && c <= '9') { keycode = KEY_1 + (c - '1'); return; }
  if (c == '0') { keycode = KEY_0; return; }
  switch (c) {
    case ' ': keycode = KEY_SPACE; break;
    case '\n': keycode = KEY_ENTER; break;
    case '\t': keycode = KEY_TAB; break;
    case '-': keycode = KEY_MINUS; break;
    case '=': keycode = KEY_EQUAL; break;
    case '[': keycode = KEY_LEFT_BRACE; break;
    case ']': keycode = KEY_RIGHT_BRACE; break;
    case '\\': keycode = KEY_BACKSLASH; break;
    case ';': keycode = KEY_SEMICOLON; break;
    case '\'': keycode = KEY_APOSTROPHE; break;
    case '`': keycode = KEY_GRAVE; break;
    case ',': keycode = KEY_COMMA; break;
    case '.': keycode = KEY_PERIOD; break;
    case '/': keycode = KEY_SLASH; break;
    case '!': keycode = KEY_1; modifier = MOD_LEFT_SHIFT; break;
    case '@': keycode = KEY_2; modifier = MOD_LEFT_SHIFT; break;
    case '#': keycode = KEY_3; modifier = MOD_LEFT_SHIFT; break;
    case '$': keycode = KEY_4; modifier = MOD_LEFT_SHIFT; break;
    case '%': keycode = KEY_5; modifier = MOD_LEFT_SHIFT; break;
    case '^': keycode = KEY_6; modifier = MOD_LEFT_SHIFT; break;
    case '&': keycode = KEY_7; modifier = MOD_LEFT_SHIFT; break;
    case '*': keycode = KEY_8; modifier = MOD_LEFT_SHIFT; break;
    case '(': keycode = KEY_9; modifier = MOD_LEFT_SHIFT; break;
    case ')': keycode = KEY_0; modifier = MOD_LEFT_SHIFT; break;
    case '_': keycode = KEY_MINUS; modifier = MOD_LEFT_SHIFT; break;
    case '+': keycode = KEY_EQUAL; modifier = MOD_LEFT_SHIFT; break;
    case '{': keycode = KEY_LEFT_BRACE; modifier = MOD_LEFT_SHIFT; break;
    case '}': keycode = KEY_RIGHT_BRACE; modifier = MOD_LEFT_SHIFT; break;
    case '|': keycode = KEY_BACKSLASH; modifier = MOD_LEFT_SHIFT; break;
    case ':': keycode = KEY_SEMICOLON; modifier = MOD_LEFT_SHIFT; break;
    case '"': keycode = KEY_APOSTROPHE; modifier = MOD_LEFT_SHIFT; break;
    case '~': keycode = KEY_GRAVE; modifier = MOD_LEFT_SHIFT; break;
    case '<': keycode = KEY_COMMA; modifier = MOD_LEFT_SHIFT; break;
    case '>': keycode = KEY_PERIOD; modifier = MOD_LEFT_SHIFT; break;
    case '?': keycode = KEY_SLASH; modifier = MOD_LEFT_SHIFT; break;
    default: keycode = KEY_NONE; break;
  }
}

uint8_t HIDComposite::key_name_to_keycode(const std::string &key) {
  std::string k = key;
  for (char &c : k) if (c >= 'a' && c <= 'z') c -= 32;
  if (k == "ENTER" || k == "RETURN") return KEY_ENTER;
  if (k == "ESC" || k == "ESCAPE") return KEY_ESC;
  if (k == "BACKSPACE") return KEY_BACKSPACE;
  if (k == "TAB") return KEY_TAB;
  if (k == "SPACE") return KEY_SPACE;
  if (k == "DELETE") return KEY_DELETE;
  if (k == "INSERT") return KEY_INSERT;
  if (k == "HOME") return KEY_HOME;
  if (k == "END") return KEY_END;
  if (k == "PAGEUP") return KEY_PAGE_UP;
  if (k == "PAGEDOWN") return KEY_PAGE_DOWN;
  if (k == "UP") return KEY_UP_ARROW;
  if (k == "DOWN") return KEY_DOWN_ARROW;
  if (k == "LEFT") return KEY_LEFT_ARROW;
  if (k == "RIGHT") return KEY_RIGHT_ARROW;
  if (k == "F1") return KEY_F1; if (k == "F2") return KEY_F2; if (k == "F3") return KEY_F3;
  if (k == "F4") return KEY_F4; if (k == "F5") return KEY_F5; if (k == "F6") return KEY_F6;
  if (k == "F7") return KEY_F7; if (k == "F8") return KEY_F8; if (k == "F9") return KEY_F9;
  if (k == "F10") return KEY_F10; if (k == "F11") return KEY_F11; if (k == "F12") return KEY_F12;
  ESP_LOGW(TAG, "Unknown key: %s", key.c_str());
  return KEY_NONE;
}

void HIDComposite::start_mouse_keep_awake(uint32_t interval_ms, uint32_t jitter_ms) {
  ESP_LOGI(TAG, "Starting mouse keep awake: interval=%dms, jitter=%dms", interval_ms, jitter_ms);
  this->mouse_keep_awake_interval_ = interval_ms;
  this->mouse_keep_awake_jitter_ = jitter_ms;
  this->mouse_keep_awake_last_time_ = millis();
  this->mouse_keep_awake_next_interval_ = interval_ms;
  this->mouse_keep_awake_enabled_ = true;
}

void HIDComposite::stop_mouse_keep_awake() {
  ESP_LOGI(TAG, "Stopping mouse keep awake");
  this->mouse_keep_awake_enabled_ = false;
}

void HIDComposite::start_keyboard_keep_awake(const std::string &key, uint32_t interval_ms, uint32_t jitter_ms) {
  ESP_LOGI(TAG, "Starting keyboard keep awake: key=%s, interval=%dms, jitter=%dms", key.c_str(), interval_ms, jitter_ms);
  this->keyboard_keep_awake_key_ = key;
  this->keyboard_keep_awake_interval_ = interval_ms;
  this->keyboard_keep_awake_jitter_ = jitter_ms;
  this->keyboard_keep_awake_last_time_ = millis();
  this->keyboard_keep_awake_next_interval_ = interval_ms;
  this->keyboard_keep_awake_enabled_ = true;
}

void HIDComposite::stop_keyboard_keep_awake() {
  ESP_LOGI(TAG, "Stopping keyboard keep awake");
  this->keyboard_keep_awake_enabled_ = false;
}

bool HIDComposite::is_connected() {
  if (!this->initialized_) return false;
  return tud_mounted();
}

bool HIDComposite::is_ready() {
  if (!this->initialized_) return false;
  return tud_mounted() && tud_hid_ready();
}

// ============ Telephony Functions ============

void HIDComposite::mute() {
  ESP_LOGD(TAG, "Sending mute");
  this->mute_button_ = true;
  this->send_telephony_report();
  delay(50);
  this->mute_button_ = false;
  this->send_telephony_report();
}

void HIDComposite::unmute() {
  ESP_LOGD(TAG, "Sending unmute");
  this->mute_button_ = true;
  this->send_telephony_report();
  delay(50);
  this->mute_button_ = false;
  this->send_telephony_report();
}

void HIDComposite::toggle_mute() {
  ESP_LOGD(TAG, "Toggling mute");
  this->mute_button_ = true;
  this->send_telephony_report();
  delay(50);
  this->mute_button_ = false;
  this->send_telephony_report();
}

void HIDComposite::hook_switch(bool state) {
  ESP_LOGD(TAG, "Hook switch: %s", state ? "ON" : "OFF");
  this->hook_button_ = state;
  this->send_telephony_report();
}

void HIDComposite::answer_call() {
  ESP_LOGI(TAG, "Answering call");
  this->hook_button_ = true;
  this->send_telephony_report();
}

void HIDComposite::hang_up() {
  ESP_LOGI(TAG, "Hanging up");
  this->hook_button_ = false;
  this->send_telephony_report();
}

void HIDComposite::send_telephony_report() {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) return;
  
  // Report: bit 0 = Hook Switch, bit 1 = Phone Mute
  uint8_t report = 0;
  if (this->hook_button_) report |= 0x01;
  if (this->mute_button_) report |= 0x02;
  
  tud_hid_report(REPORT_ID_TELEPHONY, &report, sizeof(report));
  
  ESP_LOGD(TAG, "Sent telephony report: hook=%d, mute=%d", this->hook_button_, this->mute_button_);
}

void HIDComposite::process_host_report(uint8_t const *buffer, uint16_t bufsize) {
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
  
  if (new_hold != this->hold_) {
    this->hold_ = new_hold;
    ESP_LOGI(TAG, "Hold state changed: %s", new_hold ? "ON HOLD" : "NOT ON HOLD");
    this->hold_callbacks_.call(new_hold);
  }
}

}  // namespace hid_composite
}  // namespace esphome

#else

namespace esphome {
namespace hid_composite {
static const char *const TAG = "hid_composite";
void HIDComposite::setup() { ESP_LOGE(TAG, "Only supported on ESP32-S3/S2"); }
void HIDComposite::loop() {}
void HIDComposite::dump_config() {}
void HIDComposite::move(int8_t x, int8_t y) {}
void HIDComposite::scroll(int8_t vertical, int8_t horizontal) {}
void HIDComposite::click(MouseButton button) {}
void HIDComposite::mouse_press(MouseButton button) {}
void HIDComposite::mouse_release(MouseButton button) {}
void HIDComposite::mouse_release_all() {}
void HIDComposite::key_press(const std::string &key, uint8_t modifier) {}
void HIDComposite::key_release() {}
void HIDComposite::key_release_all() {}
void HIDComposite::key_tap(const std::string &key, uint8_t modifier) {}
void HIDComposite::type(const std::string &text, uint32_t speed_ms, uint32_t jitter_ms) {}
void HIDComposite::char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier) {}
uint8_t HIDComposite::key_name_to_keycode(const std::string &key) { return 0; }
void HIDComposite::send_mouse_report() {}
void HIDComposite::send_keyboard_report(uint8_t modifier, uint8_t keycode) {}
void HIDComposite::start_mouse_keep_awake(uint32_t interval_ms, uint32_t jitter_ms) {}
void HIDComposite::stop_mouse_keep_awake() {}
void HIDComposite::start_keyboard_keep_awake(const std::string &key, uint32_t interval_ms, uint32_t jitter_ms) {}
void HIDComposite::stop_keyboard_keep_awake() {}
bool HIDComposite::is_connected() { return false; }
bool HIDComposite::is_ready() { return false; }
void HIDComposite::mute() {}
void HIDComposite::unmute() {}
void HIDComposite::toggle_mute() {}
void HIDComposite::hook_switch(bool state) {}
void HIDComposite::answer_call() {}
void HIDComposite::hang_up() {}
void HIDComposite::send_telephony_report() {}
void HIDComposite::process_host_report(uint8_t const *buffer, uint16_t bufsize) {}
}  // namespace hid_composite
}  // namespace esphome

#endif
#endif
