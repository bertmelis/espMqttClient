/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Packet.h"

namespace espMqttClientInternals {

Packet::~Packet() {
  delete[] _data;
}

Packet::Packet(const Packet& p)
: token(p.token)
, _data(nullptr)
, _size(p._size)
, _packetId(p._packetId) {
  _data = new uint8_t[_size];
  memcpy(_data, p._data, _size);
}

Packet::Packet(Packet&& p)
: token(p.token)
, _data(nullptr)
, _size(p._size)
, _packetId(p._packetId) {
  _data = p._data;
  p._data = nullptr;
  p._size = 0;
}

const uint8_t* Packet::data(size_t index) const {
  if (!_data) return nullptr;
  if (index >= _size) return nullptr;
  return &_data[index];
}

size_t Packet::size() const {
  return _size;
}

void Packet::setDup() {
  if (!data) return;
  _data[0] |= 0x08;
}

uint16_t Packet::packetId() const {
  return _packetId;
}

Packet::Packet(bool cleanSession,
               const char* username,
               const char* password,
               const char* willTopic,
               bool willRetain,
               uint8_t willQos,
               const uint8_t* willPayload,
               uint16_t willPayloadLength,
               uint16_t keepAlive,
               const char* clientId)
: token(nullptr)
, _data(nullptr)
, _size(0)
, _packetId(0) {
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
  if (!_allocate(remainingLength)) return;

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
  if (willTopic != nullptr) {
    pos += encodeString(willTopic, &_data[pos]);
    if (willPayload != nullptr && willPayloadLength == 0) {
      willPayloadLength = strlen(reinterpret_cast<const char*>(willPayload));
    }
    _data[pos++] = willPayloadLength >> 8;
    _data[pos++] = willPayloadLength & 0xFF;
    memcpy(&_data[pos], willPayload, willPayloadLength);
    pos += willPayloadLength;
  }
  // credentials
  if (username != nullptr) pos += encodeString(username, &_data[pos]);
  if (password != nullptr) pos += encodeString(password, &_data[pos]);
}

Packet::Packet(const char* topic,
               const uint8_t* payload,
               size_t payloadLength,
               uint8_t qos,
               bool retain,
               uint16_t packetId)
: token(nullptr)
, _data(nullptr)
, _size(0)
, _packetId(packetId) {
  size_t remainingLength =
    2 + strlen(topic) +  // topic length + topic
    2 +                  // packet ID
    payloadLength;

  if (qos == 0) {
    remainingLength -= 2;
    _packetId = 0;
  }

  if (!_allocate(remainingLength)) return;

    size_t pos = 0;

  // FIXED HEADER
  _data[pos] = PacketType.PUBLISH;
  if (retain) _data[pos] |= HeaderFlag.PUBLISH_RETAIN;
  if (qos == 0) {
    _data[pos++] |= HeaderFlag.PUBLISH_QOS0;
  } else if (qos == 1) {
    _data[pos++] |= HeaderFlag.PUBLISH_QOS1;
  } else if (qos == 2) {
    _data[pos++] |= HeaderFlag.PUBLISH_QOS2;
  }
  pos += encodeRemainingLength(remainingLength, &_data[pos]);

  // VARIABLE HEADER
  pos += encodeString(topic, &_data[pos]);
  if (qos > 0) {
    _data[pos++] = packetId >> 8;
    _data[pos++] = packetId & 0xFF;
  }

  // PAYLOAD
  memcpy(&_data[pos], payload, payloadLength);
}

Packet::Packet(const char* topic, uint8_t qos, uint16_t packetId)
: token(nullptr)
, _data(nullptr)
, _size(0)
, _packetId(packetId) {
  // Calculate size
  size_t remainingLength =
  2 +                  // packet ID
  2 + strlen(topic) +  // topic
  1;                   // qos

  // allocate memory
  if (!_allocate(remainingLength)) return;

  // serialize
  size_t pos = 0;
  _data[pos++] = PacketType.SUBSCRIBE | HeaderFlag.SUBSCRIBE_RESERVED;
  pos += encodeRemainingLength(remainingLength, &_data[pos]);
  _data[pos++] = packetId >> 8;
  _data[pos++] = packetId & 0xFF;
  pos += encodeString(topic, &_data[pos]);
  _data[pos] = qos;
}

Packet::Packet(MQTTPacketType type, uint16_t packetId)
: token(nullptr)
, _data(nullptr)
, _size(0)
, _packetId(packetId) {
  if (!_allocate(2)) return;

  size_t pos = 0;
  _data[pos] = type;
  if (type == PacketType.PUBREL) {
    _data[pos++] |= HeaderFlag.PUBREL_RESERVED;
  } else {
    pos++;
  }
  pos += encodeRemainingLength(2, &_data[pos]);
  _data[pos++] = packetId >> 8;
  _data[pos++] = packetId & 0xFF;
}

Packet::Packet(const char* topic, uint16_t packetId)
: token(nullptr)
, _data(nullptr)
, _size(0) {
  // Calculate size
  size_t remainingLength =
  2 +                  // packet ID
  2 + strlen(topic);   // topic

  // allocate memory
  if (!_allocate(remainingLength)) return;

  // serialize
  size_t pos = 0;
  _data[pos++] = PacketType.UNSUBSCRIBE | HeaderFlag.UNSUBSCRIBE_RESERVED;
  pos += encodeRemainingLength(remainingLength, &_data[pos]);
  _data[pos++] = packetId >> 8;
  _data[pos++] = packetId & 0xFF;
  pos += encodeString(topic, &_data[pos]);
}

Packet::Packet(MQTTPacketType type)
: token(nullptr)
, _data(nullptr)
, _size(0)
, _packetId(0) {
  if (!_allocate(0)) return;
  _data[0] |= type;
}


bool Packet::_allocate(size_t remainingLength) {
  if (EMC_GET_FREE_MEMORY() < EMC_MIN_FREE_MEMORY) {
    emc_log_w("Packet buffer not allocated: low memory");
    return false;
  }
  _size = 1 + remainingLengthLength(remainingLength) + remainingLength;
  _data = new uint8_t[_size];
  if (!_data) {
    _size = 0;
    emc_log_w("Alloc failed (l:%zu)", _size);
    return false;
  }
  emc_log_i("Alloc (l:%zu)", _size);
  memset(_data, 0, _size);
  return true;
}

}  // end namespace espMqttClientInternals
