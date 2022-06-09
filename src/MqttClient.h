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

#if defined(esp32)
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
#endif

#include <Arduino.h>  // millis()
#include <WiFiClient.h>

#include "Config.h"
#include "TypeDefs.h"
#include "Helpers.h"
#include "Logging.h"
#include "Outbox.h"
#include "Packets/Packet.h"
#include "Packets/Parser.h"

class MqttClient {
 public:
  virtual ~MqttClient();
  bool connected() const;
  bool connect();
  bool disconnect(bool force = false);
  uint16_t subscribe(const char* topic, uint8_t qos);
  uint16_t unsubscribe(const char* topic);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, const uint8_t* payload, size_t length);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, espMqttClientTypes::PayloadCallback callback, size_t length);
  void clearQueue(bool all = false);  // Not MQTT compliant and may cause unpredictable results when `all` = true!
  const char* getClientId() const;
  #if defined(ESP32)

 private:
  #endif
  void loop();

 protected:
#if defined(ESP32)
  explicit MqttClient(uint8_t priority = 1, uint8_t core = 1);
#else
  MqttClient();
#endif
  WiFiClient* _transport;

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
  uint16_t _keepAlive;
  bool _cleanSession;
  const char* _username;
  const char* _password;
  const char* _willTopic;
  const uint8_t* _willPayload;
  uint16_t _willPayloadLength;
  uint8_t _willQos;
  bool _willRetain;

 private:
  char _generatedClientId[EMC_CLIENTID_LENGTH];
  uint16_t _packetId;

  enum State {
    DISCONNECTED,
    CONNECTINGTCP,
    CONNECTINGMQTT,
    CONNECTED,
    DISCONNECTINGMQTT1,
    DISCONNECTINGMQTT2,
    DISCONNECTINGTCP
  };
  std::atomic<State> _state;

#if defined(ESP32)
  SemaphoreHandle_t _xSemaphore;
  TaskHandle_t _taskHandle;
  static void _loop(MqttClient* c);
#elif defined(ESP8266) && EMC_ESP8266_MULTITHREADING
  std::atomic<bool> _xSemaphore = false;
#endif

  uint8_t _rxBuffer[EMC_RX_BUFFER_SIZE];
  espMqttClientInternals::Outbox<espMqttClientInternals::Packet> _outbox;
  size_t _bytesSent;
  espMqttClientInternals::Parser _parser;
  uint32_t _lastClientActivity;
  uint32_t _lastServerActivity;
  espMqttClientTypes::DisconnectReason _disconnectReason;

  uint16_t _getNextPacketId();

  template <typename... Args>
  bool _addPacket(bool addBack, Args&&... args) {
    if (addBack) {
      if (_outbox.emplace(std::forward<Args>(args) ...)) return true;
    } else {
      if (_outbox.emplaceFront(std::forward<Args>(args) ...)) return true;
    }
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

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  size_t _highWaterMark;
#endif
};
