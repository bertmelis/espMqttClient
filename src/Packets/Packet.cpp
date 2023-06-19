/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Packet.h"

namespace espMqttClientInternals {

Packet::Packet(espMqttClientTypes::Error& error,
               bool cleanSession,
               const char* username,
               const char* password,
               const char* willTopic,
               bool willRetain,
               uint8_t willQos,
               const uint8_t* willPayload,
               uint16_t willPayloadLength,
               uint16_t keepAlive,
               const char* clientId)
: PacketBase(0) {
  if (willPayload && willPayloadLength == 0) {
    size_t length = strlen(reinterpret_cast<const char*>(willPayload));
    if (length > UINT16_MAX) {
      emc_log_w("Payload length truncated (l:%zu)", length);
      willPayloadLength = UINT16_MAX;
    } else {
      willPayloadLength = length;
    }
  }
  if (!clientId || strlen(clientId) == 0) {
    emc_log_w("clientId not set error");
    error = espMqttClientTypes::Error::MALFORMED_PARAMETER;
    return;
  }

  // Calculate size
  size_t remainingLength =
  6 +  // protocol
  1 +  // protocol level
  1 +  // connect flags
  2 +  // keepalive
  2 + strlen(clientId) +
  (willTopic ? 2 + strlen(willTopic) + 2 + willPayloadLength : 0) +
  (username ? 2 + strlen(username) : 0) +
  (password ? 2 + strlen(password) : 0);

  // allocate memory
  if (!_allocate(remainingLength)) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }

  // serialize
  size_t pos = 0;

  // FIXED HEADER
  _data[pos++] = PacketType.CONNECT | HeaderFlag.CONNECT_RESERVED;
  pos += encodeRemainingLength(remainingLength, &_data[pos]);
  pos += encodeString(PROTOCOL, &_data[pos]);
  _data[pos++] = PROTOCOL_LEVEL;
  uint8_t connectFlags = 0;
  if (cleanSession) connectFlags |= espMqttClientInternals::ConnectFlag.CLEAN_SESSION;
  if (username != nullptr) connectFlags |= espMqttClientInternals::ConnectFlag.USERNAME;
  if (password != nullptr) connectFlags |= espMqttClientInternals::ConnectFlag.PASSWORD;
  if (willTopic != nullptr) {
    connectFlags |= espMqttClientInternals::ConnectFlag.WILL;
    if (willRetain) connectFlags |= espMqttClientInternals::ConnectFlag.WILL_RETAIN;
    switch (willQos) {
      case 0:
        connectFlags |= espMqttClientInternals::ConnectFlag.WILL_QOS0;
        break;
      case 1:
        connectFlags |= espMqttClientInternals::ConnectFlag.WILL_QOS1;
        break;
      case 2:
        connectFlags |= espMqttClientInternals::ConnectFlag.WILL_QOS2;
        break;
    }
  }
  _data[pos++] = connectFlags;
  _data[pos++] = keepAlive >> 8;
  _data[pos++] = keepAlive & 0xFF;

  // PAYLOAD
  // client ID
  pos += encodeString(clientId, &_data[pos]);
  // will
  if (willTopic != nullptr && willPayload != nullptr) {
    pos += encodeString(willTopic, &_data[pos]);
    _data[pos++] = willPayloadLength >> 8;
    _data[pos++] = willPayloadLength & 0xFF;
    memcpy(&_data[pos], willPayload, willPayloadLength);
    pos += willPayloadLength;
  }
  // credentials
  if (username != nullptr) pos += encodeString(username, &_data[pos]);
  if (password != nullptr) encodeString(password, &_data[pos]);

  error = espMqttClientTypes::Error::SUCCESS;
}

Packet::Packet(espMqttClientTypes::Error& error,
               uint16_t packetId,
               const char* topic,
               const uint8_t* payload,
               size_t payloadLength,
               uint8_t qos,
               bool retain)
: PacketBase(packetId) {
  size_t remainingLength =
    2 + strlen(topic) +  // topic length + topic
    2 +                  // packet ID
    payloadLength;

  if (qos == 0) {
    remainingLength -= 2;
    _packetId = 0;
  }

  if (!_allocate(remainingLength)) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }

  size_t pos = _fillPublishHeader(packetId, topic, remainingLength, qos, retain);

  // PAYLOAD
  memcpy(&_data[pos], payload, payloadLength);

  error = espMqttClientTypes::Error::SUCCESS;
}

Packet::Packet(espMqttClientTypes::Error& error,
               uint16_t packetId,
               const char* topic,
               espMqttClientTypes::PayloadCallback payloadCallback,
               size_t payloadLength,
               uint8_t qos,
               bool retain)
