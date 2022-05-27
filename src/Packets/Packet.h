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
#include "../Helpers.h"
#include "RemainingLength.h"
#include "String.h"

namespace espMqttClientInternals {

class Packet {
 public:
  virtual ~Packet();

  /*
  Packet(const Packet&);
  Packet(Packet&&);
  */

  const uint8_t* data(size_t index) const;
  size_t size() const;
  void setDup();
  uint16_t packetId() const;

  void* token;  // native typeless variable to store any additional data

 protected:
  uint8_t* _data;
  size_t _size;
  uint16_t _packetId;  // save as separate variable: will be accessed frequently

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
};

}  // end namespace espMqttClientInternals
