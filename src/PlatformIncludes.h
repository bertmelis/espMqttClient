/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#ifdef ARDUINO
  #include <Arduino.h>  // millis()
  #include <Client.h>


  #ifdef ARDUINO_ARCH_ESP32
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_task_wdt.h"
  #endif

#endif

#ifdef NATIVE
  // to be implemented
#endif
