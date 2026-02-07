import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

CODEOWNERS = ["@AntorFr"]
DEPENDENCIES = ["esp32"]
CONFLICTS_WITH = ["hid_mouse", "hid_keyboard", "hid_telephony"]

CONF_LAYOUT = "layout"

hid_composite_ns = cg.esphome_ns.namespace("hid_composite")
HIDComposite = hid_composite_ns.class_("HIDComposite", cg.Component)

# Keyboard layout enum
KeyboardLayout = hid_composite_ns.enum("KeyboardLayout")
KEYBOARD_LAYOUTS = {
    "QWERTY_US": KeyboardLayout.LAYOUT_QWERTY_US,
    "AZERTY_FR": KeyboardLayout.LAYOUT_AZERTY_FR,
    "QWERTZ_DE": KeyboardLayout.LAYOUT_QWERTZ_DE,
}

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

# Keep Awake Actions
StartMouseKeepAwakeAction = hid_composite_ns.class_("StartMouseKeepAwakeAction", automation.Action)
StopMouseKeepAwakeAction = hid_composite_ns.class_("StopMouseKeepAwakeAction", automation.Action)
StartKeyboardKeepAwakeAction = hid_composite_ns.class_("StartKeyboardKeepAwakeAction", automation.Action)
StopKeyboardKeepAwakeAction = hid_composite_ns.class_("StopKeyboardKeepAwakeAction", automation.Action)

# Telephony Actions
MuteAction = hid_composite_ns.class_("MuteAction", automation.Action)
UnmuteAction = hid_composite_ns.class_("UnmuteAction", automation.Action)
ToggleMuteAction = hid_composite_ns.class_("ToggleMuteAction", automation.Action)
MuteTelephonyAction = hid_composite_ns.class_("MuteTelephonyAction", automation.Action)
MuteConsumerAction = hid_composite_ns.class_("MuteConsumerAction", automation.Action)
MuteTeamsAction = hid_composite_ns.class_("MuteTeamsAction", automation.Action)
HookSwitchAction = hid_composite_ns.class_("HookSwitchAction", automation.Action)
AnswerCallAction = hid_composite_ns.class_("AnswerCallAction", automation.Action)
HangUpAction = hid_composite_ns.class_("HangUpAction", automation.Action)

CONF_X = "x"
CONF_Y = "y"
CONF_VERTICAL = "vertical"
CONF_HORIZONTAL = "horizontal"
CONF_BUTTON = "button"
CONF_KEY = "key"
CONF_MODIFIERS = "modifiers"
CONF_TEXT = "text"
CONF_INTERVAL = "interval"

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
    cv.Optional(CONF_LAYOUT, default="QWERTY_US"): cv.enum(KEYBOARD_LAYOUTS, upper=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_layout(config[CONF_LAYOUT]))

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
    template_ = await cg.templatable(config[CONF_X], args, cg.int_)
    cg.add(var.set_x(template_))
    template_ = await cg.templatable(config[CONF_Y], args, cg.int_)
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
    template_ = await cg.templatable(config[CONF_VERTICAL], args, cg.int_)
    cg.add(var.set_vertical(template_))
    template_ = await cg.templatable(config[CONF_HORIZONTAL], args, cg.int_)
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

CONF_SPEED = "speed"
CONF_JITTER = "jitter"

TYPE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_TEXT): cv.templatable(cv.string),
    cv.Optional(CONF_SPEED, default=50): cv.templatable(cv.positive_int),
    cv.Optional(CONF_JITTER, default=0): cv.templatable(cv.positive_int),
})

@automation.register_action("hid_composite.type", TypeAction, TYPE_ACTION_SCHEMA)
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


# ============ Keep Awake Actions ============

START_MOUSE_KEEP_AWAKE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Optional(CONF_INTERVAL, default="60s"): cv.templatable(cv.positive_time_period_milliseconds),
    cv.Optional(CONF_JITTER, default="0s"): cv.templatable(cv.positive_time_period_milliseconds),
})

