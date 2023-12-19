/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "Transport/ClientAsync.h"

#include "MqttClientSetup.h"

template<class MQTTVERSION>
class espMqttClientAsync : public MqttClientSetup<espMqttClientAsync, MQTTVERSION> {
 public:
  espMqttClientAsync()
  : MqttClientSetup<espMqttClientAsync, MQTTVERSION>(espMqttClientTypes::UseInternalTask::NO)
  , _clientAsync() {
    Mqttclient::_transport = &_clientAsync;
    _clientAsync.client.onConnect(onConnectCb, this);
    _clientAsync.client.onDisconnect(onDisconnectCb, this);
    _clientAsync.client.onData(onDataCb, this);
    _clientAsync.client.onPoll(onPollCb, this);
  }

  bool connect() {
    bool ret = MqttClient::connect();
    loop();
    return ret;
  }

 protected:
  espMqttClientInternals::ClientAsync _clientAsync;

  static void _setupClient(espMqttClientAsync* c) {
    (void)c;
  }

  static void onConnectCb(void* a, AsyncClient* c) {
    c->setNoDelay(true);
    espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
    client->_state = MqttClient::State::connectingTcp2;
    client->loop();
  }

  static void onDataCb(void* a, AsyncClient* c, void* data, size_t len) {
    (void)c;
    espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
    client->_clientAsync.bufData = reinterpret_cast<uint8_t*>(data);
    client->_clientAsync.availableData = len;
    client->loop();
  }

  static void onDisconnectCb(void* a, AsyncClient* c) {
    (void)c;
    espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
    client->_state = MqttClient::State::disconnectingTcp2;
    client->loop();
  }

  static void onPollCb(void* a, AsyncClient* c) {
    (void)c;
    espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
    client->loop();
  }
};

#endif

#define espMqttClientAsync espMqttClientAsync<MqttVersion::v3_1_1>
