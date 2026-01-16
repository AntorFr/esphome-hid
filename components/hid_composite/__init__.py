import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

CODEOWNERS = ["@AntorFr"]
DEPENDENCIES = ["esp32"]
CONFLICTS_WITH = ["hid_mouse", "hid_keyboard"]

hid_composite_ns = cg.esphome_ns.namespace("hid_composite")
HIDComposite = hid_composite_ns.class_("HIDComposite", cg.Component)

# Mouse Actions
MoveAction = hid_composite_ns.class_("MoveAction", automation.Action)
ScrollAction = hid_composite_ns.class_("ScrollAction", automation.Action)
ClickAction = hid_composite_ns.class_("ClickAction", automation.Action)
MousePressAction = hid_composite_ns.class_("MousePressAction", automation.Action)
MouseReleaseAction = hid_composite_ns.class_("MouseReleaseAction", automation.Action)
MouseReleaseAllAction = hid_composite_ns.class_("MouseReleaseAllAction", automation.Action)

# Keyboard Actions
KeyPressAction = hid_composite_ns.class_("KeyPressAction", automation.Action)
KeyReleaseAction = hid_composite_ns.class_("KeyReleaseAction", automation.Action)
KeyTapAction = hid_composite_ns.class_("KeyTapAction", automation.Action)
KeyReleaseAllAction = hid_composite_ns.class_("KeyReleaseAllAction", automation.Action)
TypeAction = hid_composite_ns.class_("TypeAction", automation.Action)

CONF_X = "x"
CONF_Y = "y"
CONF_VERTICAL = "vertical"
CONF_HORIZONTAL = "horizontal"
CONF_BUTTON = "button"
CONF_KEY = "key"
CONF_MODIFIERS = "modifiers"
CONF_TEXT = "text"

MOUSE_BUTTONS = {
    "LEFT": 0,
    "RIGHT": 1,
    "MIDDLE": 2,
}

MODIFIERS = {
    "NONE": 0x00,
    "CTRL": 0x01, "LEFT_CTRL": 0x01, "LCTRL": 0x01,
    "SHIFT": 0x02, "LEFT_SHIFT": 0x02, "LSHIFT": 0x02,
    "ALT": 0x04, "LEFT_ALT": 0x04, "LALT": 0x04,
    "GUI": 0x08, "LEFT_GUI": 0x08, "LGUI": 0x08, "WIN": 0x08, "CMD": 0x08, "META": 0x08,
    "RIGHT_CTRL": 0x10, "RCTRL": 0x10,
    "RIGHT_SHIFT": 0x20, "RSHIFT": 0x20,
    "RIGHT_ALT": 0x40, "RALT": 0x40,
    "RIGHT_GUI": 0x80, "RGUI": 0x80,
    "CTRL_SHIFT": 0x03, "CTRL_ALT": 0x05, "CTRL_GUI": 0x09,
    "SHIFT_ALT": 0x06, "SHIFT_GUI": 0x0A, "ALT_GUI": 0x0C,
    "CTRL_SHIFT_ALT": 0x07, "CTRL_SHIFT_GUI": 0x0B, "CTRL_ALT_GUI": 0x0D,
    "SHIFT_ALT_GUI": 0x0E, "CTRL_SHIFT_ALT_GUI": 0x0F,
}

def validate_button(value):
    if isinstance(value, str):
        upper = value.upper()
        if upper in MOUSE_BUTTONS:
            return MOUSE_BUTTONS[upper]
    return cv.int_range(min=0, max=2)(value)

def validate_modifiers(value):
    if isinstance(value, int):
        return cv.int_range(min=0, max=255)(value)
    if isinstance(value, str):
        upper = value.upper()
        if upper in MODIFIERS:
            return MODIFIERS[upper]
        raise cv.Invalid(f"Unknown modifier: {value}")
    raise cv.Invalid(f"Invalid modifier type: {type(value)}")

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(HIDComposite),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

# ============ Mouse Actions ============

