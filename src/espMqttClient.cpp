/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClient.h"


#if defined(ESP32)
espMqttClient::espMqttClient(uint8_t priority, uint8_t core)
: MqttClient(priority, core)
, _client() {
  _transport = &_client;
}
#else
espMqttClient::espMqttClient()
: _client() {
  _transport = &_client;
}
#endif

espMqttClient& espMqttClient::setKeepAlive(uint16_t keepAlive) {
  _keepAlive = keepAlive;
  return *this;
}

espMqttClient& espMqttClient::setClientId(const char* clientId) {
  _clientId = clientId;
  return *this;
}

espMqttClient& espMqttClient::setCleanSession(bool cleanSession) {
  _cleanSession = cleanSession;
  return *this;
}

espMqttClient& espMqttClient::setCredentials(const char* username, const char* password) {
  _username = username;
  _password = password;
  return *this;
}

espMqttClient& espMqttClient::setWill(const char* topic, uint8_t qos, bool retain, const uint8_t* payload, size_t length) {
  _willTopic = topic;
  _willQos = qos;
  _willRetain = retain;
  _willPayload = payload;
  if (!_willPayload) {
    _willPayloadLength = 0;
  } else {
    _willPayloadLength = length;
  }
  return *this;
}

espMqttClient& espMqttClient::setWill(const char* topic, uint8_t qos, bool retain, const char* payload) {
  return setWill(topic, qos, retain, reinterpret_cast<const uint8_t*>(payload), strlen(payload));
}

espMqttClient& espMqttClient::setServer(IPAddress ip, uint16_t port) {
  _ip = ip;
  _port = port;
  _useIp = true;
  return *this;
}

espMqttClient& espMqttClient::setServer(const char* host, uint16_t port) {
  _host = host;
  _port = port;
  _useIp = false;
  return *this;
}

espMqttClient& espMqttClient::onConnect(espMqttClientTypes::OnConnectCallback callback) {
  _onConnectCallback = callback;
  return *this;
}

espMqttClient& espMqttClient::onDisconnect(espMqttClientTypes::OnDisconnectCallback callback) {
  _onDisconnectCallback = callback;
  return *this;
}

espMqttClient& espMqttClient::onSubscribe(espMqttClientTypes::OnSubscribeCallback callback) {
  _onSubscribeCallback = callback;
  return *this;
}

espMqttClient& espMqttClient::onUnsubscribe(espMqttClientTypes::OnUnsubscribeCallback callback) {
  _onUnsubscribeCallback = callback;
  return *this;
}

espMqttClient& espMqttClient::onMessage(espMqttClientTypes::OnMessageCallback callback) {
  _onMessageCallback = callback;
  return *this;
}

espMqttClient& espMqttClient::onPublish(espMqttClientTypes::OnPublishCallback callback) {
  _onPublishCallback = callback;
  return *this;
}

/*
espMqttClient& espMqttClient::onError(espMqttClientTypes::OnErrorCallback callback) {
  _onErrorCallback = callback;
  return *this;
}
*/
