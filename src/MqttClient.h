/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <atomic>
#include <utility>

#include "Helpers.h"
#include "Config.h"
#include "TypeDefs.h"
#include "Logging.h"
#include "Outbox.h"
#include "Packets/Constants.h"
#include "Transport/Transport.h"

template <class TPacket, class TParser>
class MqttClient {
 public:
  virtual ~MqttClient() {
    disconnect(true);
    _clearQueue(2);
    #if defined(ARDUINO_ARCH_ESP32)
    vSemaphoreDelete(_xSemaphore);
    if (_useInternalTask == espMqttClientTypes::UseInternalTask::YES) {
      #if EMC_USE_WATCHDOG
      esp_task_wdt_delete(_taskHandle);  // not sure if this is really needed
      #endif
      vTaskDelete(_taskHandle);
    }
    #endif
  }

  bool connected() const {
    if (_state == State::connected) return true;
    return false;
  }

  bool disconnected() const {
    if (_state == State::disconnected) return true;
    return false;
  }

  bool connect() {
    bool result = true;
    if (_state == State::disconnected) {
      EMC_SEMAPHORE_TAKE();
      if (_addPacketFront(_cleanSession,
                          _username,
                          _password,
                          _willTopic,
                          _willRetain,
                          _willQos,
                          _willPayload,
                          _willPayloadLength,
                          (uint16_t)(_keepAlive / 1000),  // 32b to 16b doesn't overflow because it comes from 16b orignally
                          _clientId)) {
        #if defined(ARDUINO_ARCH_ESP32)
        if (_useInternalTask == espMqttClientTypes::UseInternalTask::YES) {
          vTaskResume(_taskHandle);
        }
        #endif
        _state = State::connectingTcp1;
      } else {
        EMC_SEMAPHORE_GIVE();
        emc_log_e("Could not create CONNECT packet");
        _onError(0, espMqttClientTypes::Error::OUT_OF_MEMORY);
        result = false;
      }
      EMC_SEMAPHORE_GIVE();
    }
    return result;
  }

  bool disconnect(bool force = false) {
    if (force && _state != State::disconnected && _state != State::disconnectingTcp1 && _state != State::disconnectingTcp2) {
      _state = State::disconnectingTcp1;
      return true;
    }
    if (!force && _state == State::connected) {
      _state = State::disconnectingMqtt1;
      return true;
    }
    return false;
  }

  void clearQueue(bool deleteSessionData = false) {  // Not MQTT compliant and may cause unpredictable results when `deleteSessionData` = true!
    _clearQueue(deleteSessionData ? 2 : 0);
  }

  const char* getClientId() const {
    return _clientId;
  }

