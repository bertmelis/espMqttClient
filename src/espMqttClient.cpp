/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClient.h"


#if defined(ESP32)
espMqttClient::espMqttClient(uint8_t priority, uint8_t core)
: MqttClientSetup(priority, core)
, _client() {
  _transport = &_client;
}
#else
espMqttClient::espMqttClient()
: _client() {
  _transport = &_client;
}
#endif
