/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include "MqttClient.h"

class espMqttClient : public MqttClient {
 public:
#if defined(ESP32)
  explicit espMqttClient(uint8_t priority = 1, uint8_t core = 1);
#else
  espMqttClient();
#endif
  espMqttClient& setKeepAlive(uint16_t keepAlive);
  espMqttClient& setClientId(const char* clientId);
  espMqttClient& setCleanSession(bool cleanSession);
  espMqttClient& setCredentials(const char* username, const char* password);
  espMqttClient& setWill(const char* topic, uint8_t qos, bool retain, const uint8_t* payload, size_t length);
  espMqttClient& setWill(const char* topic, uint8_t qos, bool retain, const char* payload);
  espMqttClient& setServer(IPAddress ip, uint16_t port);
  espMqttClient& setServer(const char* host, uint16_t port);

  espMqttClient& onConnect(espMqttClientTypes::OnConnectCallback callback);
  espMqttClient& onDisconnect(espMqttClientTypes::OnDisconnectCallback callback);
  espMqttClient& onSubscribe(espMqttClientTypes::OnSubscribeCallback callback);
  espMqttClient& onUnsubscribe(espMqttClientTypes::OnUnsubscribeCallback callback);
  espMqttClient& onMessage(espMqttClientTypes::OnMessageCallback callback);
  espMqttClient& onPublish(espMqttClientTypes::OnPublishCallback callback);
  // espMqttClient& onError(espMqttClientTypes::OnErrorCallback callback);

 protected:
  WiFiClient _client;
};