@automation.register_action("hid_composite.start_mouse_keep_awake", StartMouseKeepAwakeAction, START_MOUSE_KEEP_AWAKE_ACTION_SCHEMA)
async def start_mouse_keep_awake_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_INTERVAL], args, cg.uint32)
    cg.add(var.set_interval(template_))
    template_ = await cg.templatable(config[CONF_JITTER], args, cg.uint32)
    cg.add(var.set_jitter(template_))
    return var

STOP_MOUSE_KEEP_AWAKE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.stop_mouse_keep_awake", StopMouseKeepAwakeAction, STOP_MOUSE_KEEP_AWAKE_ACTION_SCHEMA)
async def stop_mouse_keep_awake_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

START_KEYBOARD_KEEP_AWAKE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_KEY): cv.templatable(cv.string),
    cv.Optional(CONF_INTERVAL, default="60s"): cv.templatable(cv.positive_time_period_milliseconds),
    cv.Optional(CONF_JITTER, default="0s"): cv.templatable(cv.positive_time_period_milliseconds),
})

@automation.register_action("hid_composite.start_keyboard_keep_awake", StartKeyboardKeepAwakeAction, START_KEYBOARD_KEEP_AWAKE_ACTION_SCHEMA)
async def start_keyboard_keep_awake_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_KEY], args, cg.std_string)
    cg.add(var.set_key(template_))
    template_ = await cg.templatable(config[CONF_INTERVAL], args, cg.uint32)
    cg.add(var.set_interval(template_))
    template_ = await cg.templatable(config[CONF_JITTER], args, cg.uint32)
    cg.add(var.set_jitter(template_))
    return var

STOP_KEYBOARD_KEEP_AWAKE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.stop_keyboard_keep_awake", StopKeyboardKeepAwakeAction, STOP_KEYBOARD_KEEP_AWAKE_ACTION_SCHEMA)
async def stop_keyboard_keep_awake_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


# ============ Telephony Actions ============

MUTE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.mute", MuteAction, MUTE_ACTION_SCHEMA)
async def mute_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

UNMUTE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.unmute", UnmuteAction, UNMUTE_ACTION_SCHEMA)
async def unmute_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

TOGGLE_MUTE_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.toggle_mute", ToggleMuteAction, TOGGLE_MUTE_ACTION_SCHEMA)
async def toggle_mute_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

# Mute Telephony only (Page 0x0B) - for testing
MUTE_TELEPHONY_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.mute_telephony", MuteTelephonyAction, MUTE_TELEPHONY_ACTION_SCHEMA)
async def mute_telephony_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

# Mute Consumer only (Page 0x0C) - system volume mute
MUTE_CONSUMER_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.mute_consumer", MuteConsumerAction, MUTE_CONSUMER_ACTION_SCHEMA)
async def mute_consumer_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

# Mute Teams - sends Ctrl+Shift+M keyboard shortcut
MUTE_TEAMS_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.mute_teams", MuteTeamsAction, MUTE_TEAMS_ACTION_SCHEMA)
async def mute_teams_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

CONF_STATE = "state"

HOOK_SWITCH_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
    cv.Required(CONF_STATE): cv.templatable(cv.boolean),
})

@automation.register_action("hid_composite.hook_switch", HookSwitchAction, HOOK_SWITCH_ACTION_SCHEMA)
async def hook_switch_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_STATE], args, cg.bool_)
    cg.add(var.set_state(template_))
    return var

ANSWER_CALL_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.answer_call", AnswerCallAction, ANSWER_CALL_ACTION_SCHEMA)
async def answer_call_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

HANG_UP_ACTION_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(HIDComposite),
})

@automation.register_action("hid_composite.hang_up", HangUpAction, HANG_UP_ACTION_SCHEMA)
async def hang_up_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
