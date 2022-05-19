#include <unity.h>

#include <Outbox.h>

using espMqttClientInternals::Outbox;

/*
void printOutbox(Outbox* outbox) {
  std::cout << std::endl << "outbox: ";
  while (outbox->get(i)) {
    std::cout << *(outbox->get(i++)) << " ";
  }
  if (outbox->getCurrent()) {
    std::cout << " - current: " << *(outbox->getCurrent());
  }
  std::cout << std::endl;
}
*/

void test_outbox_create() {
  Outbox<uint32_t> outbox;
  Outbox<uint32_t>::Iterator it = outbox.front();
  TEST_ASSERT_NULL(outbox.getCurrent());
  TEST_ASSERT_NULL(it.data());
  TEST_ASSERT_TRUE(outbox.empty());
}

void test_outbox_add() {
  Outbox<uint32_t> outbox;
  outbox.add(1);
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(1, *(outbox.getCurrent()));
  TEST_ASSERT_FALSE(outbox.empty());

  outbox.next();
  outbox.add(2);
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(2, *(outbox.getCurrent()));

  outbox.add(3);
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(2, *(outbox.getCurrent()));
}

void test_outbox_addFront() {
  Outbox<uint32_t> outbox;
  outbox.addFront(1);
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(1, *(outbox.getCurrent()));

  outbox.addFront(2);
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(2, *(outbox.getCurrent()));
}

void test_outbox_remove1() {
  Outbox<uint32_t> outbox;
  Outbox<uint32_t>::Iterator it;
  outbox.add(1);
  outbox.add(2);
  outbox.add(3);
  outbox.add(4);
  outbox.next();
  outbox.next();
  it = outbox.front();
  ++it;
  ++it;
  ++it;
  ++it;
  outbox.remove(it);
  // 1 2 3 4, it points to nullptr, current points to 3
  TEST_ASSERT_NULL(it.data());
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(3, *(outbox.getCurrent()));

  it = outbox.front();
  ++it;
  ++it;
  ++it;
  outbox.remove(it);
  // 1 2 3, it points to nullptr, current points to 3
  TEST_ASSERT_NULL(it.data());
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(3, *(outbox.getCurrent()));


  it = outbox.front();
  outbox.remove(it);
  // 2 3, it points to 2, current points to 3
  TEST_ASSERT_NOT_NULL(it.data());
  TEST_ASSERT_EQUAL_UINT32(2, *(it.data()));
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(3, *(outbox.getCurrent()));

  it = outbox.front();
  outbox.remove(it);
  // 3, it points to 3, current points to 3
  TEST_ASSERT_NOT_NULL(it.data());
  TEST_ASSERT_EQUAL_UINT32(3, *(it.data()));
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(3, *(outbox.getCurrent()));

  it = outbox.front();
  outbox.remove(it);
  TEST_ASSERT_NULL(it.data());
  TEST_ASSERT_NULL(outbox.getCurrent());
}

void test_outbox_remove2() {
  Outbox<uint32_t> outbox;
  Outbox<uint32_t>::Iterator it;
  outbox.add(1);
  outbox.add(2);
  outbox.next();
  outbox.next();
  it = outbox.front();
  // 1 2, current points to nullptr
  TEST_ASSERT_NULL(outbox.getCurrent());
  TEST_ASSERT_NOT_NULL(it.data());
  TEST_ASSERT_EQUAL_UINT32(1, *(it.data()));

  ++it;
  // 1 2, current points to nullptr
  TEST_ASSERT_NOT_NULL(it.data());
  TEST_ASSERT_EQUAL_UINT32(2, *(it.data()));

  outbox.remove(it);
  // 1, current points to nullptr
  TEST_ASSERT_NULL(outbox.getCurrent());
  TEST_ASSERT_NULL(it.data());

  it = outbox.front();
  TEST_ASSERT_NOT_NULL(it.data());
  TEST_ASSERT_EQUAL_UINT32(1, *(it.data()));

  outbox.remove(it);
  TEST_ASSERT_NULL(it.data());
  TEST_ASSERT_TRUE(outbox.empty());
}

void test_outbox_removeCurrent() {
  Outbox<uint32_t> outbox;
  outbox.add(1);
  outbox.add(2);
  outbox.add(3);
  outbox.add(4);
  outbox.removeCurrent();
  // 2 3 4, current points to 2
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(2, *(outbox.getCurrent()));

  outbox.next();
  outbox.removeCurrent();
  // 2 4, current points to 4
  TEST_ASSERT_NOT_NULL(outbox.getCurrent());
  TEST_ASSERT_EQUAL_UINT32(4, *(outbox.getCurrent()));

  outbox.removeCurrent();
  // 4, current points to nullptr
  TEST_ASSERT_NULL(outbox.getCurrent());

  // outbox will go out of scope and destructor will be called
  // Valgrind should not detect a leak here
}

void test_outbox_clear() {
  Outbox<uint32_t> outbox;
  Outbox<uint32_t>::Iterator it;
  outbox.add(1);
  outbox.add(2);
  outbox.add(3);
  outbox.clear();
  it = outbox.front();
  TEST_ASSERT_TRUE(outbox.empty());
  TEST_ASSERT_NULL(outbox.getCurrent());
  TEST_ASSERT_NULL(it.data());
}

int test_outbox() {
  UnityBegin("Outbox");
  RUN_TEST(test_outbox_create);
  RUN_TEST(test_outbox_add);
  RUN_TEST(test_outbox_addFront);
  RUN_TEST(test_outbox_remove1);
  RUN_TEST(test_outbox_remove2);
  RUN_TEST(test_outbox_removeCurrent);
  RUN_TEST(test_outbox_clear);
  return UNITY_END();
}