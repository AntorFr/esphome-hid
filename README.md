# ESPHome HID Mouse Component

An ESPHome external component to simulate a USB HID mouse on ESP32-S3.

## Features

- Full USB HID mouse simulation
- Relative movements (X, Y)
- Left, right, and middle buttons
- Scroll wheel
- Compatible with ESP32-S3 native USB OTG

## Installation

Add the external component to your ESPHome configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/AntorFR/esphome-hid
      ref: main
    components: [hid_mouse]
```

## Configuration

```yaml
esphome:
  name: esp32-hid-mouse
  platformio_options:
    board_build.flash_mode: dio

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf
    version: recommended

hid_mouse:
  id: my_mouse

# Example usage with a button
binary_sensor:
  - platform: gpio
    pin: GPIO0
    name: "Click Button"
    on_press:
      - hid_mouse.click:
          id: my_mouse
          button: LEFT
```

## Available Actions

### `hid_mouse.move`
Moves the cursor relatively.

```yaml
- hid_mouse.move:
    id: my_mouse
    x: 10
    y: -5
```

### `hid_mouse.click`
Performs a click with the specified button.

```yaml
- hid_mouse.click:
    id: my_mouse
    button: LEFT  # LEFT, RIGHT, MIDDLE
```

### `hid_mouse.press`
Presses a button (without releasing).

```yaml
- hid_mouse.press:
    id: my_mouse
    button: LEFT
```

### `hid_mouse.release`
Releases a button.

```yaml
- hid_mouse.release:
    id: my_mouse
    button: LEFT
```

### `hid_mouse.scroll`
Scrolls the wheel.

```yaml
- hid_mouse.scroll:
    id: my_mouse
    amount: 1  # positive = up, negative = down
```

## Requirements

- ESP32-S3 with native USB OTG support
- ESPHome 2023.12.0 or newer
- ESP-IDF framework

## License

MIT License - see [LICENSE](LICENSE)
