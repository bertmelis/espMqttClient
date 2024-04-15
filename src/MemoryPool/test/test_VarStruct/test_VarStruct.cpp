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

struct MemTestStruct {
  size_t size;
  int value;
  unsigned char* data[10];
};

constexpr const size_t blockHeadersize = sizeof(void*) + sizeof(size_t);

void emptyPool() {
  const size_t nrBlocks = 3;
  const size_t blocksize = sizeof(MemTestStruct);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  TEST_ASSERT_EQUAL_UINT(nrBlocks * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(nrBlocks * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

void mallocFull() {
  const size_t nrBlocks = 3;
  const size_t blocksize = sizeof(MemTestStruct);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  MemTestStruct* var1 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var2 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var3 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var4 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  TEST_ASSERT_NOT_NULL(var1);
  TEST_ASSERT_NOT_NULL(var2);
  TEST_ASSERT_NOT_NULL(var3);
  TEST_ASSERT_NULL(var4);

  var1->value = 1;
  var2->value = 2;
  var3->value = 3;
  TEST_ASSERT_EQUAL_INT(1, var1->value);
  TEST_ASSERT_EQUAL_INT(2, var2->value);
  TEST_ASSERT_EQUAL_INT(3, var3->value);
  TEST_ASSERT_EQUAL_UINT(0, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(0, pool.maxBlockSize());
}

void mallocMultiFull() {
  const size_t nrBlocks = 4;
  const size_t blocksize = sizeof(MemTestStruct);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  MemTestStruct* var1 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var2 = reinterpret_cast<MemTestStruct*>(pool.malloc(2 * blocksize));
  MemTestStruct* var3 = reinterpret_cast<MemTestStruct*>(pool.malloc(2 * blocksize));
  MemTestStruct* var4 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));

  pool.print();

  TEST_ASSERT_NOT_NULL(var1);
  TEST_ASSERT_NOT_NULL(var2);
  TEST_ASSERT_NULL(var3);
  TEST_ASSERT_NOT_NULL(var4);

  var1->value = 1;
  var2->value = 2;
  var4->value = 4;
  TEST_ASSERT_EQUAL_INT(1, var1->value);
  TEST_ASSERT_EQUAL_INT(2, var2->value);
  TEST_ASSERT_EQUAL_INT(4, var4->value);
  TEST_ASSERT_EQUAL_UINT(0, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(0, pool.maxBlockSize());
}

void freeAppend() {
  const size_t nrBlocks = 5;
  const size_t blocksize = sizeof(MemTestStruct);
  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  MemTestStruct* var1 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var2 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var3 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var4 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var5 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  (void) var1;
  pool.free(var2);
  TEST_ASSERT_EQUAL_UINT(1 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  pool.free(var3);
  TEST_ASSERT_EQUAL_UINT(2 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  pool.free(var4);
  (void) var5;

  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

void freePrepend() {
  const size_t nrBlocks = 5;
  const size_t blocksize = sizeof(MemTestStruct);
  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  MemTestStruct* var1 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var2 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var3 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var4 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var5 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  (void) var1;
  pool.free(var4);
  pool.free(var3);
  pool.free(var2);
  (void) var5;

  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.freeMemory());
  TEST_ASSERT_EQUAL_UINT(3 * (adjustedBlocksize + blockHeadersize) - blockHeadersize, pool.maxBlockSize());
}

void freeEmpty() {
  const size_t nrBlocks = 5;
  const size_t blocksize = sizeof(MemTestStruct);
  size_t adjustedBlocksize = std::max(blocksize, blockHeadersize);
  MemoryPool::Variable<nrBlocks, blocksize> pool;

  MemTestStruct* var1 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var2 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var3 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var4 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));
  MemTestStruct* var5 = reinterpret_cast<MemTestStruct*>(pool.malloc(blocksize));

  pool.print();
  pool.free(var1);
  pool.print();
  pool.free(var2);
  pool.print();
  pool.free(var3);
  pool.print();
  pool.free(var5);
  pool.print();
  pool.free(var4);
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
