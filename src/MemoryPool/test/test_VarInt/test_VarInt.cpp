/*
Copyright (c) 2024 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <unity.h>

#include <MemoryPool.h>

void setUp() {}
void tearDown() {}

constexpr const size_t blockHeadersize = sizeof(void*) + sizeof(size_t);

void emptyPool() {
  const size_t nrBlocks = 3;
  const size_t blocksize = sizeof(int);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);

  pool.print();

  TEST_ASSERT_EQUAL_UINT(nrBlocks * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(nrBlocks * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

void mallocFull() {
  const size_t nrBlocks = 3;
  const size_t blocksize = sizeof(int);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int2 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int3 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int4 = reinterpret_cast<int*>(pool.malloc(blocksize));

  pool.print();

  TEST_ASSERT_NOT_NULL(int1);
  TEST_ASSERT_NOT_NULL(int2);
  TEST_ASSERT_NOT_NULL(int3);
  TEST_ASSERT_NULL(int4);

  *int1 = 1;
  *int2 = 2;
  *int3 = 3;
  TEST_ASSERT_EQUAL_INT(1, *int1);
  TEST_ASSERT_EQUAL_INT(2, *int2);
  TEST_ASSERT_EQUAL_INT(3, *int3);
  TEST_ASSERT_EQUAL_UINT(0, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(0, pool.maxBlockSize());
}

void mallocMultiFull() {
  // mind sizeof(int) != sizeof(internal block)
  // on amd64: int = 2, internal blocksize = 16

  const size_t nrBlocks = 4;
  const size_t blocksize = sizeof(int);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int2 = reinterpret_cast<int*>(pool.malloc(8 * blocksize));
  int* int3 = reinterpret_cast<int*>(pool.malloc(8 * blocksize + 1));
  int* int4 = reinterpret_cast<int*>(pool.malloc(blocksize));

  pool.print();

  TEST_ASSERT_NOT_NULL(int1);
  TEST_ASSERT_NOT_NULL(int2);
  TEST_ASSERT_NULL(int3);
  TEST_ASSERT_NOT_NULL(int4);

  *int1 = 1;
  *int2 = 2;
  *int4 = 4;
  TEST_ASSERT_EQUAL_INT(1, *int1);
  TEST_ASSERT_EQUAL_INT(2, *int2);
  TEST_ASSERT_EQUAL_INT(4, *int4);
  TEST_ASSERT_EQUAL_UINT(0, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(0, pool.maxBlockSize());
}

void freeAppend() {
  const size_t nrBlocks = 5;
  const size_t blocksize = sizeof(int);
  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int2 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int3 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int4 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int5 = reinterpret_cast<int*>(pool.malloc(blocksize));
  pool.print();

  (void) int1;
  pool.free(int2);
  pool.print();
  TEST_ASSERT_EQUAL_UINT(1 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  pool.free(int3);
  pool.print();
  TEST_ASSERT_EQUAL_UINT(2 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  pool.free(int4);
  pool.print();
  (void) int5;

  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

void freePrepend() {
  const size_t nrBlocks = 5;
  const size_t blocksize = sizeof(int);
  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int2 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int3 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int4 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int5 = reinterpret_cast<int*>(pool.malloc(blocksize));
  pool.print();

  (void) int1;
  pool.free(int4);
  pool.print();
  pool.free(int3);
  pool.print();
  pool.free(int2);
  pool.print();
  (void) int5;

  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

void freeEmpty() {
  const size_t nrBlocks = 5;
  const size_t blocksize = sizeof(int);
  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int2 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int3 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int4 = reinterpret_cast<int*>(pool.malloc(blocksize));
  int* int5 = reinterpret_cast<int*>(pool.malloc(blocksize));

  pool.print();
  pool.free(int1);
  pool.print();
  pool.free(int2);
  pool.print();
  pool.free(int3);
  pool.print();
  pool.free(int5);
  pool.print();
  pool.free(int4);
  pool.print();

  TEST_ASSERT_EQUAL_UINT(nrBlocks * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(nrBlocks * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(emptyPool);
  RUN_TEST(mallocFull);
  RUN_TEST(mallocMultiFull);
  RUN_TEST(freeAppend);
  RUN_TEST(freePrepend);
  RUN_TEST(freeEmpty);
  return UNITY_END();
}
