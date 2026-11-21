/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "ClientSync.h"
#include "../Config.h"
#include <lwip/sockets.h>  // socket options

namespace espMqttClientInternals {

ClientSync::ClientSync()
: client() {
  // empty
}

bool ClientSync::connect(IPAddress ip, uint16_t port) {
  #if defined(ARDUINO_ARCH_ESP32)
  // NetworkClient::connect(ip, port) waits up to its default 3000 ms for an unreachable host, on the task that
  // pumps this client. EMC_CONNECT_TIMEOUT_MS (Config.h) lets the application choose a shorter bound.
  bool ret = client.connect(ip, port, EMC_CONNECT_TIMEOUT_MS);
  #else
  bool ret = client.connect(ip, port);  // implicit conversion of return code int --> bool
  #endif
  if (ret) {
    #if defined(ARDUINO_ARCH_ESP8266)
    client.setNoDelay(EMC_TCP_NODELAY);
    #elif defined(ARDUINO_ARCH_ESP32)
    // Set TCP option directly to bypass lack of working setNoDelay for WiFiClientSecure (for consistency also here)
    int val = EMC_TCP_NODELAY;
    client.setSocketOption(IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int));
    #endif
  }
  return ret;
}

bool ClientSync::connect(const char* host, uint16_t port) {
  #if defined(ARDUINO_ARCH_ESP32)
  bool ret = client.connect(host, port, EMC_CONNECT_TIMEOUT_MS);  // see connect(ip, port)
  #else
  bool ret = client.connect(host, port);  // implicit conversion of return code int --> bool
  #endif
  if (ret) {
    #if defined(ARDUINO_ARCH_ESP8266)
    client.setNoDelay(EMC_TCP_NODELAY);
    #elif defined(ARDUINO_ARCH_ESP32)
    // Set TCP option directly to bypass lack of working setNoDelay for WiFiClientSecure (for consistency also here)
    int val = EMC_TCP_NODELAY;
    client.setSocketOption(IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int));
    #endif
  }
  return ret;
}

size_t ClientSync::write(const uint8_t* buf, size_t size) {
  return client.write(buf, size);
}

int ClientSync::read(uint8_t* buf, size_t size) {
  return client.read(buf, size);
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

}  // namespace espMqttClientInternals

#endif
