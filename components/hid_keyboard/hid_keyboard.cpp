#include "hid_keyboard.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

#ifdef HID_KEYBOARD_SUPPORTED

#include "tinyusb.h"
#include "tusb.h"
#include "class/hid/hid_device.h"

namespace esphome {
namespace hid_keyboard {

static const char *const TAG = "hid_keyboard";

// HID Keyboard Key Codes
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

// HID Report Descriptor for Keyboard
static const uint8_t hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    
    // Modifier keys
    0x05, 0x07,        //   Usage Page (Keyboard)
    0x19, 0xE0,        //   Usage Minimum (Left Control)
    0x29, 0xE7,        //   Usage Maximum (Right GUI)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Reserved byte
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant)
    
    // LEDs
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x05,        //   Usage Maximum (Kana)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Constant)
    
    // Key codes
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Keyboard)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data, Array)
    
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
    .idProduct = 0x4003,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

static const char *string_descriptors[] = {
    (const char[]){0x09, 0x04},
    "ESPHome",
    "HID Keyboard",
    "123456",
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

static const uint8_t configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(hid_report_descriptor), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10),
};

extern "C" {
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { return hid_report_descriptor; }
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) { return 0; }
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}
}

void HIDKeyboard::setup() {
  ESP_LOGI(TAG, "Setting up HID Keyboard...");
  
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

void HIDKeyboard::loop() {}

void HIDKeyboard::dump_config() {
  ESP_LOGCONFIG(TAG, "HID Keyboard:");
  ESP_LOGCONFIG(TAG, "  Status: %s", this->initialized_ ? "Initialized" : "Not initialized");
}

void HIDKeyboard::send_report(uint8_t modifier, uint8_t keycode) {
  if (!this->initialized_ || !tud_mounted() || !tud_hid_ready()) return;
  uint8_t keycodes[6] = {keycode, 0, 0, 0, 0, 0};
  tud_hid_keyboard_report(0, modifier, keycodes);
  ESP_LOGD(TAG, "Report: mod=0x%02X key=0x%02X", modifier, keycode);
}

void HIDKeyboard::press(const std::string &key, uint8_t modifier) {
  uint8_t keycode, char_mod;
  if (key.length() == 1) {
    this->char_to_keycode(key[0], keycode, char_mod);
  } else {
    keycode = this->key_name_to_keycode(key);
    char_mod = 0;
  }
  ESP_LOGD(TAG, "Press: %s", key.c_str());
  this->send_report(modifier | char_mod, keycode);
}

void HIDKeyboard::release() { this->release_all(); }
void HIDKeyboard::release_all() {
  ESP_LOGD(TAG, "Release all");
  this->send_report(0, 0);
}

void HIDKeyboard::tap(const std::string &key, uint8_t modifier) {
  this->press(key, modifier);
  delay(10);
  this->release();
}

void HIDKeyboard::type(const std::string &text, uint32_t speed_ms, uint32_t jitter_ms) {
  ESP_LOGI(TAG, "Type: %s (speed=%dms, jitter=%dms)", text.c_str(), speed_ms, jitter_ms);
  for (char c : text) {
    uint8_t keycode, mod;
    this->char_to_keycode(c, keycode, mod);
    this->send_report(mod, keycode);
    delay(10);
    this->send_report(0, 0);
    
    // Calculate delay with jitter
    uint32_t delay_ms = speed_ms;
    if (jitter_ms > 0) {
      int32_t jitter = (rand() % (jitter_ms * 2 + 1)) - jitter_ms;
      delay_ms = (int32_t)speed_ms + jitter > 10 ? speed_ms + jitter : 10;
    }
    delay(delay_ms);
  }
}

// QWERTY US layout mapping
void HIDKeyboard::char_to_keycode_qwerty(char c, uint8_t &keycode, uint8_t &modifier) {
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

// AZERTY FR layout mapping
// Maps characters to the scancodes that will produce them on an AZERTY keyboard
void HIDKeyboard::char_to_keycode_azerty(char c, uint8_t &keycode, uint8_t &modifier) {
  modifier = 0;
  
  // AZERTY has swapped letter positions: A<->Q, Z<->W, M position differs
  // The scancodes are based on physical positions, so we need to map accordingly
  
  // Letters - AZERTY swaps: A<->Q, Z<->W
  if (c == 'a') { keycode = KEY_Q; return; }  // 'a' is at Q position on AZERTY
  if (c == 'A') { keycode = KEY_Q; modifier = MOD_LEFT_SHIFT; return; }
  if (c == 'q') { keycode = KEY_A; return; }  // 'q' is at A position on AZERTY
  if (c == 'Q') { keycode = KEY_A; modifier = MOD_LEFT_SHIFT; return; }
  if (c == 'z') { keycode = KEY_W; return; }  // 'z' is at W position on AZERTY
  if (c == 'Z') { keycode = KEY_W; modifier = MOD_LEFT_SHIFT; return; }
  if (c == 'w') { keycode = KEY_Z; return; }  // 'w' is at Z position on AZERTY
  if (c == 'W') { keycode = KEY_Z; modifier = MOD_LEFT_SHIFT; return; }
  if (c == 'm') { keycode = KEY_SEMICOLON; return; }  // 'm' is at ; position on AZERTY
  if (c == 'M') { keycode = KEY_SEMICOLON; modifier = MOD_LEFT_SHIFT; return; }
  
  // Other letters (same position)
  if (c >= 'a' && c <= 'z') { keycode = KEY_A + (c - 'a'); return; }
  if (c >= 'A' && c <= 'Z') { keycode = KEY_A + (c - 'A'); modifier = MOD_LEFT_SHIFT; return; }
  
  // Numbers on AZERTY require Shift (top row produces symbols by default)
  if (c >= '1' && c <= '9') { keycode = KEY_1 + (c - '1'); modifier = MOD_LEFT_SHIFT; return; }
  if (c == '0') { keycode = KEY_0; modifier = MOD_LEFT_SHIFT; return; }
  
  switch (c) {
    case ' ': keycode = KEY_SPACE; break;
    case '\n': keycode = KEY_ENTER; break;
    case '\t': keycode = KEY_TAB; break;
    // AZERTY specific: top row without shift produces: &é"'(-è_çà)
    case '&': keycode = KEY_1; break;
    // é, ", ' etc. need proper handling but are non-ASCII
    case '-': keycode = KEY_6; break;  // - is at 6 position on AZERTY
    case '_': keycode = KEY_8; break;  // _ is at 8 position on AZERTY
    case '.': keycode = KEY_COMMA; modifier = MOD_LEFT_SHIFT; break;  // . is Shift+, on AZERTY
    case ',': keycode = KEY_M; break;  // , is at M position on AZERTY
    case ';': keycode = KEY_COMMA; break;  // ; is at , position on AZERTY
    case ':': keycode = KEY_PERIOD; break;  // : is at . position on AZERTY
    case '!': keycode = KEY_SLASH; break;  // ! is at / position on AZERTY
    case '?': keycode = KEY_M; modifier = MOD_LEFT_SHIFT; break;  // ? is Shift+M on AZERTY
    case '/': keycode = KEY_PERIOD; modifier = MOD_LEFT_SHIFT; break;  // / is Shift+. on AZERTY
    case '*': keycode = KEY_BACKSLASH; break;  // * position varies
    case '(': keycode = KEY_5; break;  // ( is at 5 position on AZERTY
    case ')': keycode = KEY_MINUS; break;  // ) is at - position on AZERTY
    case '=': keycode = KEY_EQUAL; break;
    case '+': keycode = KEY_EQUAL; modifier = MOD_LEFT_SHIFT; break;
    default: keycode = KEY_NONE; break;
  }
}

// QWERTZ DE layout mapping (German)
void HIDKeyboard::char_to_keycode_qwertz(char c, uint8_t &keycode, uint8_t &modifier) {
  modifier = 0;
  
  // QWERTZ swaps Y<->Z
  if (c == 'y') { keycode = KEY_Z; return; }
  if (c == 'Y') { keycode = KEY_Z; modifier = MOD_LEFT_SHIFT; return; }
  if (c == 'z') { keycode = KEY_Y; return; }
  if (c == 'Z') { keycode = KEY_Y; modifier = MOD_LEFT_SHIFT; return; }
  
  // Other letters (same position)
  if (c >= 'a' && c <= 'z') { keycode = KEY_A + (c - 'a'); return; }
  if (c >= 'A' && c <= 'Z') { keycode = KEY_A + (c - 'A'); modifier = MOD_LEFT_SHIFT; return; }
  if (c >= '1' && c <= '9') { keycode = KEY_1 + (c - '1'); return; }
  if (c == '0') { keycode = KEY_0; return; }
  
  switch (c) {
    case ' ': keycode = KEY_SPACE; break;
    case '\n': keycode = KEY_ENTER; break;
    case '\t': keycode = KEY_TAB; break;
    case '-': keycode = KEY_SLASH; break;  // - is at / position on QWERTZ
    case '_': keycode = KEY_SLASH; modifier = MOD_LEFT_SHIFT; break;
    case '.': keycode = KEY_PERIOD; break;
    case ',': keycode = KEY_COMMA; break;
    case ';': keycode = KEY_COMMA; modifier = MOD_LEFT_SHIFT; break;
    case ':': keycode = KEY_PERIOD; modifier = MOD_LEFT_SHIFT; break;
    case '?': keycode = KEY_MINUS; modifier = MOD_LEFT_SHIFT; break;
    case '!': keycode = KEY_1; modifier = MOD_LEFT_SHIFT; break;
    case '/': keycode = KEY_7; modifier = MOD_LEFT_SHIFT; break;
    case '(': keycode = KEY_8; modifier = MOD_LEFT_SHIFT; break;
    case ')': keycode = KEY_9; modifier = MOD_LEFT_SHIFT; break;
    case '=': keycode = KEY_0; modifier = MOD_LEFT_SHIFT; break;
    case '+': keycode = KEY_RIGHT_BRACE; break;
    case '*': keycode = KEY_RIGHT_BRACE; modifier = MOD_LEFT_SHIFT; break;
    default: keycode = KEY_NONE; break;
  }
}

// Main dispatcher based on layout
void HIDKeyboard::char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier) {
  switch (this->layout_) {
    case LAYOUT_AZERTY_FR:
      this->char_to_keycode_azerty(c, keycode, modifier);
      break;
    case LAYOUT_QWERTZ_DE:
      this->char_to_keycode_qwertz(c, keycode, modifier);
      break;
    case LAYOUT_QWERTY_US:
    default:
      this->char_to_keycode_qwerty(c, keycode, modifier);
      break;
  }
}

uint8_t HIDKeyboard::key_name_to_keycode(const std::string &key) {
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

void HIDKeyboard::start_keep_awake(const std::string &key, uint32_t interval_ms, uint32_t jitter_ms) {
  ESP_LOGI(TAG, "Starting keep awake: key=%s, interval=%dms, jitter=%dms", key.c_str(), interval_ms, jitter_ms);
  this->keep_awake_key_ = key;
  this->keep_awake_interval_ = interval_ms;
  this->keep_awake_jitter_ = jitter_ms;
  this->keep_awake_last_time_ = millis();
  this->keep_awake_next_interval_ = interval_ms;
  this->keep_awake_enabled_ = true;
}

void HIDKeyboard::stop_keep_awake() {
  ESP_LOGI(TAG, "Stopping keep awake");
  this->keep_awake_enabled_ = false;
}

bool HIDKeyboard::is_connected() {
  if (!this->initialized_) return false;
  return tud_mounted();
}

bool HIDKeyboard::is_ready() {
  if (!this->initialized_) return false;
  return tud_mounted() && tud_hid_ready();
}

}  // namespace hid_keyboard
}  // namespace esphome

