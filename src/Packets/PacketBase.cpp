/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "PacketBase.h"

namespace espMqttClientInternals {

PacketBase::PacketBase(uint16_t packetId)
: _packetId(packetId)
, _data(nullptr)
, _size(0)
, _payloadIndex(0)
, _payloadStartIndex(0)
, _payloadEndIndex(0)
, _getPayload(nullptr) {
  // empty
}

PacketBase::~PacketBase() {
  free(_data);
}

size_t PacketBase::available(size_t index) {
  if (index >= _size) return 0;
  if (!_getPayload) return _size - index;
  return _chunkedAvailable(index);
}

const uint8_t* PacketBase::data(size_t index) const {
  if (!_getPayload) {
    if (!_data) return nullptr;
    if (index >= _size) return nullptr;
    return &_data[index];
  }
  return _chunkedData(index);
}

size_t PacketBase::size() const {
  return _size;
}

void PacketBase::setDup() {
  if (!_data) return;
  if (packetType() != PacketType.PUBLISH) return;
  if (_packetId == 0) return;
  _data[0] |= 0x08;
}

uint16_t PacketBase::packetId() const {
  return _packetId;
}

MQTTPacketType PacketBase::packetType() const {
  if (_data) return static_cast<MQTTPacketType>(_data[0] & 0xF0);
  return static_cast<MQTTPacketType>(0);
}

bool PacketBase::removable() const {
  if (_packetId == 0) return true;
  if ((packetType() == PacketType.PUBACK) || (packetType() == PacketType.PUBCOMP)) return true;
  return false;
}

bool PacketBase::_allocate(size_t remainingLength) {
  if (EMC_GET_FREE_MEMORY() < EMC_MIN_FREE_MEMORY) {
    emc_log_w("Packet buffer not allocated: low memory");
    return false;
  }
  _size = 1 + remainingLengthLength(remainingLength) + remainingLength;
  _data = reinterpret_cast<uint8_t*>(malloc(_size));
  if (!_data) {
    _size = 0;
    emc_log_w("Alloc failed (l:%zu)", _size);
    return false;
  }
  emc_log_i("Alloc (l:%zu)", _size);
  memset(_data, 0, _size);
  return true;
}

size_t PacketBase::_chunkedAvailable(size_t index) {
  // index vs size check done in 'available(index)'

  // index points to header or first payload byte
  if (index < _payloadIndex) {
    if (_size > _payloadIndex && _payloadEndIndex != 0) {
      size_t copied = _getPayload(&_data[_payloadIndex], std::min(static_cast<size_t>(EMC_TX_BUFFER_SIZE), _size - _payloadStartIndex), index);
      _payloadStartIndex = _payloadIndex;
      _payloadEndIndex = _payloadStartIndex + copied - 1;
    }

  // index points to payload unavailable
  } else if (index > _payloadEndIndex || _payloadStartIndex > index) {
    _payloadStartIndex = index;
    size_t copied = _getPayload(&_data[_payloadIndex], std::min(static_cast<size_t>(EMC_TX_BUFFER_SIZE), _size - _payloadStartIndex), index);
    _payloadEndIndex = _payloadStartIndex + copied - 1;
  }

  // now index points to header or payload available
  return _payloadEndIndex - index + 1;
}

const uint8_t* PacketBase::_chunkedData(size_t index) const {
  // CAUTION!! available(index) has to be called first to check available data and possibly fill payloadbuffer
  if (index < _payloadIndex) {
    return &_data[index];
  }
  return &_data[index - _payloadStartIndex + _payloadIndex];
}

}  // end namespace espMqttClientInternals
