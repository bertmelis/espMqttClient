#include <iostream>

#include <unity.h>

#include <Packets/Property.h>

void setUp() {}
void tearDown() {}



void test_initialize() {
  uint8_t data[2] = {0x01, 0x02};
  espMqttClientTypes::PropertyCollection properties(espMqttClientTypes::PropertyId.PayloadFormatIndicator, 0x01,
                                                    espMqttClientTypes::PropertyId.MessageExpiryInterval, 30,
                                                    espMqttClientTypes::PropertyId.CorrelationData, data, 2,
                                                    espMqttClientTypes::PropertyId.ServerKeepAlive, 1234,
                                                    espMqttClientTypes::PropertyId.AssignedClientId, "test",
                                                    espMqttClientTypes::PropertyId.UserProperty, "key", "value");
  const size_t numberProperties = 6;

  TEST_ASSERT_EQUAL_UINT32(numberProperties, properties.numberProperties());
}

void test_serialize() {
  uint8_t data[2] = {0x01, 0x02};
  espMqttClientTypes::PropertyCollection properties(espMqttClientTypes::PropertyId.PayloadFormatIndicator, 0x01,     // 1 + 1
                                                    espMqttClientTypes::PropertyId.MessageExpiryInterval, 30,        // 1 + 4
                                                    espMqttClientTypes::PropertyId.CorrelationData, data, 2,         // 1 + 2 + 2
                                                    espMqttClientTypes::PropertyId.ServerKeepAlive, 1234,            // 1 + 2
                                                    espMqttClientTypes::PropertyId.AssignedClientId, "test",         // 1 + 2 + 4
                                                    espMqttClientTypes::PropertyId.UserProperty, "key", "value");    // 1 + 2 + 3 + 2 + 5
  const size_t len = 35;
  const uint8_t check[] = {espMqttClientTypes::PropertyId.PayloadFormatIndicator, 0x01,
                           espMqttClientTypes::PropertyId.MessageExpiryInterval, 0x00, 0x00, 0x00, 0x1E,
                           espMqttClientTypes::PropertyId.CorrelationData, 0x00, 0x02, 0x01, 0x02,
                           espMqttClientTypes::PropertyId.ServerKeepAlive, 0x04, 0xD2,
                           espMqttClientTypes::PropertyId.AssignedClientId, 0x00, 0x04, 't', 'e', 's', 't',
                           espMqttClientTypes::PropertyId.UserProperty, 0x00, 0x03, 'k' ,'e', 'y', 0x00, 0x05, 'v', 'a', 'l', 'u', 'e'};
  size_t bufferSize = properties.length();
  uint8_t* testBuffer = new uint8_t[bufferSize];
  properties.serialize(testBuffer);

  TEST_ASSERT_EQUAL_UINT32(len, properties.length());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, testBuffer, std::min(bufferSize, len));

  delete[] testBuffer;
}

void test_deserialize() {
  const uint8_t input[] = {espMqttClientTypes::PropertyId.PayloadFormatIndicator, 0x01,
                           espMqttClientTypes::PropertyId.MessageExpiryInterval, 0x00, 0x00, 0x00, 0x1E,
                           espMqttClientTypes::PropertyId.CorrelationData, 0x00, 0x02, 0x01, 0x02,
                           espMqttClientTypes::PropertyId.ServerKeepAlive, 0x04, 0xD2,
                           espMqttClientTypes::PropertyId.AssignedClientId, 0x00, 0x04, 't', 'e', 's', 't',
                           espMqttClientTypes::PropertyId.UserProperty, 0x00, 0x03, 'k' ,'e', 'y', 0x00, 0x05, 'v', 'a', 'l', 'u', 'e'};
  const size_t len = 35;
  const size_t numberProperties = 6;
  espMqttClientTypes::PropertyCollection properties;
  bool result = properties.deserialize(input, len);

  TEST_ASSERT(result);
  TEST_ASSERT_EQUAL_UINT32(numberProperties, properties.numberProperties());
}

void test_getting_initialized() {
  uint8_t data[2] = {0x01, 0x02};
  espMqttClientTypes::PropertyCollection properties(espMqttClientTypes::PropertyId.PayloadFormatIndicator, 0x01,
                                                    espMqttClientTypes::PropertyId.MessageExpiryInterval, 30,
                                                    espMqttClientTypes::PropertyId.CorrelationData, data, 2,
                                                    espMqttClientTypes::PropertyId.ServerKeepAlive, 1234,
                                                    espMqttClientTypes::PropertyId.AssignedClientId, "test",
                                                    espMqttClientTypes::PropertyId.UserProperty, "key", "value");

  const char* clientId = nullptr;
  int num = properties.numberProperties();
  const espMqttClientTypes::PropertyItem* p = properties.get();
  while (num > 0) {
    if (p->propertyId == espMqttClientTypes::PropertyId.AssignedClientId) {
      clientId = p->data.utf8String;
    }
    ++p;
    --num;
  }

  uint16_t serverKeepAlive = 0;
  num = properties.numberProperties();
  p = properties.get();
  while (num > 0) {
    if (p->propertyId == espMqttClientTypes::PropertyId.ServerKeepAlive) {
      serverKeepAlive = p->data.twoByteCode;
    }
    ++p;
    --num;
  }

  TEST_ASSERT_EQUAL_STRING("test", clientId);
  TEST_ASSERT_EQUAL_UINT16(1234, serverKeepAlive);
}

void test_getting_deserialized() {
  const uint8_t input[] = {espMqttClientTypes::PropertyId.PayloadFormatIndicator, 0x01,
                           espMqttClientTypes::PropertyId.MessageExpiryInterval, 0x00, 0x00, 0x00, 0x1E,
                           espMqttClientTypes::PropertyId.CorrelationData, 0x00, 0x02, 0x01, 0x02,
                           espMqttClientTypes::PropertyId.ServerKeepAlive, 0x04, 0xD2,
                           espMqttClientTypes::PropertyId.AssignedClientId, 0x00, 0x04, 't', 'e', 's', 't',
                           espMqttClientTypes::PropertyId.UserProperty, 0x00, 0x03, 'k' ,'e', 'y', 0x00, 0x05, 'v', 'a', 'l', 'u', 'e'};
  const size_t len = 35;
  espMqttClientTypes::PropertyCollection properties;
  properties.deserialize(input, len);

  const char* clientId = nullptr;
  int num = properties.numberProperties();
  const espMqttClientTypes::PropertyItem* p = properties.get();
  while (num > 0) {
    if (p->propertyId == espMqttClientTypes::PropertyId.AssignedClientId) {
      clientId = p->data.utf8String;
    }
    ++p;
    --num;
  }
  uint16_t serverKeepAlive = 0;
  num = properties.numberProperties();
  p = properties.get();
  while (num > 0) {
    if (p->propertyId == espMqttClientTypes::PropertyId.ServerKeepAlive) {
      serverKeepAlive = p->data.twoByteCode;
    }
    ++p;
    --num;
  }

  TEST_ASSERT_EQUAL_STRING("test", clientId);
  TEST_ASSERT_EQUAL_UINT16(1234, serverKeepAlive);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_initialize);
  RUN_TEST(test_serialize);
  RUN_TEST(test_deserialize);
  RUN_TEST(test_getting_initialized);
  RUN_TEST(test_getting_deserialized);
  return UNITY_END();
}
