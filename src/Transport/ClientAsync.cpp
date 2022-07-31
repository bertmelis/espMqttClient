/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ClientAsync.h"

ClientAsync::ClientAsync()
: _client()
, _available(0)
, _buff(nullptr) {
  // empty
}

int ClientAsync::connect(IPAddress ip, uint16_t port) {
  _client.connect(ip, port);
}

int ClientAsync::connect(const char *host, uint16_t port) {
  _client.connect(host, port);
}

size_t ClientAsync::write(const uint8_t *buf, size_t size) {
  _client.write(buf, size);
}

int ClientAsync::available() {
  return _available;
}

int ClientAsync::read(uint8_t* buf, size_t size) {
  memcpy(buf, _buff, std::min(size, _available));
}

void ClientAsync::stop() {
  _client.stop();
}

uint8_t ClientAsync::connected() {
  return _client.connected();
}

void stop(bool force) {
  if (force) {
    _client.stop();
  } else {
    _client.close();
  }
}

#if defined(ARDUINO_ARCH_ESP32)
AsyncClient* ClientAsync::getClient() {
#elif defined(ARDUINO_ARCH_ESP8266)
ESPAsyncClient* ClientAsync::getClient() {
#endif
  return _client;
}
