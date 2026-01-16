import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

CODEOWNERS = ["@AntorFR"]
DEPENDENCIES = ["esp32"]
AUTO_LOAD = []

CONF_HID_MOUSE_ID = "hid_mouse_id"

hid_mouse_ns = cg.esphome_ns.namespace("hid_mouse")
HIDMouse = hid_mouse_ns.class_("HIDMouse", cg.Component)

# Actions
MoveAction = hid_mouse_ns.class_("MoveAction", automation.Action)
ClickAction = hid_mouse_ns.class_("ClickAction", automation.Action)
PressAction = hid_mouse_ns.class_("PressAction", automation.Action)
ReleaseAction = hid_mouse_ns.class_("ReleaseAction", automation.Action)
ScrollAction = hid_mouse_ns.class_("ScrollAction", automation.Action)

# Button enum
MouseButton = hid_mouse_ns.enum("MouseButton")
MOUSE_BUTTONS = {
    "LEFT": MouseButton.MOUSE_BUTTON_LEFT,
    "RIGHT": MouseButton.MOUSE_BUTTON_RIGHT,
    "MIDDLE": MouseButton.MOUSE_BUTTON_MIDDLE,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HIDMouse),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)


# Action: Move
MOVE_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(HIDMouse),
        cv.Required("x"): cv.templatable(cv.int_),
        cv.Required("y"): cv.templatable(cv.int_),
    }
)


@automation.register_action("hid_mouse.move", MoveAction, MOVE_ACTION_SCHEMA)
async def hid_mouse_move_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    
    template_ = await cg.templatable(config["x"], args, cg.int_)
    cg.add(var.set_x(template_))
    
    template_ = await cg.templatable(config["y"], args, cg.int_)
    cg.add(var.set_y(template_))
    
    return var


# Action: Click
CLICK_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(HIDMouse),
        cv.Optional("button", default="LEFT"): cv.enum(MOUSE_BUTTONS, upper=True),
    }
)


@automation.register_action("hid_mouse.click", ClickAction, CLICK_ACTION_SCHEMA)
async def hid_mouse_click_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_button(config["button"]))
    return var


# Action: Press
PRESS_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(HIDMouse),
        cv.Optional("button", default="LEFT"): cv.enum(MOUSE_BUTTONS, upper=True),
    }
)


@automation.register_action("hid_mouse.press", PressAction, PRESS_ACTION_SCHEMA)
async def hid_mouse_press_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_button(config["button"]))
    return var


# Action: Release
RELEASE_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(HIDMouse),
        cv.Optional("button", default="LEFT"): cv.enum(MOUSE_BUTTONS, upper=True),
    }
)


@automation.register_action("hid_mouse.release", ReleaseAction, RELEASE_ACTION_SCHEMA)
async def hid_mouse_release_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_button(config["button"]))
    return var


# Action: Scroll
SCROLL_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(HIDMouse),
        cv.Required("amount"): cv.templatable(cv.int_),
    }
)


@automation.register_action("hid_mouse.scroll", ScrollAction, SCROLL_ACTION_SCHEMA)
async def hid_mouse_scroll_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    
    template_ = await cg.templatable(config["amount"], args, cg.int_)
    cg.add(var.set_amount(template_))
    
    return var