#else

namespace esphome {
namespace hid_keyboard {
static const char *const TAG = "hid_keyboard";
void HIDKeyboard::setup() { ESP_LOGE(TAG, "Only supported on ESP32-S3/S2"); }
void HIDKeyboard::loop() {}
void HIDKeyboard::dump_config() {}
void HIDKeyboard::press(const std::string &key, uint8_t modifier) {}
void HIDKeyboard::release() {}
void HIDKeyboard::release_all() {}
void HIDKeyboard::tap(const std::string &key, uint8_t modifier) {}
void HIDKeyboard::type(const std::string &text, uint32_t speed_ms, uint32_t jitter_ms) {}
void HIDKeyboard::char_to_keycode(char c, uint8_t &keycode, uint8_t &modifier) {}
void HIDKeyboard::char_to_keycode_qwerty(char c, uint8_t &keycode, uint8_t &modifier) {}
void HIDKeyboard::char_to_keycode_azerty(char c, uint8_t &keycode, uint8_t &modifier) {}
void HIDKeyboard::char_to_keycode_qwertz(char c, uint8_t &keycode, uint8_t &modifier) {}
uint8_t HIDKeyboard::key_name_to_keycode(const std::string &key) { return 0; }
void HIDKeyboard::send_report(uint8_t modifier, uint8_t keycode) {}
void HIDKeyboard::start_keep_awake(const std::string &key, uint32_t interval_ms, uint32_t jitter_ms) {}
void HIDKeyboard::stop_keep_awake() {}
bool HIDKeyboard::is_connected() { return false; }
bool HIDKeyboard::is_ready() { return false; }
}  // namespace hid_keyboard
}  // namespace esphome

#endif
#endif