  void loop() {
    switch (_state) {
      case State::disconnected:
        #if defined(ARDUINO_ARCH_ESP32)
        if (_useInternalTask == espMqttClientTypes::UseInternalTask::YES) {
          vTaskSuspend(_taskHandle);
        }
        #endif
        break;
      case State::connectingTcp1:
        if (_useIp ? _transport->connect(_ip, _port) : _transport->connect(_host, _port)) {
          _state = State::connectingTcp2;
        } else {
          _state = State::disconnectingTcp1;
          _disconnectReason = espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED;
          break;
        }
        // Falling through to speed up connecting on blocking transport 'connect' implementations
        [[fallthrough]];
      case State::connectingTcp2:
        if (_transport->connected()) {
          _parser.reset();
          _lastClientActivity = _lastServerActivity = millis();
          _state = State::connectingMqtt;
        }
        break;
      case State::connectingMqtt:
        #if EMC_WAIT_FOR_CONNACK
        if (_transport->connected()) {
          _sendPacket();
          _checkIncoming();
          _checkPing();
        } else {
          _state = State::disconnectingTcp1;
          _disconnectReason = espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED;
        }
        break;
        #else
        // receipt of CONNACK packet will set state to CONNECTED
        // client however is allowed to send packets before CONNACK is received
        // so we fall through to 'connected'
        [[fallthrough]];
        #endif
      case State::connected:
        [[fallthrough]];
      case State::disconnectingMqtt2:
        if (_transport->connected()) {
          // CONNECT packet is first in the queue
          _checkOutbox();
          _checkIncoming();
          _checkPing();
          _checkTimeout();
        } else {
          _state = State::disconnectingTcp1;
          _disconnectReason = espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED;
        }
        break;
      case State::disconnectingMqtt1:
        EMC_SEMAPHORE_TAKE();
        if (_outbox.empty()) {
          if (!_addPacket(espMqttClientInternals::PacketType.DISCONNECT)) {
            EMC_SEMAPHORE_GIVE();
            emc_log_e("Could not create DISCONNECT packet");
            _onError(0, espMqttClientTypes::Error::OUT_OF_MEMORY);
          } else {
            _state = State::disconnectingMqtt2;
          }
        }
        EMC_SEMAPHORE_GIVE();
        _checkOutbox();
        _checkIncoming();
        _checkPing();
        _checkTimeout();
        break;
      case State::disconnectingTcp1:
        _transport->stop();
        _state = State::disconnectingTcp2;
        break;  // keep break to accomodate async clients
      case State::disconnectingTcp2:
        if (_transport->disconnected()) {
          _clearQueue(0);
          _bytesSent = 0;
          _state = State::disconnected;
          if (_onDisconnectCallback) _onDisconnectCallback(_disconnectReason);
        }
        break;
      // all cases covered, no default case
    }
    EMC_YIELD();
    #if defined(ARDUINO_ARCH_ESP32) && ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
    size_t waterMark = uxTaskGetStackHighWaterMark(NULL);
    if (waterMark < _highWaterMark) {
      _highWaterMark = waterMark;
      emc_log_i("Stack usage: %zu/%i", EMC_TASK_STACK_SIZE - _highWaterMark, EMC_TASK_STACK_SIZE);
    }
    #endif
  }

