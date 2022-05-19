/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Parser.h"

namespace espMqttClientInternals {

void IncomingPacket::reset() {
  fixedHeader.packetType = 0;
  variableHeader.topicLength = 0;
  variableHeader.fixed.packetId = 0;
  payload.index = 0;
  payload.length = 0;
}

Parser::Parser()
: _data(nullptr)
, _len(0)
, _bytesRead(0)
, _bytePos(0)
, _parse(_fixedHeader)
, _packet()
, _payloadBuffer{0} {
  // empty
}

ParserResult Parser::parse(const uint8_t* data, size_t len, size_t* bytesRead) {
  _data = data;
  _len = len;
  _bytesRead = 0;
  ParserResult result = AWAIT_DATA;
  while (result == AWAIT_DATA && _bytesRead < _len) {
    result = _parse(this);
    ++_bytesRead;
  }
  (*bytesRead) += _bytesRead;
  return result;
}

const IncomingPacket& Parser::getPacket() const {
  return _packet;
}

ParserResult Parser::_fixedHeader(Parser* p) {
  p->_packet.reset();
  p->_packet.fixedHeader.packetType = p->_data[p->_bytesRead];

  // keep PUBLISH out of the switch and handle in separate if/else
  if ((p->_packet.fixedHeader.packetType & 0xF0) == PacketType.PUBLISH) {
    uint8_t headerFlags = p->_packet.fixedHeader.packetType & 0x0F;
    /* flags can be: 0b0000 --> no dup, qos 0, no retain
                     0x0001 --> no dup, qos 0, retain
                     0x0010 --> no dup, qos 1, no retain
                     0x0011 --> no dup, qos 1, retain
                     0x0100 --> no dup, qos 2, no retain
                     0x0101 --> no dup, qos 2, retain
                     0x1010 --> dup, qos 1, no retain
                     0x1011 --> dup, qos 1, retain
                     0x1100 --> dup, qos 2, no retain
                     0x1101 --> dup, qos 2, retain
    */
    if (headerFlags <= 0x05 || headerFlags >= 0x0A) {
      p->_parse = _remainingLengthVariable;
      p->_bytePos = 0;
    } else {
      emc_log_w("Invalid packet header: 0x%02x", p->_packet.fixedHeader.packetType);
      emc_log_w("Invalid packet header");
      return PROTOCOL_ERROR;
    }
  } else {
    switch (p->_packet.fixedHeader.packetType) {
      case PacketType.CONNACK | HeaderFlag.CONNACK_RESERVED:
      case PacketType.PUBACK | HeaderFlag.PUBACK_RESERVED:
      case PacketType.PUBREC | HeaderFlag.PUBREC_RESERVED:
      case PacketType.PUBREL | HeaderFlag.PUBREL_RESERVED:
      case PacketType.PUBCOMP | HeaderFlag.PUBCOMP_RESERVED:
      case PacketType.UNSUBACK | HeaderFlag.UNSUBACK_RESERVED:
        p->_parse = _remainingLengthFixed;
        break;
      case PacketType.SUBACK | HeaderFlag.SUBACK_RESERVED:
        p->_parse = _remainingLengthVariable;
        p->_bytePos = 0;
        break;
      case PacketType.PINGRESP | HeaderFlag.PINGRESP_RESERVED:
        p->_parse = _remainingLengthNone;
        break;
      default:
        emc_log_w("Invalid packet header: 0x%02x", p->_packet.fixedHeader.packetType);
        return PROTOCOL_ERROR;
    }
  }
  emc_log_i("Packet type: 0x%02x", p->_packet.fixedHeader.packetType);
  return AWAIT_DATA;
}

ParserResult Parser::_remainingLengthFixed(Parser* p) {
  p->_packet.fixedHeader.remainingLength.remainingLength = p->_data[p->_bytesRead];

  if (p->_packet.fixedHeader.remainingLength.remainingLength == 2) {  // variable header is 2 bytes long
    if ((p->_packet.fixedHeader.packetType & 0xF0) != PacketType.CONNACK) {
      p->_parse = _varHeaderPacketId1;
    } else {
      p->_parse = _varHeaderConnack1;
    }
    emc_log_i("Remaining length: %lu", p->_packet.fixedHeader.remainingLength.remainingLength);
    return AWAIT_DATA;
  }
  p->_parse = _fixedHeader;
  emc_log_w("Invalid remaining length (fixed): %lu",  p->_packet.fixedHeader.remainingLength.remainingLength);
  return PROTOCOL_ERROR;
}

ParserResult Parser::_remainingLengthVariable(Parser* p) {
  p->_packet.fixedHeader.remainingLength.remainingLengthRaw[p->_bytePos] = p->_data[p->_bytesRead];
  if (p->_packet.fixedHeader.remainingLength.remainingLengthRaw[p->_bytePos] & 0x80) {
    p->_bytePos++;
    if (p->_bytePos == 4) {
      emc_log_w("Invalid remaining length (variable)");
      return PROTOCOL_ERROR;
    } else {
      return AWAIT_DATA;
    }
  }

  // no need to check for negative decoded length, check is already done
  p->_packet.fixedHeader.remainingLength.remainingLength = decodeRemainingLength(p->_packet.fixedHeader.remainingLength.remainingLengthRaw);

  if ((p->_packet.fixedHeader.packetType & 0xF0) == PacketType.PUBLISH) {
    p->_parse = _varHeaderTopicLength1;
    emc_log_i("Remaining length: %lu", p->_packet.fixedHeader.remainingLength.remainingLength);
    return AWAIT_DATA;
  } else {
    int32_t payloadSize = p->_packet.fixedHeader.remainingLength.remainingLength - 2;  // total - packet ID
    if (0 < payloadSize && payloadSize < EMC_PAYLOAD_BUFFER_SIZE) {
      p->_bytePos = 0;
      p->_packet.payload.data = p->_payloadBuffer;
      p->_packet.payload.index = 0;
      p->_packet.payload.length = payloadSize;
      p->_packet.payload.total = payloadSize;
      p->_parse = _varHeaderPacketId1;
      emc_log_i("Remaining length: %lu", p->_packet.fixedHeader.remainingLength.remainingLength);
      return AWAIT_DATA;
    } else {
      emc_log_w("Invalid payload length");
    }
  }
  p->_parse = _fixedHeader;
  return PROTOCOL_ERROR;
}

ParserResult Parser::_remainingLengthNone(Parser* p) {
  p->_packet.fixedHeader.remainingLength.remainingLength = p->_data[p->_bytesRead];
  p->_parse = _fixedHeader;
  if (p->_packet.fixedHeader.remainingLength.remainingLength == 0) {
    emc_log_i("Remaining length: %lu", p->_packet.fixedHeader.remainingLength.remainingLength);
    return PACKET;
  }
  emc_log_w("Invalid remaining length (none)");
  return PROTOCOL_ERROR;
}

ParserResult Parser::_varHeaderConnack1(Parser* p) {
  uint8_t data = p->_data[p->_bytesRead];
  if (data < 2) {  // session present flag: equal to 0 or 1
    p->_packet.variableHeader.fixed.connackVarHeader.sessionPresent = data;
    p->_parse = _varHeaderConnack2;
    return AWAIT_DATA;
  }
  p->_parse = _fixedHeader;
  emc_log_w("Invalid session flags");
  return PROTOCOL_ERROR;
}

ParserResult Parser::_varHeaderConnack2(Parser* p) {
  uint8_t data = p->_data[p->_bytesRead];
  p->_parse = _fixedHeader;
  if (data <= 5) {  // connect return code max is 5
    p->_packet.variableHeader.fixed.connackVarHeader.returnCode = data;
    emc_log_i("Packet complete");
    return PACKET;
  }
  emc_log_w("Invalid connack return code");
  return PROTOCOL_ERROR;
}

ParserResult Parser::_varHeaderPacketId1(Parser* p) {
  p->_packet.variableHeader.fixed.packetId |= p->_data[p->_bytesRead] << 8;
  p->_parse = _varHeaderPacketId2;
  return AWAIT_DATA;
}

ParserResult Parser::_varHeaderPacketId2(Parser* p) {
  p->_packet.variableHeader.fixed.packetId |= p->_data[p->_bytesRead];
  p->_parse = _fixedHeader;
  if (p->_packet.variableHeader.fixed.packetId != 0) {
    if ((p->_packet.fixedHeader.packetType & 0xF0) == PacketType.SUBACK) {
      p->_parse = _payloadSuback;
      emc_log_i("Packet variable header complete");
      return AWAIT_DATA;
    } else if ((p->_packet.fixedHeader.packetType & 0xF0) == PacketType.PUBLISH) {
      p->_packet.payload.total -= 2;  // substract packet id length from payload
      p->_parse = _payloadPublish;
      emc_log_i("Packet variable header complete");
      return AWAIT_DATA;
    } else {
      emc_log_i("Packet complete");
      return PACKET;
    }
  } else {
    emc_log_w("Invalid packet id");
    return PROTOCOL_ERROR;
  }
}

ParserResult Parser::_varHeaderTopicLength1(Parser* p) {
  p->_packet.variableHeader.topicLength = p->_data[p->_bytesRead] << 8;
  p->_parse = _varHeaderTopicLength2;
  return AWAIT_DATA;
}

ParserResult Parser::_varHeaderTopicLength2(Parser* p) {
  p->_packet.variableHeader.topicLength |= p->_data[p->_bytesRead];
  size_t maxTopicLength =
    p->_packet.fixedHeader.remainingLength.remainingLength
    - 2  // topic length bytes
    - (p->_packet.fixedHeader.packetType & (HeaderFlag.PUBLISH_QOS1 | HeaderFlag.PUBLISH_QOS2) ? 2 : 0);
  if (p->_packet.variableHeader.topicLength <= maxTopicLength) {
    p->_parse = _varHeaderTopic;
    p->_bytePos = 0;
    p->_packet.payload.total = p->_packet.fixedHeader.remainingLength.remainingLength - 2 - p->_packet.variableHeader.topicLength;
    return AWAIT_DATA;
  }
  emc_log_w("Invalid topic length: %u > %lu", p->_packet.variableHeader.topicLength, maxTopicLength);
  p->_parse = _fixedHeader;
  return PROTOCOL_ERROR;
}

ParserResult Parser::_varHeaderTopic(Parser* p) {
  // no checking for character [MQTT-3.3.2-1] [MQTT-3.3.2-2]
  p->_packet.variableHeader.topic[p->_bytePos] = static_cast<char>(p->_data[p->_bytesRead]);
  p->_bytePos++;
  if (p->_bytePos == p->_packet.variableHeader.topicLength || p->_bytePos == EMC_MAX_TOPIC_LENGTH) {
    p->_packet.variableHeader.topic[p->_bytePos] = 0x00;  // add c-string delimiter
    p->_parse = (p->_packet.fixedHeader.packetType & (HeaderFlag.PUBLISH_QOS1 | HeaderFlag.PUBLISH_QOS2) ? _varHeaderPacketId1 : _payloadPublish);
    emc_log_i("Packet variable header topic complete");
  }
  return AWAIT_DATA;
}

ParserResult Parser::_payloadSuback(Parser* p) {
  uint8_t data = p->_data[p->_bytesRead];
  if (data < 0x03 || data == 0x80) {
    p->_payloadBuffer[p->_bytePos] = data;
    p->_bytePos++;
  } else {
    p->_parse = _fixedHeader;
    emc_log_w("Invalid suback return code");
    return PROTOCOL_ERROR;
  }
  if (p->_bytePos == p->_packet.payload.total) {
    p->_parse = _fixedHeader;
    emc_log_i("Packet complete");
    return PACKET;
  }
  return AWAIT_DATA;
}

ParserResult Parser::_payloadPublish(Parser* p) {
  if (p->_packet.payload.index == 0) p->_packet.payload.index += p->_packet.payload.length;
  emc_log_i("payload: index %lu, total %lu, avail %lu/%lu", p->_packet.payload.index, p->_packet.payload.total, p->_len - p->_bytesRead, p->_len);
  p->_packet.payload.length = std::min(p->_len - p->_bytesRead, p->_packet.payload.total - p->_packet.payload.index);
  p->_bytesRead += p->_packet.payload.length - 1;  // compensate for increment in _parse-loop
  if (p->_packet.payload.index + p->_packet.payload.length == p->_packet.payload.total) {
    p->_parse = _fixedHeader;
  }
  return PACKET;
}

}  // end namespace espMqttClientInternals
