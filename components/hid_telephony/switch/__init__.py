import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, CONF_TYPE

from .. import hid_telephony_ns, HIDTelephony

DEPENDENCIES = ["hid_telephony"]
CODEOWNERS = ["@AntorFr"]

CONF_HID_TELEPHONY_ID = "hid_telephony_id"

MuteSwitch = hid_telephony_ns.class_("MuteSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "mute": switch.switch_schema(MuteSwitch).extend(
            {
                cv.GenerateID(CONF_HID_TELEPHONY_ID): cv.use_id(HIDTelephony),
            }
        ).extend(cv.COMPONENT_SCHEMA),
    },
    default_type="mute",
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_HID_TELEPHONY_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    
    cg.add(var.set_parent(parent))
