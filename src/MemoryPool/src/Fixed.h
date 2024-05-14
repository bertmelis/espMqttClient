/*
Copyright (c) 2024 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cstddef>  // std::size_t
#include <cassert>  // assert
#include <mutex>  // NOLINT [build/c++11] std::mutex, std::lock_guard

#ifdef MEMPOL_DEBUG
#include <iostream>
#endif

namespace MemoryPool {

template <std::size_t nrBlocks, std::size_t blocksize>
class Fixed {
 public:
  Fixed()
  : _buffer{0}
  , _head(_buffer) {
    unsigned char* b = _head;
    std::size_t adjustedBlocksize = sizeof(std::size_t) > blocksize ? sizeof(std::size_t) : blocksize;
    for (std::size_t i = 0; i < nrBlocks - 1; ++i) {
      *reinterpret_cast<unsigned char**>(b) = b + adjustedBlocksize;
      b += adjustedBlocksize;
    }
    *reinterpret_cast<unsigned char**>(b) = nullptr;
  }

  // no copy nor move
  Fixed (const Fixed&) = delete;
  Fixed& operator= (const Fixed&) = delete;

  void* malloc() {
    const std::lock_guard<std::mutex> lockGuard(_mutex);
    if (_head) {
      void* retVal = _head;
      _head = *reinterpret_cast<unsigned char**>(_head);
      return retVal;
    }
    return nullptr;
  }

  void free(void* ptr) {
    if (!ptr) return;
    const std::lock_guard<std::mutex> lockGuard(_mutex);
    *reinterpret_cast<unsigned char**>(ptr) = _head;
    _head = reinterpret_cast<unsigned char*>(ptr);
  }

  std::size_t freeMemory() {
    const std::lock_guard<std::mutex> lockGuard(_mutex);
    unsigned char* i = _head;
    std::size_t retVal = 0;
    while (i) {
      retVal += blocksize;
      i = reinterpret_cast<unsigned char**>(i)[0];
    }
    return retVal;
  }

  #ifdef MEMPOL_DEBUG
  void print() {
    std::size_t adjustedBlocksize = sizeof(std::size_t) > blocksize ? sizeof(std::size_t) : blocksize;
    std::cout << "+--------------------" << std::endl;
    std::cout << "|start:" << reinterpret_cast<void*>(_buffer) << std::endl;
    std::cout << "|blocks:" << nrBlocks << std::endl;
    std::cout << "|blocksize:" << adjustedBlocksize << std::endl;
    std::cout << "|head: " << reinterpret_cast<void*>(_head) << std::endl;
    unsigned char* currentBlock = _buffer;

    for (std::size_t i = 0; i < nrBlocks; ++i) {
      std::cout << "|" << i + 1 << ": " << reinterpret_cast<void*>(currentBlock) << std::endl;
      if (_isFree(currentBlock)) {
        std::cout << "|   free" << std::endl;
        std::cout << "|   next: " << reinterpret_cast<void*>(*reinterpret_cast<unsigned char**>(currentBlock)) << std::endl;
      } else {
        std::cout << "|   allocated" << std::endl;
      }
      currentBlock += adjustedBlocksize;
    }
    std::cout << "+--------------------" << std::endl;
  }

  bool _isFree(unsigned char* ptr) {
    unsigned char* b = _head;
    while (b) {
      if (b == ptr) return true;
      b = *reinterpret_cast<unsigned char**>(b);
    }
    return false;
  }
  #endif

 private:
  unsigned char _buffer[nrBlocks * (sizeof(std::size_t) > blocksize ? sizeof(std::size_t) : blocksize)];
  unsigned char* _head;
  std::mutex _mutex;
};

}  // end namespace MemoryPool
