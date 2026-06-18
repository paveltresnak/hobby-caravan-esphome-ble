import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_TYPE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_HOUR,
    UNIT_PERCENT,
    UNIT_VOLT,
)
from esphome.cpp_generator import MockObjClass

from .. import (
    CONF_KEY_NAME,
    CONF_PARENT_ID,
    CaravanDeviceComponent,
    FendtCaravan,
    fendt_caravan_ns,
)

ControlUnitDeviceSensor = fendt_caravan_ns.class_(
    "ControlUnitDeviceSensor",
    CaravanDeviceComponent,
    sensor.Sensor,
    cg.Parented.template(FendtCaravan),
)

FendtSensor = fendt_caravan_ns.class_(
    "FendtSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)


def _device_schema(class_: MockObjClass, key_name_=cv.UNDEFINED) -> cv.Schema:
    return (
        sensor.sensor_schema(
            class_,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ).extend(
            {
                cv.Required(CONF_PARENT_ID): cv.use_id(FendtCaravan),
            }
        )
    ).extend(cv.COMPONENT_SCHEMA)


def _sensor_schema(
    class_: MockObjClass,
    unit_of_measurement: str = cv.UNDEFINED,
    accuracy_decimals: int = cv.UNDEFINED,
    device_class: str = cv.UNDEFINED,
    state_class: str = cv.UNDEFINED,
    key_name_: str = cv.UNDEFINED,
) -> cv.Schema:
    return (
        sensor.sensor_schema(
            class_,
            unit_of_measurement=unit_of_measurement,
            accuracy_decimals=accuracy_decimals,
            device_class=device_class,
            state_class=state_class,
        ).extend(
            {
                cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
                cv.Optional(CONF_KEY_NAME, default=key_name_): cv.string,
            }
        )
    ).extend(cv.COMPONENT_SCHEMA)


CONFIG_SCHEMA = cv.typed_schema(
    {
        "mcu_device": _device_schema(ControlUnitDeviceSensor),
        "temp_in": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_TEMPERATURE,
            key_name_="TEMP_IN",
        ),
        "temp_out": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_TEMPERATURE,
            key_name_="TEMP_OUT",
        ),
        "batt_voltage": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_VOLTAGE,
            key_name_="IBS0_UBAT",
        ),
        "batt_current": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_CURRENT,
            key_name_="IBS0_IBAT",
        ),
        "batt_soc": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_BATTERY,
            key_name_="IBS0_SOC2",
        ),
        "batt_remaining": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_HOUR,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DURATION,
            key_name_="IBS0_REMAINING_TIME",
        ),
        "batt_temp": _sensor_schema(
            FendtSensor,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_TEMPERATURE,
            key_name_="IBS0_TEMPERATURE",
        ),
        "water_level": _sensor_schema(
            FendtSensor,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            key_name_="WATER_LEVEL",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await sensor.new_sensor(config)
    if CONF_KEY_NAME in config:
        cg.add(var.set_key_name(config[CONF_KEY_NAME]))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(getattr(parent, f"set_{config[CONF_TYPE]}_sensor")(var))
