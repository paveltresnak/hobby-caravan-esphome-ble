import esphome.codegen as cg
from esphome.components import ble_client, esp32_ble_tracker
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@rawsludge"]
DEPENDENCIES = ["ble_client", "esp32_ble_tracker"]
AUTO_LOAD = ["number", "select", "light"]
PLATFORMS = ["esp32"]

MULTI_CONF = True

fendt_caravan_ns = cg.esphome_ns.namespace("fendt_caravan")
FendtCaravan = fendt_caravan_ns.class_(
    "FendtCaravan", ble_client.BLEClientNode, cg.Component
)
CaravanDeviceComponent = fendt_caravan_ns.class_("CaravanDeviceComponent", cg.Component)

CONF_PARENT_ID = "parent_id"
CONF_KEY_NAME = "key_name"
CONF_BT_ID = "bt_id"

FendtNumber = fendt_caravan_ns.class_("FendtNumber", cg.Component)
FendtSelect = fendt_caravan_ns.class_("FendtSelect", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(FendtCaravan),
            cv.Required(ble_client.CONF_BLE_CLIENT_ID): cv.use_id(ble_client.BLEClient),
            # Address announced to the panel in the handshake. The default is the phone the
            # protocol was captured from: the panel drops the link right after a BT_ID it
            # does not know (tried 2026-10-05 with the ESP's own address - reconnect loop).
            cv.Optional(CONF_BT_ID, default="C0:EE:FB:90:B0:A7"): cv.mac_address,
        }
    ).extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    cg.add(var.set_bt_id(str(config[CONF_BT_ID]).lower()))
