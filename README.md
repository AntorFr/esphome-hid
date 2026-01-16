# ESPHome HID Components

ESPHome external components to simulate USB HID devices (mouse, keyboard, telephony) on ESP32-S3.

## Components

| Component | Description |
|-----------|-------------|
| `hid_mouse` | USB HID Mouse only |
| `hid_keyboard` | USB HID Keyboard only |
| `hid_composite` | USB HID Mouse + Keyboard + Telephony combined |
| `hid_telephony` | USB HID Telephony only (mute/call control) |

> **Note**: These components are mutually exclusive. Use only ONE of them in your configuration.

## Features

### Mouse (`hid_mouse` or `hid_composite`)
- Relative cursor movements (X, Y)
- Left, right, and middle buttons
- Vertical and horizontal scroll wheel
- Keep awake (prevents PC sleep)

### Keyboard (`hid_keyboard` or `hid_composite`)
- Key press/release/tap
- All standard keys (A-Z, 0-9, F1-F12, arrows, etc.)
- Modifier keys (Ctrl, Shift, Alt, GUI/Win/Cmd)
- Type entire text strings with realistic speed/jitter
- Keyboard shortcuts (Ctrl+C, Alt+Tab, etc.)
- Keep awake (prevents PC sleep)

### Telephony (`hid_telephony` or `hid_composite`)
- Mute/unmute control
- Answer/hang up calls
- **Bidirectional sync**: PC mute state is reflected in Home Assistant
- Binary sensors for call state (in_call, ringing)
- Switch component for mute control

### Connection Status
All components provide:
- `is_connected()` - PC connected via USB
- Binary sensor for connection status

## Requirements

- **ESP32-S3** with native USB OTG port
- **ESP-IDF framework** (not Arduino)
- Board with **two USB ports** recommended:
  - **UART/CH340 port**: for flashing and serial logs
  - **USB native/JTAG port**: for HID devices

> **Important**: The HID device will only work on the **USB native/JTAG port**, not the UART/CH340 port.

## Installation

```yaml
# For mouse only
external_components:
  - source:
      type: git
      url: https://github.com/AntorFr/esphome-hid
      ref: main
    components: [hid_mouse]

# For keyboard only
external_components:
  - source:
      type: git
      url: https://github.com/AntorFr/esphome-hid
      ref: main
    components: [hid_keyboard]

# For both mouse and keyboard
external_components:
  - source:
      type: git
      url: https://github.com/AntorFr/esphome-hid
      ref: main
    components: [hid_composite]
```

## Configuration

### Required ESP-IDF Settings

```yaml
esphome:
  name: esp32-hid
  platformio_options:
    build_flags:
      - -DCFG_TUD_HID=1
    lib_deps:
      - "espressif/esp_tinyusb^2.0.1"

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf
    sdkconfig_options:
      CONFIG_USJ_ENABLE_USB_SERIAL_JTAG: "n"
      CONFIG_TINYUSB_HID_COUNT: "1"
      CONFIG_ESP_CONSOLE_UART_DEFAULT: "y"
```

## Mouse Actions

```yaml
hid_mouse:
  id: my_mouse
```

| Action | Description |
|--------|-------------|
| `hid_mouse.move` | Move cursor (x, y: -127 to 127) |
| `hid_mouse.click` | Click button (LEFT, RIGHT, MIDDLE) |
| `hid_mouse.press` | Press button |
| `hid_mouse.release` | Release button |
| `hid_mouse.release_all` | Release all buttons |
| `hid_mouse.scroll` | Scroll (vertical, horizontal) |

## Keyboard Actions

```yaml
hid_keyboard:
  id: my_keyboard
```

| Action | Description |
|--------|-------------|
| `hid_keyboard.press` | Press key |
| `hid_keyboard.release` | Release all keys |
| `hid_keyboard.tap` | Press and release key |
| `hid_keyboard.type` | Type text string |

### Special Keys
ENTER, ESC, BACKSPACE, TAB, SPACE, DELETE, INSERT, HOME, END, PAGEUP, PAGEDOWN, UP, DOWN, LEFT, RIGHT, F1-F12

### Modifiers
NONE, CTRL, SHIFT, ALT, GUI (WIN/CMD), CTRL_SHIFT, CTRL_ALT, CTRL_GUI, etc.

### Examples

```yaml
# Type text
- hid_keyboard.type:
    text: "Hello World!"

# Ctrl+C
- hid_keyboard.tap:
    key: "c"
    modifiers: CTRL

# Alt+Tab
- hid_keyboard.tap:
    key: "TAB"
    modifiers: ALT
```

## Composite Actions

```yaml
hid_composite:
  id: my_hid
```

Mouse: `hid_composite.move`, `hid_composite.click`, `hid_composite.mouse_press`, `hid_composite.mouse_release`, `hid_composite.scroll`

Keyboard: `hid_composite.key_press`, `hid_composite.key_tap`, `hid_composite.key_release`, `hid_composite.type`

Telephony: `hid_composite.mute`, `hid_composite.unmute`, `hid_composite.toggle_mute`, `hid_composite.answer_call`, `hid_composite.hang_up`

Keep Awake: `hid_composite.start_mouse_keep_awake`, `hid_composite.stop_mouse_keep_awake`, `hid_composite.start_keyboard_keep_awake`, `hid_composite.stop_keyboard_keep_awake`

## Telephony Actions

```yaml
hid_telephony:
  id: my_telephony
```

| Action | Description |
|--------|-------------|
| `hid_telephony.mute` | Send mute button press |
| `hid_telephony.unmute` | Send unmute button press |
| `hid_telephony.toggle_mute` | Toggle mute state |
| `hid_telephony.answer` | Answer incoming call |
| `hid_telephony.hang_up` | End current call |

## Binary Sensors

### Connection Status (all components)
```yaml
binary_sensor:
  - platform: hid_composite  # or hid_mouse, hid_keyboard, hid_telephony
    type: connected
    name: "PC Connected"
```

### Telephony Status (hid_composite or hid_telephony)
```yaml
binary_sensor:
  - platform: hid_composite
    type: in_call
    name: "In Call"
  
  - platform: hid_composite
    type: ringing
    name: "Ringing"
```

## Switches

### Keep Awake (hid_composite, hid_mouse, hid_keyboard)
```yaml
switch:
  - platform: hid_composite
    type: mouse
    name: "Mouse Keep Awake"
    interval: 60s
    jitter: 10s
  
  - platform: hid_composite
    type: keyboard
    name: "Keyboard Keep Awake"
    key: "F15"
    interval: 60s
```

### Mute Control (hid_composite or hid_telephony)
```yaml
switch:
  - platform: hid_composite
    type: mute
    name: "Mute"
```

> **Note**: The mute switch syncs bidirectionally with the PC. If mute is toggled via Teams/Zoom, the switch updates automatically.

## Examples

See the [examples](examples/) folder:
- [basic.yaml](examples/basic.yaml) - Mouse
- [keyboard.yaml](examples/keyboard.yaml) - Keyboard
- [composite.yaml](examples/composite.yaml) - Mouse + Keyboard
- [test_composite_switch.yaml](examples/test_composite_switch.yaml) - Composite with switches
- [test_telephony.yaml](examples/test_telephony.yaml) - Telephony controls

## License

MIT License
