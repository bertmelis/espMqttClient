/*
Copyright (c) 2022 Bert Melis. All rights reserved.

Parts are based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <stdint.h>

namespace espMqttClient {

enum class DisconnectReason : uint8_t {
  TCP_DISCONNECTED = 0,
  MQTT_UNACCEPTABLE_PROTOCOL_VERSION = 1,
  MQTT_IDENTIFIER_REJECTED = 2,
  MQTT_SERVER_UNAVAILABLE = 3,
  MQTT_MALFORMED_CREDENTIALS = 4,
  MQTT_NOT_AUTHORIZED = 5,
  TLS_BAD_FINGERPRINT = 6
};

enum class ClientError : uint8_t {
  MAX_RETRIES = 0,
  OUT_OF_MEMORY = 1
};

}  // end namespace espMqttClient
