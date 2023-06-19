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
#include "../Config.h"
#include "../TypeDefs.h"
#include "../Helpers.h"
#include "../Logging.h"
#include "RemainingLength.h"
#include "String.h"

namespace espMqttClientInternals {

class PacketBase {
 public:
  explicit PacketBase(uint16_t packetId);
  ~PacketBase();
  size_t available(size_t index);
  const uint8_t* data(size_t index) const;

  size_t size() const;
  void setDup();
  uint16_t packetId() const;
  MQTTPacketType packetType() const;
  bool removable() const;

 protected:
  uint16_t _packetId;  // save as separate variable: will be accessed frequently
  uint8_t* _data;
  size_t _size;

  // variables for chunked payload handling
  size_t _payloadIndex;
  size_t _payloadStartIndex;
  size_t _payloadEndIndex;
  espMqttClientTypes::PayloadCallback _getPayload;

  // pass remainingLength = total size - header - remainingLengthLength!
  bool _allocate(size_t remainingLength);
  size_t _chunkedAvailable(size_t index);
  const uint8_t* _chunkedData(size_t index) const;
};

}  // end namespace espMqttClientInternals
