import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UART_ID

DEPENDENCIES = ['uart']
CONF_ON_TAG = 'on_tag'

# Namespace und Klassen
uart_card_reader_ns = cg.esphome_ns.namespace('uart_card_reader')
UARTCardReader = uart_card_reader_ns.class_(
    'UARTCardReader', cg.Component, uart.UARTDevice)
CardTagTrigger = uart_card_reader_ns.class_(
    'CardTagTrigger', automation.Trigger.template(cg.uint32, cg.uint8))

# Konfigurationsschema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(UARTCardReader),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_ON_TAG): automation.validate_automation({
        cv.GenerateID(): cv.declare_id(CardTagTrigger),
    }),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # UART-Komponente holen und Instanz erzeugen
    parent = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)

    # Trigger für jedes on_tag-Element anlegen
    if CONF_ON_TAG in config:
        for conf in config[CONF_ON_TAG]:
            # Trigger-Objekt erzeugen
            trigger = cg.new_Pvariable(conf[CONF_ID])
            # In der C++-Komponente registrieren
            cg.add(var.register_trigger(trigger))
            # Automationsblöcke binden (card_id, card_type)
            await automation.build_automation(
                trigger,
                [(cg.uint32, 'card_id'), (cg.uint8, 'card_type')],
                conf,
            )