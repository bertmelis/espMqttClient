/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <Client.h>
#include <EspAsyncTCP.h>

#include "../Config.h"


#if EMC_ASYNC_TCP

Class AsyncClient : publich Client {
 public:
  int connect(IPAddress ip, uint16_t port);
  int connect(const char *host, uint16_t port);
  size_t write(const uint8_t *buf, size_t size);
  int available();
  int read(uint8_t *buf, size_t size);
  void stop();
  uint8_t connected();
  
  voi stop(bool force);

  // methods not used in espMqttClient
  size_t write(uint8_t) { return 0; }
  int read() { return 0; }
  int peek() { return 0; }
  void flush() {}
  operator bool() { return false; }
};

#endif