 protected:
  explicit MqttClient(espMqttClientTypes::UseInternalTask useInternalTask, uint8_t priority = 1, uint8_t core = 1)
  #if defined(ARDUINO_ARCH_ESP32)
  : _useInternalTask(useInternalTask)
  , _transport(nullptr)
  #else
  : _transport(nullptr)
  #endif
  , _parser()
  , _onConnectCallback(nullptr)
  , _onDisconnectCallback(nullptr)
  , _onSubscribeCallback(nullptr)
  , _onUnsubscribeCallback(nullptr)
  , _onMessageCallback(nullptr)
  , _onPublishCallback(nullptr)
  , _onErrorCallback(nullptr)
  , _clientId(nullptr)
  , _ip()
  , _host(nullptr)
  , _port(1883)
  , _useIp(false)
  , _keepAlive(15000)
  , _cleanSession(true)
  , _username(nullptr)
  , _password(nullptr)
  , _willTopic(nullptr)
  , _willPayload(nullptr)
  , _willPayloadLength(0)
  , _willQos(0)
  , _willRetain(false)
  , _timeout(10000)
  , _state(State::disconnected)
  , _generatedClientId{0}
  , _packetId(0)
  #if defined(ARDUINO_ARCH_ESP32)
  , _xSemaphore(nullptr)
  , _taskHandle(nullptr)
  #endif
  , _rxBuffer{0}
  , _outbox()
  , _bytesSent(0)
  , _lastClientActivity(0)
  , _lastServerActivity(0)
  , _pingSent(false)
  , _disconnectReason(espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED)
  #if defined(ARDUINO_ARCH_ESP32) && ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  , _highWaterMark(4294967295)
  #endif
    {
    EMC_GENERATE_CLIENTID(_generatedClientId);
  #if defined(ARDUINO_ARCH_ESP32)
    _xSemaphore = xSemaphoreCreateMutex();
    EMC_SEMAPHORE_GIVE();  // release before first use
    if (_useInternalTask == espMqttClientTypes::UseInternalTask::YES) {
      xTaskCreatePinnedToCore((TaskFunction_t)_loop, "mqttclient", EMC_TASK_STACK_SIZE, this, priority, &_taskHandle, core);
    }
  #else
    (void) useInternalTask;
    (void) priority;
    (void) core;
  #endif
    _clientId = _generatedClientId;
  }
  espMqttClientTypes::UseInternalTask _useInternalTask;
  espMqttClientInternals::Transport* _transport;
  TParser _parser;
  espMqttClientTypes::OnConnectCallback _onConnectCallback;
  espMqttClientTypes::OnDisconnectCallback _onDisconnectCallback;
  espMqttClientTypes::OnSubscribeCallback _onSubscribeCallback;
  espMqttClientTypes::OnUnsubscribeCallback _onUnsubscribeCallback;
  espMqttClientTypes::OnMessageCallback _onMessageCallback;
  espMqttClientTypes::OnPublishCallback _onPublishCallback;
  espMqttClientTypes::OnErrorCallback _onErrorCallback;
  const char* _clientId;
  IPAddress _ip;
  const char* _host;
  uint16_t _port;
  bool _useIp;
  uint32_t _keepAlive;
  bool _cleanSession;
  const char* _username;
  const char* _password;
  const char* _willTopic;
  const uint8_t* _willPayload;
  uint16_t _willPayloadLength;
  uint8_t _willQos;
  bool _willRetain;
  uint32_t _timeout;
  enum class State {
    disconnected =       0,
    connectingTcp1 =     1,
    connectingTcp2 =     2,
    connectingMqtt =     3,
    connected =          4,
    disconnectingMqtt1 = 5,
    disconnectingMqtt2 = 6,
    disconnectingTcp1 =  7,
    disconnectingTcp2 =  8
  };
  std::atomic<State> _state;
  char _generatedClientId[EMC_CLIENTID_LENGTH];
  uint16_t _packetId;

  uint16_t _getNextPacketId() {
    uint16_t packetId = 0;
    EMC_SEMAPHORE_TAKE();
    // cppcheck-suppress knownConditionTrueFalse
    packetId = (++_packetId == 0) ? ++_packetId : _packetId;
    EMC_SEMAPHORE_GIVE();
    return packetId;
  }

#if defined(ARDUINO_ARCH_ESP32)
  SemaphoreHandle_t _xSemaphore;
#elif defined(ARDUINO_ARCH_ESP8266) && EMC_ESP8266_MULTITHREADING
  std::atomic<bool> _xSemaphore = false;
#elif defined(__linux__)
  std::mutex mtx;
#endif
  template <typename... Args>
  bool _addPacket(Args&&... args) {
    espMqttClientTypes::Error error(espMqttClientTypes::Error::SUCCESS);
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.emplace(0, error, std::forward<Args>(args) ...);
    if (it && error == espMqttClientTypes::Error::SUCCESS) return true;
    return false;
  }
  template <typename... Args>
  bool _addPacketFront(Args&&... args) {
    espMqttClientTypes::Error error(espMqttClientTypes::Error::SUCCESS);
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.emplaceFront(0, error, std::forward<Args>(args) ...);
    if (it && error == espMqttClientTypes::Error::SUCCESS) return true;
    return false;
  }

  void _onError(uint16_t packetId, espMqttClientTypes::Error error) {
    if (_onErrorCallback) {
      _onErrorCallback(packetId, error);
    }
  }

