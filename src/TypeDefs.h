/*
Copyright (c) 2022 Bert Melis. All rights reserved.

Parts are based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <functional>

namespace espMqttClientTypes {

enum class DisconnectReason : uint8_t {
  USER_OK = 0,
  MQTT_UNACCEPTABLE_PROTOCOL_VERSION = 1,
  MQTT_IDENTIFIER_REJECTED = 2,
  MQTT_SERVER_UNAVAILABLE = 3,
  MQTT_MALFORMED_CREDENTIALS = 4,
  MQTT_NOT_AUTHORIZED = 5,
  TLS_BAD_FINGERPRINT = 6,
  TCP_DISCONNECTED = 7
};

static const char* disconnectReasonToString(DisconnectReason reason) {
  switch (reason) {
    case DisconnectReason::USER_OK:                            return "No error";
    case DisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION: return "Unacceptable protocol version";
    case DisconnectReason::MQTT_IDENTIFIER_REJECTED:           return "Identified rejected";
    case DisconnectReason::MQTT_SERVER_UNAVAILABLE:            return "Server unavailable";
    case DisconnectReason::MQTT_MALFORMED_CREDENTIALS:         return "Malformed credentials";
    case DisconnectReason::MQTT_NOT_AUTHORIZED:                return "Not authorized";
    case DisconnectReason::TLS_BAD_FINGERPRINT:                return "Bad fingerprint";
    case DisconnectReason::TCP_DISCONNECTED:                   return "TCP disconnected";
  }
}

enum class SubscribeReturncode : uint8_t {
  QOS0 = 0x00,
  QOS1 = 0x01,
  QOS2 = 0x02,
  FAIL = 0X80
};

static const char* subscribeReturncodeToString(SubscribeReturncode returnCode) {
  switch (returnCode) {
    case SubscribeReturncode::QOS0: return "QoS 0";
    case SubscribeReturncode::QOS1: return "QoS 1";
    case SubscribeReturncode::QOS2: return "QoS 2";
    case SubscribeReturncode::FAIL: return "Failed";
  }
}

enum class Error : uint8_t {
  SUCCESS = 0,
  OUT_OF_MEMORY = 1,
  MAX_RETRIES = 2,
  MALFORMED_PARAMETER = 3,
  MISC_ERROR = 4
};

static const char* errorToString(Error error) {
  switch (error) {
    case Error::SUCCESS:             return "Success";
    case Error::OUT_OF_MEMORY:       return "Out of memory";
    case Error::MAX_RETRIES:         return "Maximum retries exceeded";
    case Error::MALFORMED_PARAMETER: return "Malformed parameters";
    case Error::MISC_ERROR:          return "Misc error";
  }
}

struct MessageProperties {
  uint8_t qos;
  bool dup;
  bool retain;
  uint16_t packetId;
};

typedef std::function<void(bool sessionPresent)> OnConnectCallback;
typedef std::function<void(DisconnectReason reason)> OnDisconnectCallback;
typedef std::function<void(uint16_t packetId, const SubscribeReturncode* returncodes, size_t len)> OnSubscribeCallback;
typedef std::function<void(uint16_t packetId)> OnUnsubscribeCallback;
typedef std::function<void(const MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total)> OnMessageCallback;
typedef std::function<void(uint16_t packetId)> OnPublishCallback;
typedef std::function<size_t(uint8_t* data, size_t maxSize, size_t index)> PayloadCallback;
typedef std::function<void(uint16_t packetId, Error error)> OnErrorCallback;

}  // end namespace espMqttClientTypes
