/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ClientSync.h"

namespace espMqttClientInternals {

ClientSync::ClientSync()
: client() {
  // empty
}

bool ClientSync::connect(IPAddress ip, uint16_t port) {
  bool ret = client.connect(ip, port);  // implicit conversion of return code int --> bool
  client.setNoDelay(true);
  return ret;
}

bool ClientSync::connect(const char* host, uint16_t port) {
  bool ret = client.connect(host, port);  // implicit conversion of return code int --> bool
  client.setNoDelay(true);
  return ret;
}

size_t ClientSync::write(const uint8_t* buf, size_t size) {
  return client.write(buf, size);
}

int ClientSync::available() {
  return client.available();
}

int ClientSync::read(uint8_t* buf, size_t size) {
  return read(buf, size);
}

void ClientSync::stop() {
  client.stop();
}

bool ClientSync::connected() {
  return client.connected();
}

bool ClientSync::disconnected() {
  return !client.connected();
}

}  // end namespaceespMqttClientInternals