 private:
  #if defined(ARDUINO_ARCH_ESP32)
  TaskHandle_t _taskHandle;
  static void _loop(MqttClient* c) {
    #if EMC_USE_WATCHDOG
    if (esp_task_wdt_add(NULL) != ESP_OK) {
      emc_log_e("Failed to add async task to WDT");
    }
    #endif
    for (;;) {
      c->loop();
      #if EMC_USE_WATCHDOG
      esp_task_wdt_reset();
      #endif
    }
  }
  #endif
  uint8_t _rxBuffer[EMC_RX_BUFFER_SIZE];
  struct OutgoingPacket {
    uint32_t timeSent;
    TPacket packet;
    template <typename... Args>
    OutgoingPacket(uint32_t t, espMqttClientTypes::Error error, Args&&... args) :
      timeSent(t),
      packet(error, std::forward<Args>(args) ...) {}
  };
  espMqttClientInternals::Outbox<OutgoingPacket> _outbox;
  size_t _bytesSent;
  uint32_t _lastClientActivity;
  uint32_t _lastServerActivity;
  bool _pingSent;
  espMqttClientTypes::DisconnectReason _disconnectReason;

  void _checkOutbox() {
    while (_sendPacket() > 0) {
      if (!_advanceOutbox()) {
        break;
      }
    }
  }

  int _sendPacket() {
    EMC_SEMAPHORE_TAKE();
    OutgoingPacket* packet = _outbox.getCurrent();

    size_t wantToWrite = 0;
    size_t written = 0;
    if (packet && (wantToWrite == written)) {
      wantToWrite = packet->packet.available(_bytesSent);
      if (wantToWrite == 0) {
        EMC_SEMAPHORE_GIVE();
        return 0;
      }
      written = _transport->write(packet->packet.data(_bytesSent), wantToWrite);
      packet->timeSent = millis();
      _lastClientActivity = millis();
      _bytesSent += written;
      emc_log_i("tx %zu/%zu (%02x)", _bytesSent, packet->packet.size(), packet->packet.packetType());
    }
    EMC_SEMAPHORE_GIVE();
    return written;
  }

  bool _advanceOutbox() {
    EMC_SEMAPHORE_TAKE();
    OutgoingPacket* packet = _outbox.getCurrent();
    if (packet && _bytesSent == packet->packet.size()) {
      if ((packet->packet.packetType()) == espMqttClientInternals::PacketType.DISCONNECT) {
        _state = State::disconnectingTcp1;
        _disconnectReason = espMqttClientTypes::DisconnectReason::USER_OK;
      }
      if (packet->packet.removable()) {
        _outbox.removeCurrent();
      } else {
        // we already set 'dup' here, in case we have to retry
        if ((packet->packet.packetType()) == espMqttClientInternals::PacketType.PUBLISH) packet->packet.setDup();
        _outbox.next();
      }
      packet = _outbox.getCurrent();
      _bytesSent = 0;
    }
    EMC_SEMAPHORE_GIVE();
    return packet;
  }

