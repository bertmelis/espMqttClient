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

  Packet packet(cleanSession,
                username,
                password,
                willTopic,
                willRemain,
                willQoS,
                willPayload,
                willPayloadLength,
                keepalive,
                clientId);

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

  Packet packet(topic,
                payload,
                payloadLength,
                qos,
                retain,
                packetId);

  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(packetId, packet.packetId());
}

void test_encodePubAck() {
  const uint8_t check[] = {
    0b01000000,                 // header
    0x02,
    0x00,0x16,                  // packet Id
  };
  const uint32_t length = 4;

  uint16_t packetId = 22;

  Packet packet(PacketType.PUBACK, packetId);

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

  Packet packet(PacketType.PUBREC, packetId);

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

  Packet packet(PacketType.PUBREL, packetId);

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

  Packet packet(PacketType.PUBCOMP, packetId);

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

  Packet packet(topic, qos, packetId);

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

  Packet packet(PacketType.PINGREQ);

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

  Packet packet(PacketType.DISCONNECT);

  TEST_ASSERT_EQUAL_UINT32(length, packet.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(check, packet.data(0), length);
  TEST_ASSERT_EQUAL_UINT16(0, packet.packetId());
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
  RUN_TEST(test_encodePingReq);
  RUN_TEST(test_encodeDisconnect);
  return UNITY_END();
}
