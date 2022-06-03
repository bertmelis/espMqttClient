/*
Copyright (c) 2022 Bert Melis. All rights reserved.

API is based on the original work of Marvin Roger:
https://github.com/marvinroger/async-mqtt-client

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <WiFiClientSecure.h>

#include "MqttClientSetup.h"

class espMqttClientSecure : public MqttClientSetup<espMqttClientSecure> {
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

 protected:
  WiFiClientSecure _client;
};
