# CLAUDE.md - AI Assistant Guide for esphome-hid

This file provides context for AI assistants (Claude, Copilot, etc.) working with this repository.

## Project Overview

**esphome-hid** is an ESPHome external component that enables ESP32-S3 (and S2/P4) microcontrollers to act as USB HID devices, specifically a mouse. It uses TinyUSB for USB communication.

## Repository Structure

```
esphome-hid/
├── components/
│   └── hid_mouse/
│       ├── __init__.py      # ESPHome Python component configuration
│       ├── hid_mouse.h      # C++ header with class definitions
│       └── hid_mouse.cpp    # C++ implementation with TinyUSB integration
├── examples/
│   └── basic.yaml           # Example ESPHome configuration
├── README.md                # User documentation
├── LICENSE                  # MIT License
└── CLAUDE.md               # This file
```

## Technical Stack

- **ESPHome**: Home automation firmware framework
- **ESP-IDF**: Espressif IoT Development Framework (required for USB support)
- **TinyUSB**: USB stack library (via ESPHome's built-in `tinyusb` component)
- **Target Hardware**: ESP32-S2, ESP32-S3, ESP32-P4 (chips with native USB OTG)

## Key Files Explained

### `components/hid_mouse/__init__.py`
ESPHome Python configuration that:
- Declares the component with `CODEOWNERS` and `DEPENDENCIES`
- Defines the CONFIG_SCHEMA for YAML validation
- Registers automation actions (move, click, press, release, scroll)
- Sets ESP-IDF sdkconfig options for TinyUSB HID

### `components/hid_mouse/hid_mouse.h`
C++ header containing:
- `MouseButton` enum (LEFT=0x01, RIGHT=0x02, MIDDLE=0x04)
- `HIDMouse` class extending `esphome::Component`
- Template action classes (MoveAction, ClickAction, etc.)
- Conditional compilation with `HID_MOUSE_SUPPORTED` macro

### `components/hid_mouse/hid_mouse.cpp`
C++ implementation with:
- HID Report Descriptor (boot protocol compatible mouse)
- TinyUSB callbacks (`tud_hid_descriptor_report_cb`, etc.)
- Mouse action methods (move, click, press, release, scroll)
- Report sending via `tud_hid_report()`

## ESPHome Component Pattern

This follows ESPHome's external component pattern:
1. Python `__init__.py` handles YAML config parsing and code generation
2. C++ `.h/.cpp` files implement the actual component logic
3. Actions are registered in Python and implemented as template classes in C++

## Common Development Tasks

### Adding a new action
1. Add the action class in `hid_mouse.h` (template class extending `Action` and `Parented`)
2. Implement the `play()` method
3. Register the action in `__init__.py` with `@automation.register_action`
4. Define the schema and `to_code` async function

### Adding a new HID device type (e.g., keyboard)
1. Create a new component folder: `components/hid_keyboard/`
2. Follow the same pattern: `__init__.py`, `.h`, `.cpp`
3. Define appropriate HID report descriptor for the device type
4. Implement TinyUSB callbacks

### Testing compilation
```bash
# Activate virtual environment
source .venv/bin/activate

# Compile example
esphome compile examples/basic.yaml
```

## HID Report Structure

The mouse uses a 4-byte boot protocol report:
```cpp
typedef struct {
  uint8_t buttons;  // Bit 0: Left, Bit 1: Right, Bit 2: Middle
  int8_t x;         // Relative X movement (-127 to 127)
  int8_t y;         // Relative Y movement (-127 to 127)
  int8_t wheel;     // Scroll wheel (-127 to 127)
} hid_mouse_report_t;
```

## Dependencies

- ESPHome `esp32` component
- ESPHome `tinyusb` component (built-in, handles USB initialization)

## YAML Usage Example

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/AntorFR/esphome-hid
      ref: main
    components: [hid_mouse]

tinyusb:
  usb_product_str: "My HID Mouse"

hid_mouse:
  id: my_mouse

# Use in automations
binary_sensor:
  - platform: gpio
    pin: GPIO0
    on_press:
      - hid_mouse.click:
          id: my_mouse
          button: LEFT
```

## Gotchas and Notes

1. **USB OTG Required**: Only ESP32-S2, S3, and P4 have native USB OTG
2. **ESP-IDF Framework**: Must use `esp-idf` framework, not Arduino
3. **TinyUSB Dependency**: The `tinyusb` component must be included in the config
4. **Boot Protocol**: Uses boot protocol for maximum compatibility
5. **Report ID**: Report ID is 0 (boot protocol, no report ID byte)
