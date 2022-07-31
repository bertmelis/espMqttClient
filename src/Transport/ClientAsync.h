/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <Client.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include "freertos/FreeRTOS.h"
  #include <AsyncTCP.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <EspAsyncTCP.h>
#endif

#include "../Config.h"

class ClientAsync : public Client {
 public:
  int connect(IPAddress ip, uint16_t port);
  int connect(const char *host, uint16_t port);
  size_t write(const uint8_t *buf, size_t size);
  int available();
  int read(uint8_t *buf, size_t size);
  void stop();
  uint8_t connected();

  void stop(bool force);

  // pure virtual methods in Client, not used in espMqttClient
  size_t write(uint8_t) { return 0; }
  int read() { return 0; }
  int peek() { return 0; }
  void flush() {}
  operator bool() { return false; }

 private:
  #if defined(ARDUINO_ARCH_ESP32)
  AsyncClient _client;
  #elif defined(ARDUINO_ARCH_ESP8266)
  ESPAsyncClient _client;
  #endif
};
