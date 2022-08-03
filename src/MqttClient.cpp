/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClient.h"


#if defined(ARDUINO_ARCH_ESP32)
espMqttClient::espMqttClient(uint8_t priority, uint8_t core)
: MqttClientSetup(priority, core)
, _client() {
#else
espMqttClient::espMqttClient()
: _client() {
#endif
  _transport = &_client;
  _onConnectHook = reinterpret_cast<MqttClient::OnConnectHook>(_setupClient);
  _onConnectHookArg = this;
}

void espMqttClient::_setupClient(espMqttClient* c) {
  c->_client.setNoDelay(true);
  #if defined(ARDUINO_ARCH_ESP8266)
  c->_client.setSync(false);
  #endif
}

#if defined(ARDUINO_ARCH_ESP32)
espMqttClientSecure::espMqttClientSecure(uint8_t priority, uint8_t core)
: MqttClientSetup(priority, core)
, _client() {
#else
espMqttClientSecure::espMqttClientSecure()
: _client() {
#endif
  _transport = &_client;
  _onConnectHook = reinterpret_cast<MqttClient::OnConnectHook>(_setupClient);
  _onConnectHookArg = this;
}

void espMqttClientSecure::_setupClient(espMqttClientSecure* c) {
  c->_client.setNoDelay(true);
  #if defined(ARDUINO_ARCH_ESP8266)
  c->_client.setSync(false);
  #endif
}

espMqttClientSecure& espMqttClientSecure::setInsecure() {
  _client.setInsecure();
  return *this;
}

#if defined(ARDUINO_ARCH_ESP32)
espMqttClientSecure& espMqttClientSecure::setCACert(const char* rootCA) {
  _client.setCACert(rootCA);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setCertificate(const char* clientCa) {
  _client.setCertificate(clientCa);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setPrivateKey(const char* privateKey) {
  _client.setPrivateKey(privateKey);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setPreSharedKey(const char* pskIdent, const char* psKey) {
  _client.setPreSharedKey(pskIdent, psKey);
  return *this;
}
#elif defined(ARDUINO_ARCH_ESP8266)
espMqttClientSecure& espMqttClientSecure::setFingerprint(const uint8_t fingerprint[20]) {
  _client.setFingerprint(fingerprint);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setTrustAnchors(const X509List *ta) {
  _client.setTrustAnchors(ta);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setClientRSACert(const X509List *cert, const PrivateKey *sk) {
  _client.setClientRSACert(cert, sk);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setClientECCert(const X509List *cert, const PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type) {
  _client.setClientECCert(cert, sk, allowed_usages, cert_issuer_key_type);
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setCertStore(CertStoreBase *certStore) {
  _client.setCertStore(certStore);
  return *this;
}
#endif

uint16_t MqttClient::_getNextPacketId() {
  uint16_t packetId = 0;
  EMC_SEMAPHORE_TAKE();
  // cppcheck-suppress knownConditionTrueFalse
  packetId = (++_packetId == 0) ? ++_packetId : _packetId;
  EMC_SEMAPHORE_GIVE();
  return packetId;
}

void MqttClient::_checkOutgoing() {
  EMC_SEMAPHORE_TAKE();
  Packet* packet = _outbox.getCurrent();

  int32_t wantToWrite = 0;
  int32_t written = 0;
  while (packet && (wantToWrite == written)) {
    // mixing signed with unsigned here but safe because of MQTT packet size limits
    wantToWrite = packet->available(_bytesSent);
    written = _transport->write(packet->data(_bytesSent), wantToWrite);
    if (written < 0) {
      emc_log_w("Write error, check connection");
      break;
    }
    _lastClientActivity = millis();
    _bytesSent += written;
    emc_log_i("tx %zu/%zu", _bytesSent, packet->size());
    if (_bytesSent == packet->size()) {
      if ((packet->packetType()) == PacketType.DISCONNECT) _state = State::disconnectingTcp;
      if (packet->removable()) {
        _outbox.removeCurrent();
      } else {
        // handle with care! millis() returns unsigned 32 bit, token is void*
        packet->token = reinterpret_cast<void*>(millis());
        if ((packet->packetType()) == PacketType.PUBLISH) packet->setDup();
        _outbox.next();
      }
      packet = _outbox.getCurrent();
      _bytesSent = 0;
    }
  }
  EMC_SEMAPHORE_GIVE();
}

void MqttClient::_checkIncoming() {
  int32_t remainingBufferLength = _transport->read(_rxBuffer, EMC_RX_BUFFER_SIZE);
  if (remainingBufferLength > 0) {
    _lastServerActivity = millis();
    emc_log_i("rx len %i", remainingBufferLength);
    size_t bytesParsed = 0;
    size_t index = 0;
    while (remainingBufferLength > 0) {
      espMqttClientInternals::ParserResult result = _parser.parse(&_rxBuffer[index], remainingBufferLength, &bytesParsed);
      if (result == espMqttClientInternals::ParserResult::packet) {
        espMqttClientInternals::MQTTPacketType packetType = _parser.getPacket().fixedHeader.packetType & 0xF0;
        if (_state == State::connectingMqtt && packetType != PacketType.CONNACK) {
          emc_log_w("Disconnecting, expected CONNACK - protocol error");
          _state = State::disconnectingTcp;
          return;
        }
        switch (packetType & 0xF0) {
          case PacketType.CONNACK:
            _onConnack();
            if (_state != State::connected) {
              return;
            }
            break;
          case PacketType.PUBLISH:
            if (_state == State::disconnectingMqtt1 || _state == State::disconnectingMqtt2) break;  // stop processing incoming once user has called disconnect
            _onPublish();
            break;
          case PacketType.PUBACK:
            _onPuback();
            break;
          case PacketType.PUBREC:
            _onPubrec();
            break;
          case PacketType.PUBREL:
            _onPubrel();
            break;
          case PacketType.PUBCOMP:
            _onPubcomp();
            break;
          case PacketType.SUBACK:
            _onSuback();
            break;
          case PacketType.UNSUBACK:
            _onUnsuback();
            break;
          case PacketType.PINGRESP:
            // nothing to do
            break;
        }
      } else if (result ==  espMqttClientInternals::ParserResult::protocolError) {
        emc_log_w("Disconnecting, protocol error");
        _state = State::disconnectingTcp;
        return;
      }
      remainingBufferLength -= bytesParsed;
      index += bytesParsed;
      emc_log_i("Parsed %zu - remaining %i", bytesParsed, remainingBufferLength);
      bytesParsed = 0;
    }
  }
}

void MqttClient::_checkPing() {
  if (_keepAlive == 0) return;  // keepalive is disabled

  uint32_t currentMillis = millis();
  // disconnect when server was inactive for twice the keepalive time
  if (currentMillis - _lastServerActivity > 2000 * _keepAlive) {
    emc_log_w("Disconnecting, server exceeded keepalive");
    _state = State::disconnectingTcp;
    return;
  }

  // send ping when client was inactive for 0.7 times the keepalive time
  // or when server hasn't responded within keepalive time (typically due to QOS 0)
  if ((currentMillis - _lastClientActivity > 700 * _keepAlive) ||
      (currentMillis - _lastServerActivity > 1000 * _keepAlive)) {
    emc_log_i("Near keepalive, sending PING");
    if (!_addPacket(PacketType.PINGREQ)) {
      emc_log_e("Could not create PING packet");
    }
  }
}

void MqttClient::_onConnack() {
  if (_parser.getPacket().variableHeader.fixed.connackVarHeader.returnCode == 0x00) {
    _state = State::connected;
    if (_parser.getPacket().variableHeader.fixed.connackVarHeader.sessionPresent == 0) {
      _clearQueue(true);
    }
    if (_onConnectCallback) {
      _onConnectCallback(_parser.getPacket().variableHeader.fixed.connackVarHeader.sessionPresent);
    }
  } else {
    _state = State::disconnectingTcp;
    // cast is safe because the parser already checked for a valid return code
    _disconnectReason = static_cast<DisconnectReason>(_parser.getPacket().variableHeader.fixed.connackVarHeader.returnCode);
  }
}

void MqttClient::_onPublish() {
  espMqttClientInternals::IncomingPacket p = _parser.getPacket();
  uint8_t qos = p.qos();
  bool retain = p.retain();
  bool dup = p.dup();
  uint16_t packetId = p.variableHeader.fixed.packetId;
  bool callback = true;
  if (qos == 1) {
    if (p.payload.index + p.payload.length == p.payload.total) {
      EMC_SEMAPHORE_TAKE();
      if (!_addPacket(PacketType.PUBACK, packetId)) {
        emc_log_e("Could not create PUBACK packet");
      }
      EMC_SEMAPHORE_GIVE();
    }
  } else if (qos == 2) {
    EMC_SEMAPHORE_TAKE();
    espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
    while (it) {
      if ((it.get()->packetType()) == PacketType.PUBREC && it.get()->packetId() == packetId) {
        callback = false;
        emc_log_e("QoS2 packet previously delivered");
        break;
      }
      ++it;
    }
    if (p.payload.index + p.payload.length == p.payload.total) {
      if (!_addPacket(PacketType.PUBREC, packetId)) {
        emc_log_e("Could not create PUBREC packet");
      }
    }
    EMC_SEMAPHORE_GIVE();
  }
  if (callback && _onMessageCallback) _onMessageCallback({qos, dup, retain, packetId},
                                                         p.variableHeader.topic,
                                                         p.payload.data,
                                                         p.payload.length,
                                                         p.payload.index,
                                                         p.payload.total);
}

void MqttClient::_onPuback() {
  bool callback = false;
  uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  while (it) {
    // PUBACKs come in the order PUBs are sent. So we only check the first PUB packet in outbox
    // if it doesn't match the ID, return
    if ((it.get()->packetType()) == PacketType.PUBLISH) {
      if (it.get()->packetId() == idToMatch) {
        callback = true;
        _outbox.remove(it);
        break;
      }
      emc_log_w("Received out of order PUBACK");
      break;
    }
    ++it;
  }
  EMC_SEMAPHORE_GIVE();
  if (callback) {
    if (_onPublishCallback) _onPublishCallback(idToMatch);
  } else {
    emc_log_w("No matching PUBLISH packet found");
  }
}

void MqttClient::_onPubrec() {
  bool success = false;
  uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  while (it) {
    // PUBRECs come in the order PUBs are sent. So we only check the first PUB packet in outbox
    // if it doesn't match the ID, return
    if ((it.get()->packetType()) == PacketType.PUBLISH) {
      if (it.get()->packetId() == idToMatch) {
        if (!_addPacket(PacketType.PUBREL, idToMatch)) {
          emc_log_e("Could not create PUBREL packet");
        }
        _outbox.remove(it);
        success = true;
        break;
      }
      emc_log_w("Received out of order PUBREC");
      break;
    }
    ++it;
  }
  if (!success) {
    emc_log_w("No matching PUBLISH packet found");
  }
  EMC_SEMAPHORE_GIVE();
}

void MqttClient::_onPubrel() {
  bool success = false;
  uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  while (it) {
    // PUBRELs come in the order PUBRECs are sent. So we only check the first PUBREC packet in outbox
    // if it doesn't match the ID, return
    if ((it.get()->packetType()) == PacketType.PUBREC) {
      if (it.get()->packetId() == idToMatch) {
        if (!_addPacket(PacketType.PUBCOMP, idToMatch)) {
          emc_log_e("Could not create PUBCOMP packet");
        }
        _outbox.remove(it);
        success = true;
        break;
      }
      emc_log_w("Received out of order PUBREL");
      break;
    }
    ++it;
  }
  if (!success) {
    emc_log_w("No matching PUBREC packet found");
  }
  EMC_SEMAPHORE_GIVE();
}

void MqttClient::_onPubcomp() {
  bool callback = false;
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
  while (it) {
    // PUBCOMPs come in the order PUBRELs are sent. So we only check the first PUBREL packet in outbox
    // if it doesn't match the ID, return
    if ((it.get()->packetType()) == PacketType.PUBREL) {
      if (it.get()->packetId() == idToMatch) {
        if (!_addPacket(PacketType.PUBCOMP, idToMatch)) {
          emc_log_e("Could not create PUBCOMP packet");
        }
        callback = true;
        _outbox.remove(it);
        break;
      }
      emc_log_w("Received out of order PUBCOMP");
      break;
    }
    ++it;
  }
  EMC_SEMAPHORE_GIVE();
  if (callback) {
    if (_onPublishCallback) _onPublishCallback(idToMatch);
  } else {
    emc_log_w("No matching PUBREL packet found");
  }
}

void MqttClient::_onSuback() {
  bool callback = false;
  uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  while (it) {
    if (((it.get()->packetType()) == PacketType.SUBSCRIBE) && it.get()->packetId() == idToMatch) {
      callback = true;
      _outbox.remove(it);
      break;
    }
    ++it;
  }
  EMC_SEMAPHORE_GIVE();
  if (callback) {
    if (_onSubscribeCallback) _onSubscribeCallback(idToMatch, reinterpret_cast<const espMqttClientTypes::SubscribeReturncode*>(_parser.getPacket().payload.data), _parser.getPacket().payload.total);
  } else {
    emc_log_w("received SUBACK without SUB");
  }
}

void MqttClient::_onUnsuback() {
  bool callback = false;
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
  while (it) {
    if (it.get()->packetId() == idToMatch) {
      callback = true;
      _outbox.remove(it);
      break;
    }
    ++it;
  }
  EMC_SEMAPHORE_GIVE();
  if (callback) {
    if (_onUnsubscribeCallback) _onUnsubscribeCallback(idToMatch);
  } else {
    emc_log_w("received UNSUBACK without UNSUB");
  }
}

void MqttClient::_clearQueue(bool clearSession) {
  emc_log_i("clearing queue (clear session: %s)", clearSession ? "true" : "false");
  EMC_SEMAPHORE_TAKE();
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.front();
  if (clearSession) {
    while (it) {
      _outbox.remove(it);
    }
  } else {
    // keep PUB (qos > 0, aka packetID != 0), PUBREC and PUBREL
    // Spec only mentions PUB and PUBREL but this lib implements method B from point 4.3.3 (Fig. 4.3)
    // and stores the packet id in the PUBREC packet. So we also must keep PUBREC.
    while (it) {
      espMqttClientInternals::MQTTPacketType type = it.get()->packetType();
      if (type == PacketType.PUBREC ||
          type == PacketType.PUBREL ||
          (type == PacketType.PUBLISH && it.get()->packetId() != 0)) {
        ++it;
      } else {
        _outbox.remove(it);
      }
    }
  }
  EMC_SEMAPHORE_GIVE();
}

void MqttClient::_onError(uint16_t packetId, espMqttClientTypes::Error error) {
  if (_onErrorCallback) {
    _onErrorCallback(packetId, error);
  }
}
