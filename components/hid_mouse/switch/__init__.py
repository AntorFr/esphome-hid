import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID

from .. import hid_mouse_ns, HIDMouse

DEPENDENCIES = ["hid_mouse"]
CODEOWNERS = ["@AntorFr"]

CONF_HID_MOUSE_ID = "hid_mouse_id"
CONF_INTERVAL = "interval"
CONF_JITTER = "jitter"

KeepAwakeSwitch = hid_mouse_ns.class_("KeepAwakeSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = switch.switch_schema(KeepAwakeSwitch).extend(
    {
        cv.GenerateID(CONF_HID_MOUSE_ID): cv.use_id(HIDMouse),
        cv.Optional(CONF_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_JITTER, default="0s"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_HID_MOUSE_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_interval(config[CONF_INTERVAL]))
    cg.add(var.set_jitter(config[CONF_JITTER]))
