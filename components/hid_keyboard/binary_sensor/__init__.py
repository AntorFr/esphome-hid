import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_CONNECTIVITY

from .. import hid_keyboard_ns, HIDKeyboard

DEPENDENCIES = ["hid_keyboard"]
CODEOWNERS = ["@AntorFr"]

CONF_HID_KEYBOARD_ID = "hid_keyboard_id"

HIDConnectedBinarySensor = hid_keyboard_ns.class_(
    "HIDConnectedBinarySensor", binary_sensor.BinarySensor, cg.PollingComponent
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    HIDConnectedBinarySensor,
    device_class=DEVICE_CLASS_CONNECTIVITY,
).extend(
    {
        cv.GenerateID(CONF_HID_KEYBOARD_ID): cv.use_id(HIDKeyboard),
    }
).extend(cv.polling_component_schema("1s"))


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_HID_KEYBOARD_ID])
    cg.add(var.set_parent(parent))