MOVE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_X): cv.templatable(cv.int_range(min=-127, max=127)),
    cv.Required(CONF_Y): cv.templatable(cv.int_range(min=-127, max=127)),
})

@automation.register_action("hid_composite.move", MoveAction, MOVE_ACTION_SCHEMA)
async def move_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_X], args, cg.int8)
    cg.add(var.set_x(template_))
    template_ = await cg.templatable(config[CONF_Y], args, cg.int8)
    cg.add(var.set_y(template_))
    return var

SCROLL_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Optional(CONF_VERTICAL, default=0): cv.templatable(cv.int_range(min=-127, max=127)),
    cv.Optional(CONF_HORIZONTAL, default=0): cv.templatable(cv.int_range(min=-127, max=127)),
})

@automation.register_action("hid_composite.scroll", ScrollAction, SCROLL_ACTION_SCHEMA)
async def scroll_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_VERTICAL], args, cg.int8)
    cg.add(var.set_vertical(template_))
    template_ = await cg.templatable(config[CONF_HORIZONTAL], args, cg.int8)
    cg.add(var.set_horizontal(template_))
    return var

CLICK_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Optional(CONF_BUTTON, default="LEFT"): cv.templatable(validate_button),
})

@automation.register_action("hid_composite.click", ClickAction, CLICK_ACTION_SCHEMA)
async def click_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_BUTTON], args, cg.uint8)
    cg.add(var.set_button(template_))
    return var

MOUSE_PRESS_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Optional(CONF_BUTTON, default="LEFT"): cv.templatable(validate_button),
})

@automation.register_action("hid_composite.mouse_press", MousePressAction, MOUSE_PRESS_ACTION_SCHEMA)
async def mouse_press_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_BUTTON], args, cg.uint8)
    cg.add(var.set_button(template_))
    return var

MOUSE_RELEASE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Optional(CONF_BUTTON, default="LEFT"): cv.templatable(validate_button),
})

@automation.register_action("hid_composite.mouse_release", MouseReleaseAction, MOUSE_RELEASE_ACTION_SCHEMA)
async def mouse_release_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_BUTTON], args, cg.uint8)
    cg.add(var.set_button(template_))
    return var

MOUSE_RELEASE_ALL_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.mouse_release_all", MouseReleaseAllAction, MOUSE_RELEASE_ALL_ACTION_SCHEMA)
async def mouse_release_all_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

# ============ Keyboard Actions ============

KEY_PRESS_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_KEY): cv.templatable(cv.string),
    cv.Optional(CONF_MODIFIERS, default="NONE"): validate_modifiers,
})

@automation.register_action("hid_composite.key_press", KeyPressAction, KEY_PRESS_ACTION_SCHEMA)
async def key_press_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_KEY], args, cg.std_string)
    cg.add(var.set_key(template_))
    cg.add(var.set_modifier(config[CONF_MODIFIERS]))
    return var

KEY_RELEASE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.key_release", KeyReleaseAction, KEY_RELEASE_ACTION_SCHEMA)
async def key_release_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

KEY_TAP_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_KEY): cv.templatable(cv.string),
    cv.Optional(CONF_MODIFIERS, default="NONE"): validate_modifiers,
})

@automation.register_action("hid_composite.key_tap", KeyTapAction, KEY_TAP_ACTION_SCHEMA)
async def key_tap_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_KEY], args, cg.std_string)
    cg.add(var.set_key(template_))
    cg.add(var.set_modifier(config[CONF_MODIFIERS]))
    return var

KEY_RELEASE_ALL_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.key_release_all", KeyReleaseAllAction, KEY_RELEASE_ALL_ACTION_SCHEMA)
async def key_release_all_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

TYPE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_TEXT): cv.templatable(cv.string),
})

@automation.register_action("hid_composite.type", TypeAction, TYPE_ACTION_SCHEMA)
async def type_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_TEXT], args, cg.std_string)
    cg.add(var.set_text(template_))
    return var
