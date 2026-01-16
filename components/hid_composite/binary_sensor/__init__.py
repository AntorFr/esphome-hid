import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_TYPE, DEVICE_CLASS_CONNECTIVITY

from .. import hid_composite_ns, HIDComposite

DEPENDENCIES = ["hid_composite"]
CODEOWNERS = ["@AntorFr"]

CONF_HID_COMPOSITE_ID = "hid_composite_id"

HIDConnectedBinarySensor = hid_composite_ns.class_(
    "HIDConnectedBinarySensor", binary_sensor.BinarySensor, cg.PollingComponent
)
HIDMutedBinarySensor = hid_composite_ns.class_(
    "HIDMutedBinarySensor", binary_sensor.BinarySensor, cg.Component
)
HIDInCallBinarySensor = hid_composite_ns.class_(
    "HIDInCallBinarySensor", binary_sensor.BinarySensor, cg.Component
)
HIDRingingBinarySensor = hid_composite_ns.class_(
    "HIDRingingBinarySensor", binary_sensor.BinarySensor, cg.Component
)

TYPES = {
    "connected": {
        "class": HIDConnectedBinarySensor,
        "device_class": DEVICE_CLASS_CONNECTIVITY,
        "polling": True,
    },
    "muted": {
        "class": HIDMutedBinarySensor,
        "device_class": "sound",
        "polling": False,
    },
    "in_call": {
        "class": HIDInCallBinarySensor,
        "device_class": "occupancy",
        "polling": False,
    },
    "ringing": {
        "class": HIDRingingBinarySensor,
        "device_class": "sound",
        "polling": False,
    },
}


def validate_type(value):
    value = cv.string_strict(value).lower()
    if value not in TYPES:
        raise cv.Invalid(f"Unknown sensor type: {value}. Valid types are: {list(TYPES.keys())}")
    return value


CONFIG_SCHEMA = cv.typed_schema(
    {
        "connected": binary_sensor.binary_sensor_schema(
            HIDConnectedBinarySensor,
            device_class=DEVICE_CLASS_CONNECTIVITY,
        ).extend({
            cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
        }).extend(cv.polling_component_schema("1s")),
        
        "muted": binary_sensor.binary_sensor_schema(
            HIDMutedBinarySensor,
            device_class="sound",
        ).extend({
            cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
        }).extend(cv.COMPONENT_SCHEMA),
        
        "in_call": binary_sensor.binary_sensor_schema(
            HIDInCallBinarySensor,
            device_class="occupancy",
        ).extend({
            cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
        }).extend(cv.COMPONENT_SCHEMA),
        
        "ringing": binary_sensor.binary_sensor_schema(
            HIDRingingBinarySensor,
            device_class="sound",
        ).extend({
            cv.GenerateID(CONF_HID_COMPOSITE_ID): cv.use_id(HIDComposite),
        }).extend(cv.COMPONENT_SCHEMA),
    },
    default_type="connected",
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_HID_COMPOSITE_ID])
    cg.add(var.set_parent(parent))
