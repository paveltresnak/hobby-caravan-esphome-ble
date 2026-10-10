import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv

from .. import CONF_PARENT_ID, CaravanDeviceComponent, fendt_caravan_ns

FendtDometicClimate = fendt_caravan_ns.class_(
    "FendtDometicClimate",
    climate.Climate,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)

# State from the panel, control through the BLE commands in climate/dometic_commands.h.
CONFIG_SCHEMA = cv.typed_schema(
    {
        "dometic_freshjet": climate.climate_schema(FendtDometicClimate)
        .extend(
            {
                cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            }
        )
        .extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
