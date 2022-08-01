/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClientAsync.h"


#if defined(ARDUINO_ARCH_ESP32)
espMqttClientAsync::espMqttClientAsync(uint8_t priority, uint8_t core)
: MqttClientSetup(false, priority, core)
, _clientAsync() {
#else
espMqttClientAsync::espMqttClientAsync()
: _clientAsync() {
#endif
  _transport = &_clientAsync;
  //_onConnectHook = reinterpret_cast<MqttClient::OnConnectHook>(_setupClient);
  //_onConnectHookArg = this;
  _clientAsync.tcpClient.onConnect(onConnectCb, this);
  _clientAsync.tcpClient.onDisconnect(onDisconnectCb, this);
  _clientAsync.tcpClient.onData(onDataCb, this);
  _clientAsync.tcpClient.onPoll(onPollCb, this);
}

bool espMqttClientAsync::connect() {
  bool ret = MqttClient::connect();
  loop();
  return ret;
}

void espMqttClientAsync::_setupClient(espMqttClientAsync* c) {
  // empty
}

void espMqttClientAsync::onConnectCb(void* a, AsyncClient* c) {
  espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
  client->_state = MqttClient::State::connectingTcp2;
  client->loop();
}

void espMqttClientAsync::onDataCb(void* a, AsyncClient* c, void* data, size_t len) {
  espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
  client->_clientAsync.bufData = reinterpret_cast<uint8_t*>(data);
  client->_clientAsync.availableData = len;
  client->loop();
}

void espMqttClientAsync::onDisconnectCb(void* a, AsyncClient* c) {
  espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
  client->_state = MqttClient::State::disconnectingTcp;
  client->loop();
}

void espMqttClientAsync::onPollCb(void* a, AsyncClient* c) {
  espMqttClientAsync* client = reinterpret_cast<espMqttClientAsync*>(a);
  client->loop();
}
