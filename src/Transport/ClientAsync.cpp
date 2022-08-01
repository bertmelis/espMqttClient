/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ClientAsync.h"

ClientAsync::ClientAsync()
: tcpClient()
, availableData(0)
, bufData(nullptr) {
  tcpClient.setNoDelay(true);
}

int ClientAsync::connect(IPAddress ip, uint16_t port) {
  return tcpClient.connect(ip, port) ? 1 : 0;
}

int ClientAsync::connect(const char* host, uint16_t port) {
  return tcpClient.connect(host, port) ? 1 : 0;
}

size_t ClientAsync::write(const uint8_t* buf, size_t size) {
  return tcpClient.write(reinterpret_cast<const char*>(buf), size);
}

int ClientAsync::available() {
  return static_cast<int>(availableData);  // availableData will never be large enough to cause an overflow
}

int ClientAsync::read(uint8_t* buf, size_t size) {
  size_t willRead = std::min(size, availableData);
  memcpy(buf, bufData, std::min(size, availableData));
  if (availableData > size) {
    emc_log_w("Buffer is smaller than available data: %zu - %zu", size, availableData);
  }
  availableData = 0;
  return willRead;
}

void ClientAsync::stop() {
  tcpClient.stop();
}

uint8_t ClientAsync::connected() {
  return tcpClient.connected();
}

void ClientAsync::stop(bool force) {
  if (force) {
    tcpClient.stop();
  } else {
    tcpClient.close();
  }
}
