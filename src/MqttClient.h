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
  void clearQueue(bool deleteSessionData = false);  // Not MQTT compliant and may cause unpredictable results when `deleteSessionData` = true!
  const char* getClientId() const;
  void loop();

 protected:
  explicit MqttClient(espMqttClientTypes::UseInternalTask useInternalTask, uint8_t priority = 1, uint8_t core = 1);
  espMqttClientTypes::UseInternalTask _useInternalTask;
  espMqttClientInternals::Transport* _transport;
  espMqttClientInternals::Parser* _parser;
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
  uint16_t _getNextPacketId();
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
    espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.emplace(0, error, std::forward<Args>(args) ...);
    if (it && error == espMqttClientTypes::Error::SUCCESS) return true;
    return false;
  }
  template <typename... Args>
  bool _addPacketFront(Args&&... args) {
    espMqttClientTypes::Error error(espMqttClientTypes::Error::SUCCESS);
    espMqttClientInternals::Outbox<OutgoingPacket>::Iterator it = _outbox.emplaceFront(0, error, std::forward<Args>(args) ...);
    if (it && error == espMqttClientTypes::Error::SUCCESS) return true;
    return false;
  }
  void _onError(uint16_t packetId, espMqttClientTypes::Error error);

 private:
  #if defined(ARDUINO_ARCH_ESP32)
  TaskHandle_t _taskHandle;
  static void _loop(MqttClient* c);
  #endif
  uint8_t _rxBuffer[EMC_RX_BUFFER_SIZE];
  struct OutgoingPacket {
    uint32_t timeSent;
    espMqttClientInternals::Packet packet;
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
  void _checkOutbox();
  int _sendPacket();
  bool _advanceOutbox();
  void _checkIncoming();
  void _checkPing();
  void _checkTimeout();
  void _onConnack();
  void _onPublish();
  void _onPuback();
  void _onPubrec();
  void _onPubrel();
  void _onPubcomp();
  void _onSuback();
  void _onUnsuback();
  void _clearQueue(int clearData);  // 0: keep session,
                                    // 1: keep only PUBLISH qos > 0
                                    // 2: delete all
  #if defined(ARDUINO_ARCH_ESP32)
  #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  size_t _highWaterMark;
  #endif
  #endif
};
