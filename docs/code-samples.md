# Code samples

A number of examples are in the [examples](/examples) directory. These include basic operation on ESP8266 and ESP32. Please examine these to understand the basic operation of the MQTT client.

Below are examples on specific points for working with this library

## Printing payloads

MQTT 3.1.1 payloads have no format and need to be treated as binary. If you want to print a payload to the Arduino serial console, you have to make sure that the payload is null-terminated (c-strin).

```cpp
void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total) {
  Serial.println("Publish received:");
  Serial.printf("  topic: %s\n  payload:", topic);
  const char* p = reinterpret_cast<const char*>(payload);
  for (size_t i = 0; i < len; ++i) {
    Serial.print(p[i]);
  }
  Serial.print("\n");
}
```

```cpp
void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total) {
  Serial.println("Publish received:");
  Serial.printf("  topic: %s\n  payload:", topic);
  char* p = new char[len + 1];
  memcpy(p, payload, len);
  payload[len] = "\0";
  Serial.println(p);
  delete[] p;
}
```

## Assembling chunked messages

The `onMessage`-callback is called as data comes in. So if the data comes in chuncked, the callback will be called on every receipt on a chunk, with the proper `index`, (chunk)`size` and `total` set. With little code, you can reassemble chunked messages yourself.

```cpp

const size_t maxPayloadSize = 8192;
uint8_t* payloadbuffer = nullptr;
size_t payloadbufferSize = 0;
size_t payloadbufferIndex = 0;

void onOversizedMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total) {
  // handle oversized messages
}

void onCompleteMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total) {
  // handle oversized messages
}

void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total) {
  // payload is bigger then max: return chunked
  if (total > maxPayloadSize) {
    onOversizedMqttMessage(properties, topic, payload, len, index, total);
    return;
  }

  // start new packet, increase buffer size if neccesary
  if (index == 0 || !payloadBuffer) {
    if (total > payloadbufferSize) {
      delete[] payloadbuffer;
      payloadbufferSize = total;
      payloadbuffer = new uint8_t[payloadbufferSize];  // you may want to check for nullptr after this operation
    }
    payloadbufferIndex = 0;
  }

  // add data and dispatch when done
  memcpy(&payloadbuffer[payloadbufferIndex], payload, len);
  payloadbufferIndex += len;
  if (payloadbufferIndex == total) {
    // message is complete here
    onCompleteMqttMessage(properties, topic, payloadBuffer, total, 0, total);
    // optionally:
    delete[] payloadBuffer;
    payloadBuffer = nullptr;
    payloadbufferSize = 0;
  }
}
```
