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

## Requirements

- **ESP32-S3** with native USB OTG port
- **ESP-IDF framework** (not Arduino)
- Board with **two USB ports** recommended:
  - **UART/CH340 port**: for flashing and serial logs
  - **USB native/JTAG port**: for HID mouse (this is where your mouse will appear)

> ⚠️ **Important**: On boards with two USB ports, the HID mouse will only work on the **USB native/JTAG port**, not the UART/CH340 port. Connect this port to the computer where you want to control the mouse.

## Configuration

```yaml
esphome:
  name: esp32-hid-mouse

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf
    version: recommended
    sdkconfig_options:
      # Disable USB-Serial/JTAG to free USB port for HID (REQUIRED)
      CONFIG_USJ_ENABLE_USB_SERIAL_JTAG: "n"
      # Enable TinyUSB HID (REQUIRED)
      CONFIG_TINYUSB_HID_COUNT: "1"
      # Console on UART0 for debugging (recommended)
      CONFIG_ESP_CONSOLE_UART_DEFAULT: "y"
    components:
      - espressif/esp_tinyusb^2.0.1

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
