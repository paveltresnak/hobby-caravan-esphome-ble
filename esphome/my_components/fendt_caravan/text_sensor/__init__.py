import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_TYPE, ENTITY_CATEGORY_DIAGNOSTIC

from .. import (
    CONF_KEY_NAME,
    CONF_PARENT_ID,
    CaravanDeviceComponent,
    FendtCaravan,
    fendt_caravan_ns,
)

CONF_FENDT_CARAVAN_ID = "fendt_caravan_id"
CONF_IGNORE_KEYS = "ignore_keys"
TYPE_TRAFFIC = "traffic"

FendtTextSensor = fendt_caravan_ns.class_(
    "FendtTextSensor",
    text_sensor.TextSensor,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)


def _text_schema(
    device_class: str = cv.UNDEFINED,
    entity_category: str = cv.UNDEFINED,
    icon: str = cv.UNDEFINED,
    key_name=cv.UNDEFINED,
) -> cv.Schema:
    return text_sensor.text_sensor_schema(
        FendtTextSensor,
        device_class=device_class,
        entity_category=entity_category,
        icon=icon,
    ).extend(
        {
            cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            cv.Optional(CONF_KEY_NAME, default=key_name): cv.string,
        }
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "power_status": _text_schema(icon="mdi:power-plug", key_name="LINE_EN"),
        "software_version": _text_schema(
            icon="mdi:application-braces-outline", key_name="SOFTWARE_VERSION"
        ),
        # Every command sent to the panel and every message it sends back, one state per
        # line ("#12 -> cmd-tgl:LIGHT_KUECHE", "#13 <- LIGHT_KUECHE:1"). Meant for testing
        # commands from a phone: the history of the entity reads like the debug log.
        # Lines leave one per 250 ms, the API would pass on only the last one of a burst.
        TYPE_TRAFFIC: text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:swap-horizontal",
        ).extend(
            {
                cv.Required(CONF_FENDT_CARAVAN_ID): cv.use_id(FendtCaravan),
                # the temperatures arrive several times a second and would drown the rest
                cv.Optional(
                    CONF_IGNORE_KEYS, default=["TEMP_IN", "TEMP_OUT"]
                ): cv.ensure_list(cv.string),
            }
        ),
    }
)


async def to_code(config):
    if config[CONF_TYPE] == TYPE_TRAFFIC:
        hub = await cg.get_variable(config[CONF_FENDT_CARAVAN_ID])
        var = await text_sensor.new_text_sensor(config)
        cg.add(hub.set_traffic_text_sensor(var))
        for key in config[CONF_IGNORE_KEYS]:
            cg.add(hub.add_traffic_ignore_key(key))
        return

    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await text_sensor.new_text_sensor(config)
    if CONF_KEY_NAME in config:
        cg.add(var.set_key_name(config[CONF_KEY_NAME]))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(getattr(parent, f"set_{config[CONF_TYPE]}_text_sensor")(var))
