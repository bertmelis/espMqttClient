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
  Packet();

  uint8_t* _data;
  size_t _size;
  uint16_t _packetId;  // save as separate variable: will be accessed frequently

  // variables for chunked payload handling
  size_t _payloadIndex;
  size_t _payloadStartIndex;
  size_t _payloadEndIndex;
  espMqttClientTypes::PayloadCallback _getPayload;

 public:
  // CONNECT
  Packet(bool cleanSession,
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
  Packet(const char* topic,
         const uint8_t* payload,
         size_t payloadLength,
         uint8_t qos,
         bool retain,
         uint16_t packetId);
  Packet(const char* topic,
         espMqttClientTypes::PayloadCallback payloadCallback,
         size_t payloadLength,
         uint8_t qos,
         bool retain,
         uint16_t packetId);
  // SUBSCRIBE
  Packet(const char* topic,
         uint8_t qos,
         uint16_t packetId);
  // UNSUBSCRIBE
  Packet(const char* topic,
         uint16_t packetId);
  // PUBACK, PUBREC, PUBREL, PUBCOMP
  Packet(MQTTPacketType type,
         uint16_t packetId);
  // PING, DISCONN
  explicit Packet(MQTTPacketType type);

 private:
  // pass remainingLength = total size - header - remainingLengthLength!
  bool _allocate(size_t remainingLength);

  // fills header and returns index of next available byte in buffer
  size_t _fillPublishHeader(const char* topic,
                            size_t remainingLength,
                            uint8_t qos,
                            bool retain,
                            uint16_t packetId);

  size_t _chunkedAvailable(size_t index);
  const uint8_t* _chunkedData(size_t index) const;
};

}  // end namespace espMqttClientInternals
