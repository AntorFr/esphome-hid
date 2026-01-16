import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, CONF_TYPE

from .. import hid_composite_ns, HIDComposite

DEPENDENCIES = ["hid_composite"]
CODEOWNERS = ["@AntorFr"]

CONF_HID_COMPOSITE_ID = "hid_composite_id"
CONF_KEY = "key"
CONF_INTERVAL = "interval"
CONF_JITTER = "jitter"

MouseKeepAwakeSwitch = hid_composite_ns.class_("MouseKeepAwakeSwitch", switch.Switch, cg.Component)
KeyboardKeepAwakeSwitch = hid_composite_ns.class_("KeyboardKeepAwakeSwitch", switch.Switch, cg.Component)
MuteSwitch = hid_composite_ns.class_("MuteSwitch", switch.Switch, cg.Component)

MOUSE_SCHEMA = switch.switch_schema(MouseKeepAwakeSwitch).extend(
    {
        cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
        cv.Optional(CONF_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_JITTER, default="0s"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)

KEYBOARD_SCHEMA = switch.switch_schema(KeyboardKeepAwakeSwitch).extend(
    {
        cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
        cv.Required(CONF_KEY): cv.string,
        cv.Optional(CONF_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_JITTER, default="0s"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)

MUTE_SCHEMA = switch.switch_schema(MuteSwitch).extend(
    {
        cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "mouse": MOUSE_SCHEMA,
        "keyboard": KEYBOARD_SCHEMA,
        "mute": MUTE_SCHEMA,
    },
    lower=True,
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_HID_COMPOSITE_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    
    cg.add(var.set_parent(parent))
    
    if config[CONF_TYPE] in ["mouse", "keyboard"]:
        cg.add(var.set_interval(config[CONF_INTERVAL]))
        cg.add(var.set_jitter(config[CONF_JITTER]))
    
    if config[CONF_TYPE] == "keyboard":
        cg.add(var.set_key(config[CONF_KEY]))
