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
#include <stddef.h>
#include <functional>

#include "Packets/Property.h"

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

const char* disconnectReasonToString(DisconnectReason reason);

enum class SubscribeReturncode : uint8_t {
  QOS0 = 0x00,
  QOS1 = 0x01,
  QOS2 = 0x02,
  FAIL = 0X80
};

const char* subscribeReturncodeToString(SubscribeReturncode returnCode);

enum class Error : uint8_t {
  SUCCESS = 0,
  OUT_OF_MEMORY = 1,
  MAX_RETRIES = 2,
  MALFORMED_PARAMETER = 3,
  MISC_ERROR = 4
};

const char* errorToString(Error error);

struct MessageProperties {
  uint8_t qos;
  bool dup;
  bool retain;
  uint16_t packetId;
};

constexpr struct {
  const uint8_t Invalid                         = 0x00;
  const uint8_t PayloadFormatIndicator          = 0x01;
  const uint8_t MessageExpiryInterval           = 0x02;
  const uint8_t ContentType                     = 0x03;
  const uint8_t ResponseTopic                   = 0x08;
  const uint8_t CorrelationData                 = 0x09;
  const uint8_t SubscriptionIdentifier          = 0x0B;
  const uint8_t SessionExpiryInterval           = 0x11;
  const uint8_t AssignedClientId                = 0x12;
  const uint8_t ServerKeepAlive                 = 0x13;
  const uint8_t AuthenticationMethod            = 0x15;
  const uint8_t AuthenticationData              = 0x16;
  const uint8_t RequestProblemInformation       = 0x17;
  const uint8_t WillDelay                       = 0x18;
  const uint8_t RequestResponse                 = 0x19;
  const uint8_t ResponseInformation             = 0x1A;
  const uint8_t ServerReference                 = 0x1C;
  const uint8_t ReasonString                    = 0x1F;
  const uint8_t ReceiveMaximum                  = 0x21;
  const uint8_t TopicAliasMaximum               = 0x22;
  const uint8_t TopicAlias                      = 0x23;
  const uint8_t MaximumQoS                      = 0x24;
  const uint8_t RetainAvailable                 = 0x25;
  const uint8_t UserProperty                    = 0x26;
  const uint8_t MaximumPacketSize               = 0x27;
  const uint8_t WildcardSubcriptionAvailable    = 0x28;
  const uint8_t SubscriptionIdentifierAvailable = 0x29;
  const uint8_t SharedSubscriptionAvailable     = 0x2A;
} PropertyId;

class PropertyCollection;  // forward declare see Packets/Property.h

// MQTT 3.1.1
typedef std::function<void(bool sessionPresent)> OnConnectCallback;
typedef std::function<void(DisconnectReason reason)> OnDisconnectCallback;
typedef std::function<void(uint16_t packetId, const SubscribeReturncode* returncodes, size_t len)> OnSubscribeCallback;
typedef std::function<void(uint16_t packetId)> OnUnsubscribeCallback;
typedef std::function<void(const MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total)> OnMessageCallback;
typedef std::function<void(uint16_t packetId)> OnPublishCallback;

// MQTT 5.0
typedef std::function<void(bool sessionPresent, const espMqttClientTypes::PropertyCollection& propertyCollection)> OnConnect5Callback;
typedef std::function<void(DisconnectReason reason, const espMqttClientTypes::PropertyCollection& propertyCollection)> OnDisconnect5Callback;
typedef std::function<void(uint16_t packetId, const SubscribeReturncode* returncodes, size_t len, const espMqttClientTypes::PropertyCollection& propertyCollection)> OnSubscribe5Callback;
typedef std::function<void(uint16_t packetId, const espMqttClientTypes::PropertyCollection& propertyCollection)> OnUnsubscribe5Callback;
typedef std::function<void(const MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total, const espMqttClientTypes::PropertyCollection& propertyCollection)> OnMessage5Callback;
typedef std::function<void(uint16_t packetId, const espMqttClientTypes::PropertyCollection& propertyCollection)> OnPublish5Callback;

typedef std::function<size_t(uint8_t* data, size_t maxSize, size_t index)> PayloadCallback;
typedef std::function<void(uint16_t packetId, Error error)> OnErrorCallback;

enum class UseInternalTask {
  NO = 0,
  YES = 1,
};

}  // end namespace espMqttClientTypes
