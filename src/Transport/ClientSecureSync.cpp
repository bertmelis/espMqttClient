/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ClientSecureSync.h"

namespace espMqttClientInternals {

ClientSecureSync::ClientSecureSync()
: client() {
  // empty
}

bool ClientSecureSync::connect(IPAddress ip, uint16_t port) {
  bool ret = client.connect(ip, port);  // implicit conversion of return code int --> bool
  client.setNoDelay(true);
  return ret;
}

bool ClientSecureSync::connect(const char* host, uint16_t port) {
  bool ret = client.connect(host, port);  // implicit conversion of return code int --> bool
  client.setNoDelay(true);
  return ret;
}

size_t ClientSecureSync::write(const uint8_t* buf, size_t size) {
  return client.write(buf, size);
}

int ClientSecureSync::available() {
  return client.available();
}

int ClientSecureSync::read(uint8_t* buf, size_t size) {
  return read(buf, size);
}

void ClientSecureSync::stop() {
  client.stop();
}

bool ClientSecureSync::connected() {
  return client.connected();
}

bool ClientSecureSync::disconnected() {
  return !client.connected();
}

}  // namespace espMqttClientInternals
