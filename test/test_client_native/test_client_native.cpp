#include <unity.h>
#include <thread>
#include <iostream>
#include <espMqttClient.h>  // espMqttClient for Linux also defines millis()

void setUp() {}
void tearDown() {}

espMqttClient mqttClient;
std::atomic_bool exitProgram(false);
std::thread t;

//const IPAddress broker(192,168,1,10);
const char* broker = "mosquittomqtt";
const uint16_t broker_port = 1883;

/*

- setup the client with basic settings
- connect to the broker
- successfully connect

*/
void test_connect() {
  std::atomic<bool> onConnectCalledTest(false);
  bool sessionPresentTest = true;
  mqttClient.setServer(broker, broker_port)
            .setCleanSession(true)
            .setKeepAlive(5)
            .onConnect([&](bool sessionPresent) mutable {
              sessionPresentTest = sessionPresent;
              onConnectCalledTest = true;
            });
  mqttClient.connect();
  uint32_t start = millis();
  while (millis() - start < 2000) {
    if (onConnectCalledTest) {
      break;
    }
    std::this_thread::yield();
  }

  TEST_ASSERT_TRUE(mqttClient.connected());
  TEST_ASSERT_TRUE(onConnectCalledTest);
  TEST_ASSERT_FALSE(sessionPresentTest);
}

/*

- keepalive is set at 5 seconds in previous test
- client should stay connected during 2x keepalive period

*/

void test_ping() {
  bool pingTest = true;
  uint32_t start = millis();
  while (millis() - start < 11000) {
    if (mqttClient.disconnected()) {
      pingTest = false;
      break;
    }
    std::this_thread::yield();
  }

  TEST_ASSERT_TRUE(mqttClient.connected());
  TEST_ASSERT_TRUE(pingTest);
}

/*

- client subscribes to topic
- ack is received from broker

*/

void test_subscribe() {
  std::atomic<bool> subscribeTest(false);
  mqttClient.onSubscribe([&](uint16_t packetId, const espMqttClientTypes::SubscribeReturncode* returncodes, size_t len) mutable {
    (void) packetId;
    if (len == 1 && returncodes[0] == espMqttClientTypes::SubscribeReturncode::QOS0) {
      subscribeTest = true;
    }
  });
  mqttClient.subscribe("test/test", 0);
  uint32_t start = millis();
  while (millis() - start < 2000) {
    if (subscribeTest) {
      break;
    }
    std::this_thread::yield();
  }

  TEST_ASSERT_TRUE(mqttClient.connected());
  TEST_ASSERT_TRUE(subscribeTest);
}

/*

- client publishes using all three qos levels
- all publish get packetID returned > 0 (equal to 1 for qos 0)
- 2 pubacks are received

*/

void test_publish() {
  std::atomic<int> publishReceiveTest(0);
  mqttClient.onPublish([&](uint16_t packetId) mutable {
    (void) packetId;
    publishReceiveTest++;
  });
  uint16_t sendQos0Test = mqttClient.publish("test/test", 0, false, "test0");
  uint16_t sendQos1Test = mqttClient.publish("test/test", 1, false, "test1");
  uint16_t sendQos2Test = mqttClient.publish("test/test", 2, false, "test2");
  uint32_t start = millis();
  while (millis() - start < 2000) {
    if (publishReceiveTest == 3) {
      break;
    }
    std::this_thread::yield();
  }

  TEST_ASSERT_TRUE(mqttClient.connected());
  TEST_ASSERT_EQUAL_UINT16(1, sendQos0Test);
  TEST_ASSERT_GREATER_THAN_UINT16(0, sendQos1Test);
  TEST_ASSERT_GREATER_THAN_UINT16(0, sendQos2Test);
  TEST_ASSERT_EQUAL_INT(2, publishReceiveTest);
}

/*

- client unsibscribes from topic

*/

void test_unsubscribe() {
  std::atomic<bool> unsubscribeTest(false);
  mqttClient.onUnsubscribe([&](uint16_t packetId) mutable {
    (void) packetId;
    unsubscribeTest = true;
  });
  mqttClient.unsubscribe("test/test");
  uint32_t start = millis();
  while (millis() - start < 2000) {
    if (unsubscribeTest) {
      break;
    }
    std::this_thread::yield();
  }

  TEST_ASSERT_TRUE(mqttClient.connected());
  TEST_ASSERT_TRUE(unsubscribeTest);
}

/*

- client disconnects cleanly

*/

void test_disconnect() {
  std::atomic<bool> onDisconnectCalled(false);
  espMqttClientTypes::DisconnectReason reasonTest = espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED;
  mqttClient.onDisconnect([&](espMqttClientTypes::DisconnectReason reason) mutable {
    reasonTest = reason;
    onDisconnectCalled = true;
  });
  mqttClient.disconnect();
  uint32_t start = millis();
  while (millis() - start < 2000) {
    if (onDisconnectCalled) {
      break;
    }
    std::this_thread::yield();
  }

  TEST_ASSERT_TRUE(onDisconnectCalled);
  TEST_ASSERT_EQUAL_UINT8(espMqttClientTypes::DisconnectReason::USER_OK, reasonTest);
  TEST_ASSERT_TRUE(mqttClient.disconnected());
}

int main() {
  UNITY_BEGIN();
  t = std::thread([] {
    while (1) {
      mqttClient.loop();
      if (exitProgram) break;
    }
  });
  RUN_TEST(test_connect);
  RUN_TEST(test_ping);
  RUN_TEST(test_subscribe);
  RUN_TEST(test_publish);
  RUN_TEST(test_unsubscribe);
  RUN_TEST(test_disconnect);
  exitProgram = true;
  t.join();
  return UNITY_END();
}
