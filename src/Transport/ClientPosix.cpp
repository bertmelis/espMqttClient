/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ClientPosix.h"

#if defined(__linux__)

namespace espMqttClientInternals {

ClientPosix::ClientPosix()
: _sockfd(-1)
, _host() {
  // empty
}

ClientPosix::~ClientPosix() {
  ClientPosix::stop();
}

bool ClientPosix::connect(IPAddress ip, uint16_t port) {
  if (connected()) stop();

  _sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (_sockfd < 0) {
    emc_log_e("Error %d: \"%s\" opening socket", errno, strerror(errno));
  }

  int flag = 1;
  if (setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0) {
    emc_log_e("Error %d: \"%s\" disabling nagle", errno, strerror(errno));
  }

  memset(&_host, 0, sizeof(_host));
  _host.sin_family = AF_INET;
  _host.sin_addr.s_addr = htonl(uint32_t(ip));
  _host.sin_port = ::htons(port);

  int ret = ::connect(_sockfd, reinterpret_cast<sockaddr*>(&_host), sizeof(_host));

  if (ret < 0) {
    emc_log_e("Error connecting: %d - (%d) %s", ret, errno, strerror(errno));
    return false;
  }

  emc_log_i("Socket connected");
  return true;
}

bool ClientPosix::connect(const char* hostname, uint16_t port) {
  if (connected()) stop();

  int err;
  struct addrinfo hints = {}, *addrs;
  char port_str[16] = {};
  
  hints.ai_family = AF_INET; // Since your original code was using sockaddr_in and
                             // PF_INET, I'm using AF_INET here to match.  Use
                             // AF_UNSPEC instead if you want to allow getaddrinfo()
                             // to find both IPv4 and IPv6 addresses for the hostname.
                             // Just make sure the rest of your code is equally family-
                             // agnostic when dealing with the IP addresses associated
                             // with this connection. For instance, make sure any uses
                             // of sockaddr_in are changed to sockaddr_storage,
                             // and pay attention to its ss_family field, etc...
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  emc_log_i("Connecting to %s:%d", hostname, port);

  err = getaddrinfo(hostname, port_str, &hints, &addrs);
  if (err != 0) {
    emc_log_e("Error getting addr info: %s - %s\n", hostname, gai_strerror(err));
    return false;
  }

  for(struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next) {
    _sockfd = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (_sockfd == -1) {
      err = errno;
      break; // if using AF_UNSPEC above instead of AF_INET/6 specifically,
             // replace this 'break' with 'continue' instead, as the 'ai_family'
             // may be different on the next iteration...
    }

    int flag = 1;
    if (setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0) {
      emc_log_e("Error %d disabling nagle", errno);
    }

    if (::connect(_sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
      break;
    }

    err = errno;

    close(_sockfd);
    _sockfd = -1;
  }

  freeaddrinfo(addrs);

  if (_sockfd < 0) {
    emc_log_e("%s: %s", hostname, strerror(err));
    return false;
  }

  emc_log_i("Socket connected");
  return true;
}

size_t ClientPosix::write(const uint8_t* buf, size_t size) {
  return ::send(_sockfd, buf, size, 0);
}

int ClientPosix::read(uint8_t* buf, size_t size) {
  int ret = ::recv(_sockfd, buf, size, MSG_DONTWAIT);
  /*
  if (ret < 0) {
    emc_log_e("Error reading: %s", strerror(errno));
  }
  */
  return ret;
}

void ClientPosix::stop() {
  if (_sockfd >= 0) {
    ::close(_sockfd);
    _sockfd = -1;
  }
}

bool ClientPosix::connected() {
  return _sockfd >= 0;
}

bool ClientPosix::disconnected() {
  return _sockfd < 0;
}

}  // namespace espMqttClientInternals

#endif
