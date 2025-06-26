#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace uart_card_reader {

/**
 * Component to read 9-byte RFID frames from UART and fire an on_tag trigger
 */
class UARTCardReader : public Component, public uart::UARTDevice {
 public:
  explicit UARTCardReader(uart::UARTComponent *parent) : UARTDevice(parent) {
      explicit UARTCardReader(uart::UARTComponent *parent) : UARTDevice(parent) {}

  void setup() override {}

  void loop() override {
    while (available()) {
      if (read() != 0x02)  // Start byte
        continue;
      uint8_t buf[8];
      if (!read_array(buf, 8))  // length, type, UID(4), BCC, end
        continue;
      if (buf[0] != 0x09 || buf[7] != 0x03)  // length & end
        continue;
      uint8_t bcc = 0;
      for (int i = 0; i <= 5; ++i)
        bcc ^= buf[i];
      if (bcc != buf[6])  // BCC check
        continue;
      uint32_t tag_id = 0;
      for (int i = 2; i <= 5; ++i)
        tag_id = (tag_id << 8) | buf[i];
      uint8_t tag_type = buf[1];
      if (on_tag_)  // guard null pointer
        on_tag_->trigger(tag_id, tag_type);
    }
  }

  // Allow Python integration to set the trigger
  void set_on_tag_trigger(Trigger<uint32_t, uint8_t> *trigger) { on_tag_ = trigger; }

  void setup() override {}

  void loop() override {
    while (available()) {
      if (read() != 0x02)  // Start byte
        continue;
      uint8_t buf[8];
      if (!read_array(buf, 8))  // length, type, UID(4), BCC, end
        continue;
      if (buf[0] != 0x09 || buf[7] != 0x03)  // length & end
        continue;
      uint8_t bcc = 0;
      for (int i = 0; i <= 5; ++i)
        bcc ^= buf[i];
      if (bcc != buf[6])  // BCC check
        continue;
      uint32_t tag_id = 0;
      for (int i = 2; i <= 5; ++i)
        tag_id = (tag_id << 8) | buf[i];
      uint8_t tag_type = buf[1];
      // Always safe to trigger, pointer initialized in constructor
      this->on_tag_->trigger(tag_id, tag_type);
    }
  }

  /**
   * Automation uses this accessor to attach on_tag actions
   */
  Trigger<uint32_t, uint8_t>* get_tag_trigger() { return this->on_tag_; }

 protected:
  Trigger<uint32_t, uint8_t> *on_tag_{nullptr};
};

}  // namespace uart_card_reader
}  // namespace esphome