/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(__linux__)

#include "Transport/ClientPosix.h"

#include "MqttClientSetup.h"

class espMqttClientPosix : public MqttClientSetup<espMqttClientPosix> {
 public:
  espMqttClientPosix();

 protected:
  espMqttClientInternals::ClientPosix _client;
};

#endif
