#include <iostream>

#include <unity.h>

#include "tests/test_outbox.cpp"
#include "tests/test_packets.cpp"
#include "tests/test_parser.cpp"
#include "tests/test_remainingLength.cpp"
#include "tests/test_string.cpp"

#define RUN_TEST_FUCTION(test) retval = test(); if (retval) return retval; std::cout << std::endl;

void setUp(void) {
  // empty
}
void tearDown(void) {
  // empty
}

int main (void) {
  int retval = 0;
  
  RUN_TEST_FUCTION(test_outbox);
  RUN_TEST_FUCTION(test_packets);
  RUN_TEST_FUCTION(test_parser);
  RUN_TEST_FUCTION(test_remainingLength);
  RUN_TEST_FUCTION(test_string);
  
  return retval;
}