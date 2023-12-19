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
#include "Transport/ClientSync.h"
#include "Transport/ClientSecureSync.h"
#elif defined(__linux__)
#include "Transport/ClientPosix.h"
#endif

#include "MqttClientSetup.h"

#if defined(ARDUINO_ARCH_ESP8266)

template<class MQTTVERSION>
class espMqttClient : public MqttClientSetup<espMqttClient, MQTTVERSION> {
 public:
  espMqttClient()
  : MqttClientSetup<espMqttClient, MQTTVERSION>(espMqttClientTypes::UseInternalTask::NO)
  , _client() {
    MqttClient::_transport = &_client;
  }

 protected:
  espMqttClientInternals::ClientSync _client;
};

template<class MQTTVERSION>
class espMqttClientSecure : public MqttClientSetup<espMqttClientSecure, MQTTVERSION> {
 public:
  espMqttClientSecure()
  : MqttClientSetup<espMqttClientSecure, MQTTVERSION>(espMqttClientTypes::UseInternalTask::NO)
  , _client() {
    MqttClient::_transport = &_client;
  }

  espMqttClientSecure& setInsecure() {
    _client.client.setInsecure();
    return *this;
  }

  espMqttClientSecure& setFingerprint(const uint8_t fingerprint[20]) {
    _client.client.setFingerprint(fingerprint);
    return *this;
  }

  espMqttClientSecure& setTrustAnchors(const X509List *ta) {
    _client.client.setTrustAnchors(ta);
    return *this;
  }

  espMqttClientSecure& setClientRSACert(const X509List *cert, const PrivateKey *sk) {
    _client.client.setClientRSACert(cert, sk);
    return *this;
  }

  espMqttClientSecure& setClientECCert(const X509List *cert, const PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type) {
    _client.client.setClientECCert(cert, sk, allowed_usages, cert_issuer_key_type);
    return *this;
  }

  espMqttClientSecure& setCertStore(CertStoreBase *certStore) {
    _client.client.setCertStore(certStore);
    return *this;
  }

 protected:
  espMqttClientInternals::ClientSecureSync _client;
};
#endif

#if defined(ARDUINO_ARCH_ESP32)
template<class MQTTVERSION>
class espMqttClient : public MqttClientSetup<espMqttClient, MQTTVERSION> {
 public:
  explicit espMqttClient(espMqttClientTypes::UseInternalTask useInternalTask)
  : MqttClientSetup<espMqttClient, MQTTVERSION>(useInternalTask)
  , _client() {
    MqttClient::_transport = &_client;
  }

  explicit espMqttClient(uint8_t priority = 1, uint8_t core = 1)
  : MqttClientSetup<espMqttClient, MQTTVERSION>(espMqttClientTypes::UseInternalTask::YES, priority, core)
  , _client() {
    MqttClient::_transport = &_client;
  }

 protected:
  espMqttClientInternals::ClientSync _client;
};

template<class MQTTVERSION>
class espMqttClientSecure : public MqttClientSetup<espMqttClientSecure, MQTTVERSION> {
 public:
  explicit espMqttClientSecure(espMqttClientTypes::UseInternalTask useInternalTask)
  : MqttClientSetup<espMqttClientSecure, MQTTVERSION>(useInternalTask)
  , _client() {
    MqttClient::_transport = &_client;
  }

  explicit espMqttClientSecure(uint8_t priority = 1, uint8_t core = 1)
  : MqttClientSetup<espMqttClientSecure, MQTTVERSION>(espMqttClientTypes::UseInternalTask::YES, priority, core)
  , _client() {
    MqttClient::_transport = &_client;
  }

  espMqttClientSecure& setInsecure() {
    _client.client.setInsecure();
    return *this;
  }

  espMqttClientSecure& setCACert(const char* rootCA) {
    _client.client.setCACert(rootCA);
    return *this;
  }

  espMqttClientSecure& setCertificate(const char* clientCa) {
    _client.client.setCertificate(clientCa);
    return *this;
  }

  espMqttClientSecure& setPrivateKey(const char* privateKey) {
    _client.client.setPrivateKey(privateKey);
    return *this;
  }

  espMqttClientSecure& esetPreSharedKey(const char* pskIdent, const char* psKey) {
    _client.client.setPreSharedKey(pskIdent, psKey);
    return *this;
  }

 protected:
  espMqttClientInternals::ClientSecureSync _client;
};
#endif

#if defined(__linux__)
template<class MQTTVERSION>
class espMqttClient : public MqttClientSetup<espMqttClient, MQTTVERSION> {
 public:
  espMqttClient()
  : MqttClientSetup<espMqttClient, MQTTVERSION>(espMqttClientTypes::UseInternalTask::NO)
  , _client() {
    MqttClient::_transport = &_client;
  }

 protected:
  espMqttClientInternals::ClientPosix _client;
};
#endif

#define espMqttClient espMqttClient<MqttVersion::v3_1_1>
#define espMqttClientSecure espMqttClientSecure<MqttVersion::v3_1_1>
