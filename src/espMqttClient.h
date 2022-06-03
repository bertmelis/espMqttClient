/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <WiFiClient.h>

#include "MqttClientSetup.h"

class espMqttClient : public MqttClientSetup<espMqttClient> {
 public:
#if defined(ESP32)
  explicit espMqttClient(uint8_t priority = 1, uint8_t core = 1);
#else
  espMqttClient();
#endif

 protected:
  WiFiClient _client;
};
