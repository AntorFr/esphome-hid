import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_TYPE, DEVICE_CLASS_CONNECTIVITY

from .. import hid_telephony_ns, HIDTelephony

DEPENDENCIES = ["hid_telephony"]
CODEOWNERS = ["@AntorFr"]

CONF_HID_TELEPHONY_ID = "hid_telephony_id"

# Binary sensor types
CONF_TYPE_CONNECTED = "connected"
CONF_TYPE_MUTED = "muted"
CONF_TYPE_IN_CALL = "in_call"
CONF_TYPE_RINGING = "ringing"

TelephonyConnectedBinarySensor = hid_telephony_ns.class_(
    "TelephonyConnectedBinarySensor", binary_sensor.BinarySensor, cg.PollingComponent
)
TelephonyMutedBinarySensor = hid_telephony_ns.class_(
    "TelephonyMutedBinarySensor", binary_sensor.BinarySensor, cg.Component
)
TelephonyInCallBinarySensor = hid_telephony_ns.class_(
    "TelephonyInCallBinarySensor", binary_sensor.BinarySensor, cg.Component
)
TelephonyRingingBinarySensor = hid_telephony_ns.class_(
    "TelephonyRingingBinarySensor", binary_sensor.BinarySensor, cg.Component
)

BASE_SCHEMA = {
    cv.GenerateID(CONF_HID_TELEPHONY_ID): cv.use_id(HIDTelephony),
}

TYPES = {
    CONF_TYPE_CONNECTED: binary_sensor.binary_sensor_schema(
        TelephonyConnectedBinarySensor,
        device_class=DEVICE_CLASS_CONNECTIVITY,
    ).extend(cv.polling_component_schema("1s")).extend(BASE_SCHEMA),
    CONF_TYPE_MUTED: binary_sensor.binary_sensor_schema(
        TelephonyMutedBinarySensor,
    ).extend(cv.COMPONENT_SCHEMA).extend(BASE_SCHEMA),
    CONF_TYPE_IN_CALL: binary_sensor.binary_sensor_schema(
        TelephonyInCallBinarySensor,
    ).extend(cv.COMPONENT_SCHEMA).extend(BASE_SCHEMA),
    CONF_TYPE_RINGING: binary_sensor.binary_sensor_schema(
        TelephonyRingingBinarySensor,
    ).extend(cv.COMPONENT_SCHEMA).extend(BASE_SCHEMA),
}

CONFIG_SCHEMA = cv.typed_schema(TYPES, lower=True)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_HID_TELEPHONY_ID])
    cg.add(var.set_parent(parent))
