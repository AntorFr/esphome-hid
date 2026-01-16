# GitHub Copilot Instructions for esphome-hid

## Project Context

This is an ESPHome external component for USB HID mouse simulation on ESP32-S3.

## Code Style

- **Python**: Follow ESPHome conventions, use `async def` for `to_code` functions
- **C++**: Use ESPHome namespaces (`esphome::hid_mouse`), ESP-IDF logging macros
- **YAML**: Follow ESPHome YAML style with 2-space indentation

## Key Patterns

### Adding ESPHome Actions
```python
# In __init__.py
ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDMouse),
    cv.Required("param"): cv.templatable(cv.int_),
})

@automation.register_action("hid_mouse.action", ActionClass, ACTION_SCHEMA)
async def action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config["param"], args, cg.int_)
    cg.add(var.set_param(template_))
    return var
```

### C++ Action Template
```cpp
template<typename... Ts> 
class NewAction : public Action<Ts...>, public Parented<HIDMouse> {
 public:
  TEMPLATABLE_VALUE(int, param)
  void play(Ts... x) override {
    this->parent_->method(this->param_.value(x...));
  }
};
```

## Important Files

- `components/hid_mouse/__init__.py` - Component registration and actions
- `components/hid_mouse/hid_mouse.h` - Class definitions
- `components/hid_mouse/hid_mouse.cpp` - TinyUSB integration

## Testing

```bash
source .venv/bin/activate
esphome compile examples/basic.yaml
```
