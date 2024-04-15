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

void emptyPool() {
  const size_t nrBlocks = 3;
  const size_t blocksize = sizeof(int);
  MemoryPool::Fixed<nrBlocks, blocksize> pool;

  pool.print();

  TEST_ASSERT_EQUAL_UINT(nrBlocks * blocksize, pool.freeMemory());
}

void mallocFull() {
  const size_t nrBlocks = 3;
  const size_t blocksize = sizeof(int);
  MemoryPool::Fixed<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc());
  int* int2 = reinterpret_cast<int*>(pool.malloc());
  int* int3 = reinterpret_cast<int*>(pool.malloc());
  int* int4 = reinterpret_cast<int*>(pool.malloc());

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
}

void freePartial() {
  const size_t nrBlocks = 4;
  const size_t blocksize = sizeof(int);
  MemoryPool::Fixed<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc());
  int* int2 = reinterpret_cast<int*>(pool.malloc());
  int* int3 = reinterpret_cast<int*>(pool.malloc());
  int* int4 = reinterpret_cast<int*>(pool.malloc());
  int* int5 = reinterpret_cast<int*>(pool.malloc());
  pool.print();

  (void) int1;
  (void) int3;
  pool.free(int2);
  pool.print();
  TEST_ASSERT_EQUAL_UINT(1 * blocksize, pool.freeMemory());
  pool.free(int4);
  pool.print();
  TEST_ASSERT_EQUAL_UINT(2 * blocksize, pool.freeMemory());
  int5 = reinterpret_cast<int*>(pool.malloc());
  TEST_ASSERT_NOT_NULL(int5);
  pool.print();

  TEST_ASSERT_EQUAL_UINT(1 * blocksize, pool.freeMemory());
}

void freeEmpty() {
  const size_t nrBlocks = 4;
  const size_t blocksize = sizeof(int);
  MemoryPool::Fixed<nrBlocks, blocksize> pool;

  int* int1 = reinterpret_cast<int*>(pool.malloc());
  int* int2 = reinterpret_cast<int*>(pool.malloc());
  int* int3 = reinterpret_cast<int*>(pool.malloc());
  int* int4 = reinterpret_cast<int*>(pool.malloc());

  pool.print();
  pool.free(int1);
  pool.print();
  pool.free(int2);
  pool.print();
  pool.free(int3);
  pool.print();
  pool.free(int4);
  pool.print();

  TEST_ASSERT_EQUAL_UINT(nrBlocks * blocksize, pool.freeMemory());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(emptyPool);
  RUN_TEST(mallocFull);
  RUN_TEST(freePartial);
  RUN_TEST(freeEmpty);
  return UNITY_END();
}
