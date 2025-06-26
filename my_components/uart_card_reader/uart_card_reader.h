#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace uart_card_reader {

/**
 * Trigger, der aufgerufen wird, wenn eine Karte gelesen wird.
 * Parameter: card_id, card_type
 */
class CardTagTrigger : public Trigger<uint32_t, uint8_t> {
 public:
  /**
   * Wird vom Component aufgerufen, um den Trigger auszulösen.
   */
  void process(uint32_t card_id, uint8_t card_type) { this->trigger(card_id, card_type); }
};

/**
 * UARTCardReader liest 9-Byte-Pakete vom UART und feuert on_tag-Trigger.
 */
class UARTCardReader : public Component, public uart::UARTDevice {
 public:
  explicit UARTCardReader(uart::UARTComponent *parent) : UARTDevice(parent) {}

  void setup() override {}

  void loop() override {
    while (available()) {
      // Startbyte
      if (read() != 0x02)
        continue;
      // Restbytes: Länge, Typ, UID0..UID3, BCC, Ende
      uint8_t buf[8];
      if (!read_array(buf, 8))
        continue;
      // Länge = 0x09, Ende = 0x03
      if (buf[0] != 0x09 || buf[7] != 0x03)
        continue;
      // BCC-Check (XOR von buf[0..5])
      uint8_t bcc = 0;
      for (int i = 0; i <= 5; ++i)
        bcc ^= buf[i];
      if (bcc != buf[6])
        continue;
      // Card ID zusammensetzen
      uint32_t card_id = 0;
      for (int i = 2; i <= 5; ++i)
        card_id = (card_id << 8) | buf[i];
      uint8_t card_type = buf[1];
      // Alle registrierten Trigger ausführen
      for (auto *trig : this->triggers_)
        trig->process(card_id, card_type);
    }
  }

  /**
   * Registrierung eines Triggers, wird von to_code() aufgerufen
   */
  void register_trigger(CardTagTrigger *trigger) { this->triggers_.push_back(trigger); }

 protected:
  std::vector<CardTagTrigger *> triggers_;
};

}  // namespace uart_card_reader
}  // namespace esphome