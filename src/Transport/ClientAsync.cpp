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
  return _client.connect(ip, port) ? 1 : 0;
}

int ClientAsync::connect(const char *host, uint16_t port) {
  return _client.connect(host, port) ? 1 : 0;
}

size_t ClientAsync::write(const uint8_t *buf, size_t size) {
  return _client.write(reinterpret_cast<const char*>(buf), size);
}

int ClientAsync::available() {
  return _available;
}

int ClientAsync::read(uint8_t* buf, size_t size) {
  size_t willRead = std::min(size, _available);
  memcpy(buf, _buff, std::min(size, _available));
  return willRead;
}

void ClientAsync::stop() {
  _client.stop();
}

uint8_t ClientAsync::connected() {
  return _client.connected();
}

void ClientAsync::stop(bool force) {
  if (force) {
    _client.stop();
  } else {
    _client.close();
  }
}

AsyncClient* ClientAsync::getClient() {
  return &_client;
}
