/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "Constants.h"
#include "Config.h"
#include "../TypeDefs.h"
#include "../Helpers.h"
#include "../Logging.h"
#include "RemainingLength.h"
#include "String.h"

namespace espMqttClientInternals {

class Packet {
 public:
  ~Packet();
  size_t available(size_t index);
  const uint8_t* data(size_t index) const;

  size_t size() const;
  void setDup();
  uint16_t packetId() const;
  MQTTPacketType packetType() const;
  bool removable() const;

  void* token;  // native typeless variable to store any additional data

 protected:
  explicit Packet(espMqttClientTypes::Error& error);  // NOLINT(runtime/references)

  uint16_t _packetId;  // save as separate variable: will be accessed frequently
  uint8_t* _data;
  size_t _size;

  // variables for chunked payload handling
  size_t _payloadIndex;
  size_t _payloadStartIndex;
  size_t _payloadEndIndex;
  espMqttClientTypes::PayloadCallback _getPayload;

  struct SubscribeList {
    const char* topic;
    uint8_t qos;
  };

 public:
  // CONNECT
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         bool cleanSession,
         const char* username,
         const char* password,
         const char* willTopic,
         bool willRetain,
         uint8_t willQos,
         const uint8_t* willPayload,
         uint16_t willPayloadLength,
         uint16_t keepAlive,
         const char* clientId);
  // PUBLISH
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic,
         const uint8_t* payload,
         size_t payloadLength,
         uint8_t qos,
         bool retain);
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic,
         espMqttClientTypes::PayloadCallback payloadCallback,
         size_t payloadLength,
         uint8_t qos,
         bool retain);
  // SUBSCRIBE
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic,
         uint8_t qos);
  template<typename ... Args>
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic,
         uint8_t qos,
         Args&& ... args) {
    // to be implemented
  }
  // UNSUBSCRIBE
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic);
  template<typename ... Args>
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic,
         Args&& ... args) {
    // to be implemented
         }
  // PUBACK, PUBREC, PUBREL, PUBCOMP
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         MQTTPacketType type,
         uint16_t packetId);
  // PING, DISCONN
  explicit Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
                  MQTTPacketType type);

 private:
  // pass remainingLength = total size - header - remainingLengthLength!
  bool _allocate(size_t remainingLength);

  // fills header and returns index of next available byte in buffer
  size_t _fillPublishHeader(uint16_t packetId,
                            const char* topic,
                            size_t remainingLength,
                            uint8_t qos,
                            bool retain);


  size_t _chunkedAvailable(size_t index);
  const uint8_t* _chunkedData(size_t index) const;
};

}  // end namespace espMqttClientInternals
