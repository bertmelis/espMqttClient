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
#include "Packets/Packet.h"
#include "Packets/Parser.h"
#include "Transport/Transport.h"

class MqttClient {
 public:
  virtual ~MqttClient();
  bool connected() const;
  bool disconnected() const;
  bool connect();
  bool disconnect(bool force = false);
  template <typename... Args>
  uint16_t subscribe(const char* topic, uint8_t qos, Args&&... args) {
    uint16_t packetId = _getNextPacketId();
    if (_state != State::connected) {
      packetId = 0;
    } else {
      EMC_SEMAPHORE_TAKE();
      if (!_addPacket(packetId, topic, qos, std::forward<Args>(args) ...)) {
        emc_log_e("Could not create SUBSCRIBE packet");
        packetId = 0;
      }
      EMC_SEMAPHORE_GIVE();
    }
    return packetId;
  }
  template <typename... Args>
  uint16_t unsubscribe(const char* topic, Args&&... args) {
    uint16_t packetId = _getNextPacketId();
    if (_state != State::connected) {
      packetId = 0;
    } else {
      EMC_SEMAPHORE_TAKE();
      if (!_addPacket(packetId, topic, std::forward<Args>(args) ...)) {
        emc_log_e("Could not create UNSUBSCRIBE packet");
        packetId = 0;
      }
      EMC_SEMAPHORE_GIVE();
    }
    return packetId;
  }
  uint16_t publish(const char* topic, uint8_t qos, bool retain, const uint8_t* payload, size_t length);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, espMqttClientTypes::PayloadCallback callback, size_t length);
  void clearQueue(bool all = false);  // Not MQTT compliant and may cause unpredictable results when `all` = true!
  const char* getClientId() const;
  #if defined(ARDUINO_ARCH_ESP32)

 protected:
  #endif
  void loop();
  #if defined(ARDUINO_ARCH_ESP32)
  explicit MqttClient(bool useTask, uint8_t priority = 1, uint8_t core = 1);
  bool _useTask;
  #else

 protected:
  MqttClient();
  #endif
  espMqttClientInternals::Transport* _transport;

  espMqttClientTypes::OnConnectCallback _onConnectCallback;
  espMqttClientTypes::OnDisconnectCallback _onDisconnectCallback;
  espMqttClientTypes::OnSubscribeCallback _onSubscribeCallback;
  espMqttClientTypes::OnUnsubscribeCallback _onUnsubscribeCallback;
  espMqttClientTypes::OnMessageCallback _onMessageCallback;
  espMqttClientTypes::OnPublishCallback _onPublishCallback;
  espMqttClientTypes::OnErrorCallback _onErrorCallback;
  typedef void(*mqttClientHook)(void*);
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

  // state is protected to allow state changes by the transport system, defined in child classes
  // eg. to allow AsyncTCP
  enum class State {
    disconnected,
    connectingTcp1,
    connectingTcp2,
    connectingMqtt,
    connected,
    disconnectingMqtt1,
    disconnectingMqtt2,
    disconnectingTcp1,
    disconnectingTcp2
  };
  std::atomic<State> _state;

 private:
  char _generatedClientId[EMC_CLIENTID_LENGTH];
  uint16_t _packetId;

#if defined(ARDUINO_ARCH_ESP32)
  SemaphoreHandle_t _xSemaphore;
  TaskHandle_t _taskHandle;
  static void _loop(MqttClient* c);
#elif defined(ARDUINO_ARCH_ESP8266) && EMC_ESP8266_MULTITHREADING
  std::atomic<bool> _xSemaphore = false;
#elif defined(__linux__)
  std::mutex mtx;
#endif

  uint8_t _rxBuffer[EMC_RX_BUFFER_SIZE];
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet> _outbox;
  size_t _bytesSent;
  espMqttClientInternals::Parser _parser;
  uint32_t _lastClientActivity;
  uint32_t _lastServerActivity;
  bool _pingSent;
  espMqttClientTypes::DisconnectReason _disconnectReason;

  uint16_t _getNextPacketId();

  template <typename... Args>
  bool _addPacket(Args&&... args) {
    espMqttClientTypes::Error error;
    espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.emplace(error, std::forward<Args>(args) ...);
    if (it && error == espMqttClientTypes::Error::SUCCESS) return true;
    _outbox.remove(it);
    return false;
  }

  template <typename... Args>
  bool _addPacketFront(Args&&... args) {
    espMqttClientTypes::Error error;
    espMqttClientInternals::Outbox<espMqttClientInternals::Packet>::Iterator it = _outbox.emplaceFront(error, std::forward<Args>(args) ...);
    if (it && error == espMqttClientTypes::Error::SUCCESS) return true;
    _outbox.remove(it);
    return false;
  }

  void _checkOutgoing();
  void _checkIncoming();
  void _checkPing();

  void _onConnack();
  void _onPublish();
  void _onPuback();
  void _onPubrec();
  void _onPubrel();
  void _onPubcomp();
  void _onSuback();
  void _onUnsuback();

  void _clearQueue(bool clearSession);
  void _onError(uint16_t packetId, espMqttClientTypes::Error error);

  #if defined(ARDUINO_ARCH_ESP32)
  #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  size_t _highWaterMark;
  #endif
  #endif
};
