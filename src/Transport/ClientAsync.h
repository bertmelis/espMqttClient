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
  #include <ESPAsyncTCP.h>
#endif

#include "Transport.h"
#include "../Config.h"
#include "../Logging.h"

namespace espMqttClientInternals {

class ClientAsync : public Transport {
 public:
  ClientAsync();
  bool connect(IPAddress ip, uint16_t port);
  bool connect(const char* host, uint16_t port);
  size_t write(const uint8_t* buf, size_t size);
  int available();
  int read(uint8_t* buf, size_t size);
  void stop();
  bool connected();
  bool disconnected();

  AsyncClient client;
  size_t availableData;
  uint8_t* bufData;
};

}  // end namespace espMqttClientInternals
