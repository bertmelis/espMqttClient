/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP32)
  #include <Arduino.h>  // ESP.getFreeHeap();
  #define EMC_SEMAPHORE_TAKE() xSemaphoreTake(_xSemaphore, portMAX_DELAY)
  #define EMC_SEMAPHORE_GIVE() xSemaphoreGive(_xSemaphore)
  #define EMC_GET_FREE_MEMORY() ESP.getMaxAllocHeap()
  #define EMC_YIELD() taskYIELD()
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <Arduino.h>  // ESP.getFreeHeap();
  #if EMC_ESP8266_MULTITHREADING
    // This lib doesn't run use multithreading on ESP8266
    // _xSemaphore defined as std::atomic<bool>
    #define SEMAPHORE_TAKE() while (_xSemaphore) { /*ESP.wdtFeed();*/ } _xSemaphore = true
    #define SEMAPHORE_GIVE() _xSemaphore = false
  #else
    #define EMC_SEMAPHORE_TAKE()
    #define EMC_SEMAPHORE_GIVE()
  #endif
  #define EMC_GET_FREE_MEMORY() ESP.getMaxFreeBlockSize()
  #define EMC_YIELD() yield()
#else
  #define EMC_GET_FREE_MEMORY() 1000000000
#endif