  void _checkIncoming() {
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
          if (_state == State::connectingMqtt && packetType != espMqttClientInternals::PacketType.CONNACK) {
            emc_log_w("Disconnecting, expected CONNACK - protocol error");
            _state = State::disconnectingTcp1;
            return;
          }
          switch (packetType & 0xF0) {
            case espMqttClientInternals::PacketType.CONNACK:
              _onConnack();
              if (_state != State::connected) {
                return;
              }
              break;
            case espMqttClientInternals::PacketType.PUBLISH:
              if (_state >= State::disconnectingMqtt1) break;  // stop processing incoming once user has called disconnect
              _onPublish();
              break;
            case espMqttClientInternals::PacketType.PUBACK:
              _onPuback();
              break;
            case espMqttClientInternals::PacketType.PUBREC:
              _onPubrec();
              break;
            case espMqttClientInternals::PacketType.PUBREL:
              _onPubrel();
              break;
            case espMqttClientInternals::PacketType.PUBCOMP:
              _onPubcomp();
              break;
            case espMqttClientInternals::PacketType.SUBACK:
              _onSuback();
              break;
            case espMqttClientInternals::PacketType.UNSUBACK:
              _onUnsuback();
              break;
            case espMqttClientInternals::PacketType.PINGRESP:
              _pingSent = false;
              break;
          }
        } else if (result ==  espMqttClientInternals::ParserResult::protocolError) {
          emc_log_w("Disconnecting, protocol error");
          _state = State::disconnectingTcp1;
          _disconnectReason = espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED;
          return;
        }
        remainingBufferLength -= bytesParsed;
        index += bytesParsed;
        emc_log_i("Parsed %zu - remaining %i", bytesParsed, remainingBufferLength);
        bytesParsed = 0;
      }
    }
  }

  void _checkPing() {
    if (_keepAlive == 0) return;  // keepalive is disabled

    uint32_t currentMillis = millis();

    // disconnect when server was inactive for twice the keepalive time
    if (currentMillis - _lastServerActivity > 2 * _keepAlive) {
      emc_log_w("Disconnecting, server exceeded keepalive");
      _state = State::disconnectingTcp1;
      _disconnectReason = espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED;
      return;
    }

    // send ping when client was inactive during the keepalive time
    // or when server hasn't responded within keepalive time (typically due to QOS 0)
    if (!_pingSent &&
        ((currentMillis - _lastClientActivity > _keepAlive) ||
        (currentMillis - _lastServerActivity > _keepAlive))) {
      EMC_SEMAPHORE_TAKE();
      if (!_addPacket(espMqttClientInternals::PacketType.PINGREQ)) {
        EMC_SEMAPHORE_GIVE();
        emc_log_e("Could not create PING packet");
        return;
      }
      EMC_SEMAPHORE_GIVE();
      _pingSent = true;
    }
  }

  void _checkTimeout() {
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    // check that we're not busy sending
    // don't check when first item hasn't been sent yet
    if (it && _bytesSent == 0 && it.get() != _outbox.getCurrent()) {
      if (millis() - it.get()->timeSent > _timeout) {
        emc_log_w("Packet ack timeout, retrying");
        _outbox.resetCurrent();
      }
    }
    EMC_SEMAPHORE_GIVE();
  }

  void _onConnack() {
    if (_parser.getPacket().variableHeader.fixed.connackVarHeader.returnCode == 0x00) {
      _pingSent = false;  // reset after keepalive timeout disconnect
      _state = State::connected;
      _advanceOutbox();
      if (_parser.getPacket().variableHeader.fixed.connackVarHeader.sessionPresent == 0) {
        _clearQueue(1);
      }
      if (_onConnectCallback) {
        _onConnectCallback(_parser.getPacket().variableHeader.fixed.connackVarHeader.sessionPresent);
      }
    } else {
      _state = State::disconnectingTcp1;
      // cast is safe because the parser already checked for a valid return code
      _disconnectReason = static_cast<espMqttClientTypes::DisconnectReason>(_parser.getPacket().variableHeader.fixed.connackVarHeader.returnCode);
    }
  }

  void _onPublish() {
    typename TParser::IncomingPacket p = _parser.getPacket();
    uint8_t qos = p.qos();
    bool retain = p.retain();
    bool dup = p.dup();
    uint16_t packetId = p.variableHeader.fixed.packetId;
    bool callback = true;
    if (qos == 1) {
      if (p.payload.index + p.payload.length == p.payload.total) {
        EMC_SEMAPHORE_TAKE();
        if (!_addPacket(espMqttClientInternals::PacketType.PUBACK, packetId)) {
          emc_log_e("Could not create PUBACK packet");
        }
        EMC_SEMAPHORE_GIVE();
      }
    } else if (qos == 2) {
      EMC_SEMAPHORE_TAKE();
      typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
      while (it) {
        if ((it.get()->packet.packetType()) == espMqttClientInternals::PacketType.PUBREC && it.get()->packet.packetId() == packetId) {
          callback = false;
          emc_log_e("QoS2 packet previously delivered");
          break;
        }
        ++it;
      }
      if (p.payload.index + p.payload.length == p.payload.total) {
        if (!_addPacket(espMqttClientInternals::PacketType.PUBREC, packetId)) {
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

  void _onPuback() {
    bool callback = false;
    uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    while (it) {
      // PUBACKs come in the order PUBs are sent. So we only check the first PUB packet in outbox
      // if it doesn't match the ID, return
      if ((it.get()->packet.packetType()) == espMqttClientInternals::PacketType.PUBLISH) {
        if (it.get()->packet.packetId() == idToMatch) {
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

  void _onPubrec() {
    bool success = false;
    uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    while (it) {
      // PUBRECs come in the order PUBs are sent. So we only check the first PUB packet in outbox
      // if it doesn't match the ID, return
      if ((it.get()->packet.packetType()) == espMqttClientInternals::PacketType.PUBLISH) {
        if (it.get()->packet.packetId() == idToMatch) {
          if (!_addPacket(espMqttClientInternals::PacketType.PUBREL, idToMatch)) {
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

  void _onPubrel() {
    bool success = false;
    uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    while (it) {
      // PUBRELs come in the order PUBRECs are sent. So we only check the first PUBREC packet in outbox
      // if it doesn't match the ID, return
      if ((it.get()->packet.packetType()) == espMqttClientInternals::PacketType.PUBREC) {
        if (it.get()->packet.packetId() == idToMatch) {
          if (!_addPacket(espMqttClientInternals::PacketType.PUBCOMP, idToMatch)) {
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

  void _onPubcomp() {
    bool callback = false;
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
    while (it) {
      // PUBCOMPs come in the order PUBRELs are sent. So we only check the first PUBREL packet in outbox
      // if it doesn't match the ID, return
      if ((it.get()->packet.packetType()) == espMqttClientInternals::PacketType.PUBREL) {
        if (it.get()->packet.packetId() == idToMatch) {
          if (!_addPacket(espMqttClientInternals::PacketType.PUBCOMP, idToMatch)) {
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

  void _onSuback() {
    bool callback = false;
    uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    while (it) {
      if (((it.get()->packet.packetType()) == espMqttClientInternals::PacketType.SUBSCRIBE) && it.get()->packet.packetId() == idToMatch) {
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

  void _onUnsuback() {
    bool callback = false;
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    uint16_t idToMatch = _parser.getPacket().variableHeader.fixed.packetId;
    while (it) {
      if (it.get()->packet.packetId() == idToMatch) {
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

  void _clearQueue(int clearData) {  // 0: keep session,
                                     // 1: keep only PUBLISH qos > 0
                                     // 2: delete all
    emc_log_i("clearing queue (clear session: %d)", clearData);
    EMC_SEMAPHORE_TAKE();
    typename espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.front();
    if (clearData == 0) {
      // keep PUB (qos > 0, aka packetID != 0), PUBREC and PUBREL
      // Spec only mentions PUB and PUBREL but this lib implements method B from point 4.3.3 (Fig. 4.3)
      // and stores the packet id in the PUBREC packet. So we also must keep PUBREC.
      while (it) {
        espMqttClientInternals::MQTTPacketType type = it.get()->packet.packetType();
        if (type == espMqttClientInternals::PacketType.PUBREC ||
            type == espMqttClientInternals::PacketType.PUBREL ||
            (type == espMqttClientInternals::PacketType.PUBLISH && it.get()->packet.packetId() != 0)) {
          ++it;
        } else {
          _outbox.remove(it);
        }
      }
    } else if (clearData == 1) {
      // keep PUB
      while (it) {
        if (it.get()->packet.packetType() == espMqttClientInternals::PacketType.PUBLISH) {
          ++it;
        } else {
          _outbox.remove(it);
        }
      }
    } else {  // clearData == 2
      while (it) {
        _outbox.remove(it);
      }
    }
    EMC_SEMAPHORE_GIVE();
  }

  #if defined(ARDUINO_ARCH_ESP32)
  #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  size_t _highWaterMark;
  #endif
  #endif
};
