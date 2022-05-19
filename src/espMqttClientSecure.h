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

class espMqttClientSecure : public MqttClient {
 public:
  #if defined(ESP32)
  explicit espMqttClientSecure(uint8_t priority = 1, uint8_t core = 1);
  #else
  espMqttClientSecure();
  #endif
  espMqttClientSecure& setInsecure();
  #if defined(ESP32)
  espMqttClientSecure& setCACert(const char* rootCA);
  espMqttClientSecure& setCertificate(const char* clientCa);
  espMqttClientSecure& setPrivateKey(const char* privateKey);
  espMqttClientSecure& setPreSharedKey(const char* pskIdent, const char* psKey);
  #else
  espMqttClientSecure& setFingerprint(const uint8_t fingerprint[20]);
  espMqttClientSecure& setTrustAnchors(const X509List *ta);
  espMqttClientSecure& setClientRSACert(const X509List *cert, const PrivateKey *sk);
  espMqttClientSecure& setClientECCert(const X509List *cert, const PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type);
  espMqttClientSecure& setCertStore(CertStoreBase *certStore);
  #endif
  espMqttClientSecure& setKeepAlive(uint16_t keepAlive);
  espMqttClientSecure& setClientId(const char* clientId);
  espMqttClientSecure& setCleanSession(bool cleanSession);
  espMqttClientSecure& setCredentials(const char* username, const char* password);
  espMqttClientSecure& setWill(const char* topic, uint8_t qos, bool retain, const uint8_t* payload, size_t length);
  espMqttClientSecure& setWill(const char* topic, uint8_t qos, bool retain, const char* payload);
  espMqttClientSecure& setServer(IPAddress ip, uint16_t port);
  espMqttClientSecure& setServer(const char* host, uint16_t port);

  espMqttClientSecure& onConnect(espMqttClientTypes::OnConnectCallback callback);
  espMqttClientSecure& onDisconnect(espMqttClientTypes::OnDisconnectCallback callback);
  espMqttClientSecure& onSubscribe(espMqttClientTypes::OnSubscribeCallback callback);
  espMqttClientSecure& onUnsubscribe(espMqttClientTypes::OnUnsubscribeCallback callback);
  espMqttClientSecure& onMessage(espMqttClientTypes::OnMessageCallback callback);
  espMqttClientSecure& onPublish(espMqttClientTypes::OnPublishCallback callback);
  // espMqttClientSecure& onError(espMqttClientTypes::OnErrorCallback callback);

 protected:
  WiFiClientSecure _client;
};
