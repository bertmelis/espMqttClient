/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include "PacketBase.h"

namespace espMqttClientInternals {

class Packet : public PacketBase {
  struct SubscribeItem {
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
         const char* topic1,
         uint8_t qos1,
         const char* topic2,
         uint8_t qos2,
         Args&& ... args)
  : PacketBase(packetId) {
    static_assert(sizeof...(Args) % 2 == 0, "Subscribe should be in topic/qos pairs");
    size_t numberTopics = 2 + (sizeof...(Args) / 2);
    SubscribeItem list[numberTopics] = {topic1, qos1, topic2, qos2, args...};
    _createSubscribe(error, list, numberTopics);
  }
  // UNSUBSCRIBE
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic);
  template<typename ... Args>
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         uint16_t packetId,
         const char* topic1,
         const char* topic2,
         Args&& ... args)
  : PacketBase(packetId) {
    size_t numberTopics = 2 + sizeof...(Args);
    const char* list[numberTopics] = {topic1, topic2, args...};
    _createUnsubscribe(error, list, numberTopics);
  }
  // PUBACK, PUBREC, PUBREL, PUBCOMP
  Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
         MQTTPacketType type,
         uint16_t packetId);
  // PING, DISCONN
  explicit Packet(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
                  MQTTPacketType type);

 private:
  // fills header and returns index of next available byte in buffer
  size_t _fillPublishHeader(uint16_t packetId,
                            const char* topic,
                            size_t remainingLength,
                            uint8_t qos,
                            bool retain);
  void _createSubscribe(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
                        SubscribeItem* list,
                        size_t numberTopics);
  void _createUnsubscribe(espMqttClientTypes::Error& error,  // NOLINT(runtime/references)
                          const char** list,
                          size_t numberTopics);
};

}  // end namespace espMqttClientInternals
