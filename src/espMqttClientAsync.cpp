/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClientAsync.h"


#if defined(ARDUINO_ARCH_ESP32)
espMqttClientAsync::espMqttClientAsync(uint8_t priority, uint8_t core)
: MqttClientSetup(priority, core)
, _client() {
#else
espMqttClientAsync::espMqttClientAsync()
: _client() {
#endif
  _transport = &_client;
}
