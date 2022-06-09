#include <unity.h>

#include <Packets/Packet.h>

using espMqttClientInternals::Packet;
using espMqttClientInternals::PacketType;

void setUp() {}
void tearDown() {}

void test_encodeConnect() {
  const uint8_t check[] = {
    0b00010000,                 // header
    0x20,                       // remaining length
    0x00,0x04,'M','Q','T','T',  // protocol
    0b00000100,                 // protocol level
    0b11101110,                 // connect flags
    0x00,0x10,                  // keepalive (16)
    0x00,0x03,'c','l','i',      // client id
    0x00,0x03,'t','o','p',      // will topic
    0x00,0x02,'p','l',          // will payload
    0x00,0x02,'u','n',          // username
    0x00,0x02,'p','a'           // password
  };
  const uint32_t length = 34;

  bool cleanSession = true;
  const char* username = "un";
  const char* password = "pa";
  const char* willTopic = "top";
  bool willRemain = true;
  uint8_t willQoS = 1;
  const uint8_t willPayload[] = {'p', 'l'};
  uint16_t willPayloadLength = 2;
  uint16_t keepalive = 16;
  const char* clientId = "cli";
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error,
                cleanSession,
                username,
                password,
                willTopic,
                willRemain,
                willQoS,
                willPayload,
                willPayloadLength,
                keepalive,
                clientId);

  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(0, packet.packetId());
}

void test_encodePublish() {
  const uint8_t check[] = {
    0b00110011,                 // header, dup, qos, retain
    0x0B,
    0x00,0x03,'t','o','p',      // topic
    0x00,0x16,                  // packet Id
    0x01,0x02,0x03,0x04         // payload
  };
  const uint32_t length = 13;

  const char* topic = "top";
  uint8_t qos = 1;
  bool retain = true;
  const uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
  uint16_t payloadLength = 4;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error,
                topic,
                payload,
                payloadLength,
                qos,
                retain,
                packetId);

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());

  const uint8_t checkDup[] = {
    0b00111011,                 // header, dup, qos, retain
    0x0B,
    0x00,0x03,'t','o','p',      // topic
    0x00,0x16,                  // packet Id
    0x01,0x02,0x03,0x04         // payload
  };

  packet.setDup();
  TEST_ASSERT_EQUAL_UINT8_ARRAY(checkDup, packet.data(0), length);
}

void test_encodePubAck() {
  const uint8_t check[] = {
    0b01000000,                 // header
    0x02,
    0x00,0x16,                  // packet Id
  };
  const uint32_t length = 4;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, PacketType.PUBACK, packetId);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodePubRec() {
  const uint8_t check[] = {
    0b01010000,                 // header
    0x02,
    0x00,0x16,                  // packet Id
  };
  const uint32_t length = 4;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, PacketType.PUBREC, packetId);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodePubRel() {
  const uint8_t check[] = {
    0b01100010,                 // header
    0x02,
    0x00,0x16,                  // packet Id
  };
  const uint32_t length = 4;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, PacketType.PUBREL, packetId);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodePubComp() {
  const uint8_t check[] = {
    0b01110000,                 // header
    0x02,                       // remaining length
    0x00,0x16,                  // packet Id
  };
  const uint32_t length = 4;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, PacketType.PUBCOMP, packetId);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodeSubscribe() {
  const uint8_t check[] = {
    0b10000010,                 // header
    0x08,                       // remaining length
    0x00,0x16,                  // packet Id
    0x00, 0x03, 'a', '/', 'b',  // topic
    0x02                        // qos
  };
  const uint32_t length = 10;
  const char* topic = "a/b";
  uint8_t qos = 2;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, topic, qos, packetId);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodeUnubscribe() {
  const uint8_t check[] = {
    0b10100010,                 // header
    0x07,                       // remaining length
    0x00,0x16,                  // packet Id
    0x00, 0x03, 'a', '/', 'b',  // topic
  };
  const uint32_t length = 9;
  const char* topic = "a/b";
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, topic, packetId);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodePingReq() {
  const uint8_t check[] = {
    0b11000000,                 // header
    0x00
  };
  const uint32_t length = 2;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, PacketType.PINGREQ);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(0, packet.packetId());
}

