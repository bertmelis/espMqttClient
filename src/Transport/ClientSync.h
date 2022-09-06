/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include <WiFiClient.h>  // includes IPAddress

#include "Transport.h"

namespace espMqttClientInternals {

class ClientSync : public Transport {
 public:
  ClientSync();
  bool connect(IPAddress ip, uint16_t port) override;
  bool connect(const char* host, uint16_t port) override;
  size_t write(const uint8_t* buf, size_t size) override;
  int available() override;
  int read(uint8_t* buf, size_t size) override;
  void stop() override;
  bool connected() override;
  bool disconnected() override;
  WiFiClient client;
};

}  // namespace espMqttClientInternals

#endif
