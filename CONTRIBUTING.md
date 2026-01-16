# Contributing to esphome-hid

Thank you for your interest in contributing to esphome-hid!

## Development Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/AntorFR/esphome-hid.git
   cd esphome-hid
   ```

2. Create a virtual environment:
   ```bash
   python3.12 -m venv .venv
   source .venv/bin/activate
   ```

3. Install ESPHome:
   ```bash
   pip install esphome
   ```

4. Test compilation:
   ```bash
   esphome compile examples/basic.yaml
   ```

## Project Structure

```
components/hid_mouse/
├── __init__.py      # ESPHome component configuration (Python)
├── hid_mouse.h      # C++ header with class definitions
└── hid_mouse.cpp    # C++ implementation
```

## Adding New Features

### Adding a New Action

1. **Define the action class** in `hid_mouse.h`:
   ```cpp
   template<typename... Ts> 
   class NewAction : public Action<Ts...>, public Parented<HIDMouse> {
    public:
     TEMPLATABLE_VALUE(int, parameter)
     void play(Ts... x) override {
       this->parent_->new_method(this->parameter_.value(x...));
     }
   };
   ```

2. **Implement the method** in `hid_mouse.cpp`:
   ```cpp
   void HIDMouse::new_method(int parameter) {
     // Implementation
   }
   ```

3. **Register the action** in `__init__.py`:
   ```python
   NEW_ACTION_SCHEMA = cv.Schema({
       cv.GenerateID(): cv.use_id(HIDMouse),
       cv.Required("parameter"): cv.templatable(cv.int_),
   })

   @automation.register_action("hid_mouse.new_action", NewAction, NEW_ACTION_SCHEMA)
   async def new_action_to_code(config, action_id, template_arg, args):
       var = cg.new_Pvariable(action_id, template_arg)
       await cg.register_parented(var, config[CONF_ID])
       template_ = await cg.templatable(config["parameter"], args, cg.int_)
       cg.add(var.set_parameter(template_))
       return var
   ```

### Adding a New HID Device Type

To add a new HID device (keyboard, gamepad, etc.):

1. Create a new component folder: `components/hid_keyboard/`
2. Create the three required files:
   - `__init__.py` - Component registration
   - `hid_keyboard.h` - Class definitions
   - `hid_keyboard.cpp` - Implementation with HID report descriptor
3. Define the appropriate HID report descriptor for your device type
4. Implement the TinyUSB callbacks

## Code Style Guidelines

### Python (`__init__.py`)
- Use `async def` for `to_code` functions
- Follow ESPHome naming conventions
- Use `cv.` validators for schema definitions

### C++ (`.h` and `.cpp`)
- Use `esphome::component_name` namespace pattern
- Use ESP-IDF logging macros (`ESP_LOGI`, `ESP_LOGD`, etc.)
- Follow ESPHome coding style (2-space indentation)

### YAML Examples
- Use 2-space indentation
- Include comments explaining configuration options
- Provide complete, working examples

## Testing

Before submitting a PR:

1. Ensure the code compiles:
   ```bash
   esphome compile examples/basic.yaml
   ```

2. Test on actual hardware if possible

3. Update documentation if adding new features

## Submitting Changes

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/new-feature`
3. Make your changes
4. Test compilation
5. Commit with descriptive messages
6. Push and create a Pull Request

## Questions?

Open an issue on GitHub for any questions or discussions.