void test_encodeDisconnect() {
  const uint8_t check[] = {
    0b11100000,                 // header
    0x00
  };
  const uint32_t length = 2;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error, PacketType.DISCONNECT);
  packet.setDup();  // no effect

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(0, packet.packetId());
}

size_t getData(uint8_t* dest, size_t len, size_t index) {
  static uint8_t i = 1;
  memset(dest, i, len);
  ++i;
  return len;
}

void test_encodeChunkedPublish() {
  const uint8_t check[] = {
    0b00110011,                 // header, dup, qos, retain
    0xCF, 0x01,                 // 7 + 200 = (0x4F * 1) & 0x40 + (0x01 * 128)
    0x00,0x03,'t','o','p',      // topic
    0x00,0x16                   // packet Id
  };
  uint8_t payloadChunk[EMC_RX_BUFFER_SIZE] = {};
  memset(payloadChunk, 0x01, EMC_RX_BUFFER_SIZE);
  const char* topic = "top";
  uint8_t qos = 1;
  bool retain = true;
  size_t headerLength = 10;
  size_t payloadLength = 200;
  size_t size = headerLength + payloadLength;
  uint16_t packetId = 22;
  espMqttClientTypes::Error error = espMqttClientTypes::Error::OUT_OF_MEMORY;

  Packet packet(error,
                topic,
                getData,
                payloadLength,
                qos,
                retain,
                packetId);

  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::Error::SUCCESS, error);
  TEST_ASSERT_EQUAL_UINT32(size, packet.size());
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());

  size_t available = 0;
  size_t index = 0;

  // call 'available' before 'data'
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(headerLength + EMC_RX_BUFFER_SIZE, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(index), headerLength);

  // index == first payload byte
  index = headerLength;
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(EMC_RX_BUFFER_SIZE, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payloadChunk, packet.data(index), available);

  // index == first payload byte
  index = headerLength + 4;
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(EMC_RX_BUFFER_SIZE - 4, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payloadChunk, packet.data(index), available);

  // index == last payload byte in first chunk
  index = headerLength + EMC_RX_BUFFER_SIZE - 1;
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(1, available);

  // index == first payloadbyte in second chunk
  memset(payloadChunk, 0x02, EMC_RX_BUFFER_SIZE);
  index = headerLength + EMC_RX_BUFFER_SIZE;
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(EMC_RX_BUFFER_SIZE, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payloadChunk, packet.data(index), available);

  memset(payloadChunk, 0x03, EMC_RX_BUFFER_SIZE);
  index = headerLength + EMC_RX_BUFFER_SIZE + EMC_RX_BUFFER_SIZE + 10;
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(EMC_RX_BUFFER_SIZE, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payloadChunk, packet.data(index), available);

  const uint8_t checkDup[] = {
    0b00111011,                 // header, dup, qos, retain
    0xCF, 0x01,                 // 7 + 200 = (0x4F * 0) + (0x01 * 128)
    0x00,0x03,'t','o','p',      // topic
    0x00,0x16,                  // packet Id
  };

  index = 0;
  packet.setDup();
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(headerLength + EMC_RX_BUFFER_SIZE, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(checkDup, packet.data(index), headerLength);

  memset(payloadChunk, 0x04, EMC_RX_BUFFER_SIZE);
  index = headerLength;
  available = packet.available(index);
  TEST_ASSERT_EQUAL_UINT32(EMC_RX_BUFFER_SIZE, available);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payloadChunk, packet.data(index), available);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_encodeConnect);
  RUN_TEST(test_encodePublish);
  RUN_TEST(test_encodePubAck);
  RUN_TEST(test_encodePubRec);
  RUN_TEST(test_encodePubRel);
  RUN_TEST(test_encodePubComp);
  RUN_TEST(test_encodeSubscribe);
  RUN_TEST(test_encodeUnubscribe);
  RUN_TEST(test_encodePingReq);
  RUN_TEST(test_encodeDisconnect);
  RUN_TEST(test_encodeChunkedPublish);
  return UNITY_END();
}
