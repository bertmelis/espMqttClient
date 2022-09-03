/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <WiFiClientSecure.h>  // includes IPAddress

#include "Transport.h"

namespace espMqttClientInternals {

class ClientSecureSync : public Transport {
 public:
  ClientSecureSync();
  bool connect(IPAddress ip, uint16_t port);
  bool connect(const char* host, uint16_t port);
  size_t write(const uint8_t* buf, size_t size);
  int available();
  int read(uint8_t* buf, size_t size);
  void stop();
  bool connected();
  bool disconnected();
  WiFiClientSecure client;
};

}  // end namespace espMqttClientInternals
