/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#if defined(__linux__)

#include "espMqttClientPosix.h"

espMqttClientPosix::espMqttClientPosix()
: _client() {
  _transport = &_client;
}

#endif
