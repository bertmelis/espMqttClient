/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include "Transport/ClientAsync.h"

#include "MqttClientSetup.h"

class espMqttClientAsync : public MqttClientSetup<espMqttClientAsync> {
 public:
#if defined(ARDUINO_ARCH_ESP32)
  explicit espMqttClientAsync(uint8_t priority = 1, uint8_t core = 1);
#else
  espMqttClientAsync();
#endif

 protected:
  ClientAsync _client;
};
