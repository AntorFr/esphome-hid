import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

CODEOWNERS = ["@AntorFr"]
DEPENDENCIES = ["esp32"]

# Cannot be used with other HID components (each configures USB)
CONFLICTS_WITH = ["hid_mouse", "hid_keyboard", "hid_composite"]

hid_telephony_ns = cg.esphome_ns.namespace("hid_telephony")
HIDTelephony = hid_telephony_ns.class_("HIDTelephony", cg.Component)

# Actions
MuteAction = hid_telephony_ns.class_("MuteAction", automation.Action)
UnmuteAction = hid_telephony_ns.class_("UnmuteAction", automation.Action)
ToggleMuteAction = hid_telephony_ns.class_("ToggleMuteAction", automation.Action)
HookSwitchAction = hid_telephony_ns.class_("HookSwitchAction", automation.Action)
HangUpAction = hid_telephony_ns.class_("HangUpAction", automation.Action)
AnswerAction = hid_telephony_ns.class_("AnswerAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HIDTelephony),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)


# Action: Mute
@automation.register_action(
    "hid_telephony.mute",
    MuteAction,
    cv.Schema({cv.GenerateID(): cv.use_id(HIDTelephony)}),
)
async def mute_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


# Action: Unmute
@automation.register_action(
    "hid_telephony.unmute",
    UnmuteAction,
    cv.Schema({cv.GenerateID(): cv.use_id(HIDTelephony)}),
)
async def unmute_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


# Action: Toggle Mute
@automation.register_action(
    "hid_telephony.toggle_mute",
    ToggleMuteAction,
    cv.Schema({cv.GenerateID(): cv.use_id(HIDTelephony)}),
)
async def toggle_mute_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


# Action: Hook Switch (toggle off-hook/on-hook)
@automation.register_action(
    "hid_telephony.hook_switch",
    HookSwitchAction,
    cv.Schema({cv.GenerateID(): cv.use_id(HIDTelephony)}),
)
async def hook_switch_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


# Action: Answer call (go off-hook)
@automation.register_action(
    "hid_telephony.answer",
    AnswerAction,
    cv.Schema({cv.GenerateID(): cv.use_id(HIDTelephony)}),
)
async def answer_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


# Action: Hang up (go on-hook)
@automation.register_action(
    "hid_telephony.hang_up",
    HangUpAction,
    cv.Schema({cv.GenerateID(): cv.use_id(HIDTelephony)}),
)
async def hang_up_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
