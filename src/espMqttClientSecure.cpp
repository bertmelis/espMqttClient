/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClientSecure.h"


#if defined(ESP32)
espMqttClientSecure::espMqttClientSecure(uint8_t priority, uint8_t core)
: MqttClient(priority, core)
, _client() {
  _transport = &_client;
}
#else
espMqttClientSecure::espMqttClientSecure()
: _client() {
  _transport = &_client;
}
#endif

espMqttClientSecure& espMqttClientSecure::setInsecure() {
  _client.setInsecure();
  return *this;
}

#if defined(ESP32)
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
#elif defined(ESP8266)
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

espMqttClientSecure& espMqttClientSecure::setKeepAlive(uint16_t keepAlive) {
  _keepAlive = keepAlive;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setClientId(const char* clientId) {
  _clientId = clientId;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setCleanSession(bool cleanSession) {
  _cleanSession = cleanSession;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setCredentials(const char* username, const char* password) {
  _username = username;
  _password = password;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setWill(const char* topic, uint8_t qos, bool retain, const uint8_t* payload, size_t length) {
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

espMqttClientSecure& espMqttClientSecure::setWill(const char* topic, uint8_t qos, bool retain, const char* payload) {
  return setWill(topic, qos, retain, reinterpret_cast<const uint8_t*>(payload), strlen(payload));
}

espMqttClientSecure& espMqttClientSecure::setServer(IPAddress ip, uint16_t port) {
  _ip = ip;
  _port = port;
  _useIp = true;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::setServer(const char* host, uint16_t port) {
  _host = host;
  _port = port;
  _useIp = false;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::onConnect(espMqttClientTypes::OnConnectCallback callback) {
  _onConnectCallback = callback;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::onDisconnect(espMqttClientTypes::OnDisconnectCallback callback) {
  _onDisconnectCallback = callback;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::onSubscribe(espMqttClientTypes::OnSubscribeCallback callback) {
  _onSubscribeCallback = callback;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::onUnsubscribe(espMqttClientTypes::OnUnsubscribeCallback callback) {
  _onUnsubscribeCallback = callback;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::onMessage(espMqttClientTypes::OnMessageCallback callback) {
  _onMessageCallback = callback;
  return *this;
}

espMqttClientSecure& espMqttClientSecure::onPublish(espMqttClientTypes::OnPublishCallback callback) {
  _onPublishCallback = callback;
  return *this;
}

/*
espMqttClientSecure& espMqttClientSecure::onError(espMqttClientTypes::OnErrorCallback callback) {
  _onErrorCallback = callback;
  return *this;
}
*/
