import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@AntorFr"]

# Cannot be used with hid_mouse or hid_composite (each configures USB)
CONFLICTS_WITH = ["hid_mouse", "hid_composite"]

CONF_LAYOUT = "layout"

hid_keyboard_ns = cg.esphome_ns.namespace("hid_keyboard")
HIDKeyboard = hid_keyboard_ns.class_("HIDKeyboard", cg.Component)

# Keyboard layout enum
KeyboardLayout = hid_keyboard_ns.enum("KeyboardLayout")
KEYBOARD_LAYOUTS = {
    "QWERTY_US": KeyboardLayout.LAYOUT_QWERTY_US,
    "AZERTY_FR": KeyboardLayout.LAYOUT_AZERTY_FR,
    "QWERTZ_DE": KeyboardLayout.LAYOUT_QWERTZ_DE,
}

# Actions
PressAction = hid_keyboard_ns.class_("PressAction", automation.Action)
ReleaseAction = hid_keyboard_ns.class_("ReleaseAction", automation.Action)
TapAction = hid_keyboard_ns.class_("TapAction", automation.Action)
TypeAction = hid_keyboard_ns.class_("TypeAction", automation.Action)
ReleaseAllAction = hid_keyboard_ns.class_("ReleaseAllAction", automation.Action)
StartKeepAwakeAction = hid_keyboard_ns.class_("StartKeepAwakeAction", automation.Action)
StopKeepAwakeAction = hid_keyboard_ns.class_("StopKeepAwakeAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HIDKeyboard),
        cv.Optional(CONF_LAYOUT, default="QWERTY_US"): cv.enum(KEYBOARD_LAYOUTS, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_layout(config[CONF_LAYOUT]))


CONF_KEY = "key"
CONF_TEXT = "text"
CONF_MODIFIER = "modifier"
CONF_INTERVAL = "interval"

MODIFIERS = {
    "NONE": 0x00,
    "LEFT_CTRL": 0x01,
    "LEFT_SHIFT": 0x02,
    "LEFT_ALT": 0x04,
    "LEFT_GUI": 0x08,
    "RIGHT_CTRL": 0x10,
    "RIGHT_SHIFT": 0x20,
    "RIGHT_ALT": 0x40,
    "RIGHT_GUI": 0x80,
    "CTRL": 0x01,
    "SHIFT": 0x02,
    "ALT": 0x04,
    "GUI": 0x08,
    "WIN": 0x08,
    "CMD": 0x08,
}


@automation.register_action(
    "hid_keyboard.press",
    PressAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
            cv.Required(CONF_KEY): cv.templatable(cv.string),
            cv.Optional(CONF_MODIFIER, default="NONE"): cv.enum(MODIFIERS, upper=True),
        }
    ),
)
async def press_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_KEY], args, cg.std_string)
    cg.add(var.set_key(template_))
    cg.add(var.set_modifier(config[CONF_MODIFIER]))
    return var


@automation.register_action(
    "hid_keyboard.release",
    ReleaseAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
        }
    ),
)
async def release_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "hid_keyboard.tap",
    TapAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
            cv.Required(CONF_KEY): cv.templatable(cv.string),
            cv.Optional(CONF_MODIFIER, default="NONE"): cv.enum(MODIFIERS, upper=True),
        }
    ),
)
async def tap_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_KEY], args, cg.std_string)
    cg.add(var.set_key(template_))
    cg.add(var.set_modifier(config[CONF_MODIFIER]))
    return var


@automation.register_action(
    "hid_keyboard.release_all",
    ReleaseAllAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
        }
    ),
)
async def release_all_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


CONF_SPEED = "speed"
CONF_JITTER = "jitter"

@automation.register_action(
    "hid_keyboard.type",
    TypeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
            cv.Required(CONF_TEXT): cv.templatable(cv.string),
            cv.Optional(CONF_SPEED, default=50): cv.templatable(cv.positive_int),
            cv.Optional(CONF_JITTER, default=0): cv.templatable(cv.positive_int),
        }
    ),
)
async def type_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_TEXT], args, cg.std_string)
    cg.add(var.set_text(template_))
    speed = await cg.templatable(config[CONF_SPEED], args, cg.uint32)
    cg.add(var.set_speed(speed))
    jitter = await cg.templatable(config[CONF_JITTER], args, cg.uint32)
    cg.add(var.set_jitter(jitter))
    return var


# Action: Start Keep Awake
@automation.register_action(
    "hid_keyboard.start_keep_awake",
    StartKeepAwakeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
            cv.Required(CONF_KEY): cv.templatable(cv.string),
            cv.Optional(CONF_INTERVAL, default="60s"): cv.templatable(cv.positive_time_period_milliseconds),
            cv.Optional(CONF_JITTER, default="0s"): cv.templatable(cv.positive_time_period_milliseconds),
        }
    ),
)
async def start_keep_awake_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_KEY], args, cg.std_string)
    cg.add(var.set_key(template_))
    template_ = await cg.templatable(config[CONF_INTERVAL], args, cg.uint32)
    cg.add(var.set_interval(template_))
    template_ = await cg.templatable(config[CONF_JITTER], args, cg.uint32)
    cg.add(var.set_jitter(template_))
    return var


# Action: Stop Keep Awake
@automation.register_action(
    "hid_keyboard.stop_keep_awake",
    StopKeepAwakeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(HIDKeyboard),
        }
    ),
)
async def stop_keep_awake_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