: PacketBase(packetId) {
  _getPayload = payloadCallback;
  size_t remainingLength =
    2 + strlen(topic) +  // topic length + topic
    2 +                  // packet ID
    payloadLength;

  if (qos == 0) {
    remainingLength -= 2;
    _packetId = 0;
  }

  if (!_allocate(remainingLength - payloadLength + std::min(payloadLength, static_cast<size_t>(EMC_RX_BUFFER_SIZE)))) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }

  size_t pos = _fillPublishHeader(packetId, topic, remainingLength, qos, retain);

  // payload will be added by 'Packet::available'
  _size = pos + payloadLength;
  _payloadIndex = pos;
  _payloadStartIndex = _payloadIndex;
  _payloadEndIndex = _payloadIndex;

  error = espMqttClientTypes::Error::SUCCESS;
}

Packet::Packet(espMqttClientTypes::Error& error, uint16_t packetId, const char* topic, uint8_t qos)
: PacketBase(packetId) {
  SubscribeItem list[1] = {topic, qos};
  _createSubscribe(error, list, 1);
}

Packet::Packet(espMqttClientTypes::Error& error, MQTTPacketType type, uint16_t packetId)
: PacketBase(packetId) {
  if (!_allocate(2)) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }

  size_t pos = 0;
  _data[pos] = type;
  if (type == PacketType.PUBREL) {
    _data[pos++] |= HeaderFlag.PUBREL_RESERVED;
  } else {
    pos++;
  }
  pos += encodeRemainingLength(2, &_data[pos]);
  _data[pos++] = packetId >> 8;
  _data[pos] = packetId & 0xFF;

  error = espMqttClientTypes::Error::SUCCESS;
}

Packet::Packet(espMqttClientTypes::Error& error, uint16_t packetId, const char* topic)
: PacketBase(packetId) {
  const char* list[1] = {topic};
  _createUnsubscribe(error, list, 1);
}

Packet::Packet(espMqttClientTypes::Error& error, MQTTPacketType type)
: PacketBase(0) {
  if (!_allocate(0)) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }
  _data[0] |= type;

  error = espMqttClientTypes::Error::SUCCESS;
}

size_t Packet::_fillPublishHeader(uint16_t packetId,
                                  const char* topic,
                                  size_t remainingLength,
                                  uint8_t qos,
                                  bool retain) {
  size_t index = 0;

  // FIXED HEADER
  _data[index] = PacketType.PUBLISH;
  if (retain) _data[index] |= HeaderFlag.PUBLISH_RETAIN;
  if (qos == 0) {
    _data[index++] |= HeaderFlag.PUBLISH_QOS0;
  } else if (qos == 1) {
    _data[index++] |= HeaderFlag.PUBLISH_QOS1;
  } else if (qos == 2) {
    _data[index++] |= HeaderFlag.PUBLISH_QOS2;
  }
  index += encodeRemainingLength(remainingLength, &_data[index]);

  // VARIABLE HEADER
  index += encodeString(topic, &_data[index]);
  if (qos > 0) {
    _data[index++] = packetId >> 8;
    _data[index++] = packetId & 0xFF;
  }

  return index;
}

void Packet::_createSubscribe(espMqttClientTypes::Error& error,
                              SubscribeItem* list,
                              size_t numberTopics) {
  // Calculate size
  size_t payload = 0;
  for (size_t i = 0; i < numberTopics; ++i) {
    payload += 2 + strlen(list[i].topic) + 1;  // length bytes, string, qos
  }
  size_t remainingLength = 2 + payload;  // packetId + payload

  // allocate memory
  if (!_allocate(remainingLength)) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }

  // serialize
  size_t pos = 0;
  _data[pos++] = PacketType.SUBSCRIBE | HeaderFlag.SUBSCRIBE_RESERVED;
  pos += encodeRemainingLength(remainingLength, &_data[pos]);
  _data[pos++] = _packetId >> 8;
  _data[pos++] = _packetId & 0xFF;
  for (size_t i = 0; i < numberTopics; ++i) {
    pos += encodeString(list[i].topic, &_data[pos]);
    _data[pos++] = list[i].qos;
  }

  error = espMqttClientTypes::Error::SUCCESS;
}

void Packet::_createUnsubscribe(espMqttClientTypes::Error& error,
                                const char** list,
                                size_t numberTopics) {
  // Calculate size
  size_t payload = 0;
  for (size_t i = 0; i < numberTopics; ++i) {
    payload += 2 + strlen(list[i]);  // length bytes, string
  }
  size_t remainingLength = 2 + payload;  // packetId + payload

  // allocate memory
  if (!_allocate(remainingLength)) {
    error = espMqttClientTypes::Error::OUT_OF_MEMORY;
    return;
  }

  // serialize
  size_t pos = 0;
  _data[pos++] = PacketType.UNSUBSCRIBE | HeaderFlag.UNSUBSCRIBE_RESERVED;
  pos += encodeRemainingLength(remainingLength, &_data[pos]);
  _data[pos++] = _packetId >> 8;
  _data[pos++] = _packetId & 0xFF;
  for (size_t i = 0; i < numberTopics; ++i) {
    pos += encodeString(list[i], &_data[pos]);
  }

  error = espMqttClientTypes::Error::SUCCESS;
}

}  // end namespace espMqttClientInternals
