![platformio](https://github.com/bertmelis/espMqttClient/actions/workflows/build_platformio.yml/badge.svg)
![cpplint](https://github.com/bertmelis/espMqttClient/actions/workflows/cpplint.yml/badge.svg)
![cppcheck](https://github.com/bertmelis/espMqttClient/actions/workflows/cppcheck.yml/badge.svg)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/bertmelis/library/espMqttClient.svg)](https://registry.platformio.org/libraries/bertmelis/espMqttClient)

# Features

- MQTT 3.1.1 compliant library
- Sending and receiving at all QoS levels
- TCP and TCP/TLS using standard WiFiClient and WiFiClientSecure connections
- Virtually unlimited incoming and outgoing payload sizes
- Readable and understandable code
- No dependencies outside the Arduino framework

# Contents

1. [Runtime behaviour](#runtime-behaviour)
2. [API Reference](#api-reference)
3. [Compile-time configuration](#compile-time-configuration)
4. [Code samples](#code-samples)

# Runtime behaviour

A normal cycle of an MQTT client goes like this:

1. setup the client
2. connect to the broker
3. subscribe/publish/receive
4. disconnect/reconnect when disconnected

### Setup

Setting up the client means to tell which host and port to connect to, possible credentials to use and so on. espMqttClient has a set of methods to configure the client. Setup is generally done in the `setup()` function of the Arduino framework.
One important thing to remember is that there are a number of settings that are not stored inside the library: `username`, `password`, `willTopic`, `willPayload`, `clientId` and `host`. Make sure these variables stay available during the lifetime of the `espMqttClient`.

For TLS secured connections, the relevant methods from `WiFiClientSecure` have been made available.

### Connecting

After setting up the client, you are ready to connect. A simple call to `connect()` does the trick. If you set a `OnConnectCallback`, you will be notified when the connection has been made. On failure, `OnDisconnectCallback` will be called.

### Subscribing, publishing and receiving

Once connected, you can subscribe, publish and receive. The methods to do this return the packetId of the generated packet or `1` for packets without ID. In case of an error, the method returns `0`. When the client is not connected, you cannot subscribe, unsubscribe or publish.

Receiving packets is done via the `onMessage`-callback. This callback gives you the topic, properties (qos, dup, retain, packetId) and payload. For the payload, you get a pointer to the data, the index, length and total length. On long payloads it is normal that you get multiple callbacks for the same packet. This way, you can receive payloads longer than what could fit in the microcontroller's RAM-memory.

#### Payloads and strings

> Beware that MQTT payloads are binary. MQTT payloads are **not** c-strings unless explicitely constructed like that. You therefore can **not** print the payload to your Serial monitor without supporting code.

### Disconnecting

You can disconnect from the broker by calling `disconnect()`. If you do not force-disconnect, the client will first send the remaining messages that are in the queue and disconnect afterwards. During this period however, no new incoming PUBLISH messages will be processed.

You can use the [editor on GitHub](https://github.com/bertmelis/espMqttClient/edit/gh-pages/index.md) to maintain and preview the content for your website in Markdown files.

Whenever you commit to this repository, GitHub Pages will run [Jekyll](https://jekyllrb.com/) to rebuild the pages in your site, from the content in your Markdown files.

# API Reference

```cpp
espMqttClient()
espMqttClientSecure()
```

Instantiate a new espMqttClient or espMqttSecure object.
On ESP32, two optional parameters are available: `espMqttClient(uint8_t priority = 1, uint8_t core = 1)`. This will change the priority of the MQTT client task and the core on which it runs.

### Configuration

```cpp
espMqttClient& setKeepAlive(uint16_t `keepAlive`)
```

Set the keep alive. Defaults to 15 seconds.

* **`keepAlive`**: Keep alive in seconds

```cpp
espMqttClient& setClientId(const char\* `clientId`)
```

Set the client ID. Defaults to `esp8266-123456` where `123456` is the chip ID.
The library only stores a pointer to the client ID. Make sure the variable pointed to stays available throughout the lifetime of espMqttClient.

- **`clientId`**: Client ID, expects a null-terminated char array (c-string)

```cpp
espMqttClient& setCleanSession(bool `cleanSession`)
```

Set the CleanSession flag. Defaults to `true`.

- **`cleanSession`**: clean session wanted or not

```cpp
espMqttClient& setCredentials(const char\* `username`, const char\* `password`)
```

Set the username/password. Defaults to non-auth.
The library only stores a pointer to the username and password. Make sure the variable to pointed stays available throughout the lifetime of espMqttClient.

- **`username`**: Username, expects a null-terminated char array (c-string)
- **`password`**: Password, expects a null-terminated char array (c-string)

```cpp
espMqttClient& setWill(const char\* `topic`, uint8_t `qos`, bool `retain`, const uint8_t\* `payload`, size_t `length`)
```

Set the Last Will Testament. Defaults to none.
The library only stores a pointer to the topic and payload. Make sure the variable pointed to stays available throughout the lifetime of espMqttClient.

- **`topic`**: Topic of the LWT, expects a null-terminated char array (c-string)
- **`qos`**: QoS of the LWT
- **`retain`**: Retain flag of the LWT
- **`payload`**: Payload of the LWT.
- **`length`**: Payload length

```cpp
espMqttClient& setWill(const char\* `topic`, uint8_t `qos`, bool `retain`, const char\* `payload`)
```

Set the Last Will Testament. Defaults to none.
The library only stores a pointer to the topic and payload. Make sure the variable pointed to stays available throughout the lifetime of espMqttClient.

- **`topic`**: Topic of the LWT, expects a null-terminated char array (c-string)
- **`qos`**: QoS of the LWT
- **`retain`**: Retain flag of the LWT
- **`payload`**: Payload of the LWT, expects a null-terminated char array (c-string). It's lenght will be calculated using `strlen(payload)`

```cpp
espMqttClient& setServer(IPAddress `ip`, uint16_t `port`)
```

Set the server. Mind that when using `espMqttClientSecure` with a certificate, the hostname will be chacked against the certificate. OFten IP-addresses are not valid and the connection will fail.

- **`ip`**: IP of the server
- **`port`**: Port of the server

```cpp
espMqttClient& setServer(const char\* `host`, uint16_t `port`)
```

Set the server.

- **`host`**: Host of the server, expects a null-terminated char array (c-string)
- **`port`**: Port of the server

#### Options for TLS connections

All common options from WiFiClientSecure to setup an encrypted connection are made available. These include:

- `espMqttClientSecure& setInsecure()`
- `espMqttClientSecure& setCACert(const char* rootCA)` (ESP32 only)
- `espMqttClientSecure& setCertificate(const char* clientCa)` (ESP32 only)
- `espMqttClientSecure& setPrivateKey(const char* privateKey)` (ESP32 only)
- `espMqttClientSecure& setPreSharedKey(const char* pskIdent, const char* psKey)` (ESP32 only)
- `espMqttClientSecure& setFingerprint(const uint8_t fingerprint[20])` (ESP8266 only)
- `espMqttClientSecure& setTrustAnchors(const X509List *ta)` (ESP8266 only)
- `espMqttClientSecure& setClientRSACert(const X509List *cert, const PrivateKey *sk)` (ESP8266 only)
- `espMqttClientSecure& setClientECCert(const X509List *cert, const PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type)` (ESP8266 only)
- `espMqttClientSecure& setCertStore(CertStoreBase *certStore)` (ESP8266 only)

For documenation, please go to [ESP8266's documentation](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#bearssl-client-secure-and-server-secure) or [ESP32's documentation](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure)

### Events handlers

```cpp
espMqttClient& onConnect(espMqttClientTypes::OnConnectCallback `callback`)
```

Add a connect event handler.

- **`callback`**: Function to call

```cpp
espMqttClient& onDisconnect(espMqttClientTypes::OnDisconnectCallback `callback`)
```

Add a disconnect event handler.

- **`callback`**: Function to call

```cpp
espMqttClient& onSubscribe(espMqttClientTypes::OnSubscribeCallback `callback`)
```

Add a subscribe acknowledged event handler.

- **`callback`**: Function to call

```cpp
espMqttClient& onUnsubscribe(espMqttClientTypes::OnUnsubscribeCallback `callback`)
```

Add an unsubscribe acknowledged event handler.

- **`callback`**: Function to call

```cpp
espMqttClient& onMessage(espMqttClientTypes::OnMessageCallback `callback`)
```

Add a publish received event handler.

- **`callback`**: Function to call

```cpp
espMqttClient& onPublish(espMqttClientTypes::OnPublishCallback `callback`)
```

Add a publish acknowledged event handler.

- **`callback`**: Function to call

### Operational functions

```cpp
bool connected()
```

Return if the client is currently connected to the broker or not.

```cpp
void connect()
```

Connect to the server.

```cpp
void disconnect(bool `force` = false)
```

Disconnect from the server.
when disconnecting with `force` false, the client first tries to handle all the outgoing messages in the queue and disconnect cleanly afterwards. during this time, no incoming PUBLISH messages are handled.

- **`force`**: Whether to force the disconnection. Defaults to `false` (clean disconnection).

```cpp
uint16_t subscribe(const char\* `topic`, uint8_t `qos`)
```

Subscribe to the given topic at the given QoS. Return the packet ID or 0 if failed.

- **`topic`**: Topic, expects a null-terminated char array (c-string)
- **`qos`**: QoS

```cpp
uint16_t unsubscribe(const char\* `topic`)
```

Unsubscribe from the given topic. Return the packet ID or 0 if failed.

- **`topic`**: Topic, expects a null-terminated char array (c-string)

```cpp
uint16_t publish(const char\* `topic`, uint8_t `qos`, bool `retain`, const uint8\* `payload`, size_t `length`)
```

Publish a packet. Return the packet ID (or 1 if QoS 0) or 0 if failed. The topic and payload will be buffered by the library.

- **`topic`**: Topic, expects a null-terminated char array (c-string)
- **`qos`**: QoS
- **`retain`**: Retain flag
- **`payload`**: Payload
- **`length`**: Payload length

```cpp
uint16_t publish(const char\* `topic`, uint8_t `qos`, bool `retain`, const char\* `payload`)
```

Publish a packet. Return the packet ID (or 1 if QoS 0) or 0 if failed. The topic and payload will be buffered by the library.

- **`topic`**: Topic, expects a null-terminated char array (c-string)
- **`qos`**: QoS
- **`retain`**: Retain flag
- **`payload`**: Payload, expects a null-terminated char array (c-string). It's lenght will be calculated using `strlen(payload)`

```cpp
void clearQueue()
```

When disconnected, clears all queued messages.
Keep in mind that this also deletes any session data and therefore is no MQTT compliant.

```cpp
void loop()
```

This is the worker function of the MQTT client. For ESP8266 you must call this function in the Arduino loop. For ESP32 this function is only used internally and is not available in the API.

```cpp
const char* getClientId() const
```

Retuns the client ID.

# Compile time configuration

A number of constants which influence the behaviour or the client can be set at compile time. You can set these options in the `Config.h` file or pass the values as compiler flags. Because these options are compile-time constants, they are used for all instances of `espMqttClient` you create in your program.

### EMC_RX_BUFFER_SIZE 1440

The client copies incoming data into a buffer before parsing. This sets the buffer size.

### EMC_MAX_TOPIC_LENGTH 128

For **incoming** messages, a maximum topic length is set. Topics longer then this will be truncated.

### EMC_PAYLOAD_BUFFER_SIZE 32

Set the incoming payload buffer size for SUBACK messages. Although the library needs this value, it doesn't currently influence the behaviour.

### EMC_MIN_FREE_MEMORY 4096

The client keeps all outgoing packets in a queue which stores it's data in heap memory. With this option, you can set the minimum available (contiguous) heap memory that needs to be available for adding a message to the queue.

### EMC_ESP8266_MULTITHREADING 0

ESP8266 doesn't use multithreading and is only single-core. There is therefore no need for semaphores/mutexes on this platform. However, you can still enable this.

### EMC_CLIENTID_LENGTH 18 + 1

The (maximum) length of the client ID.

### EMC_TASK_STACK_SIZE 10000

Only used on ESP32. Sets the stack size (in words) of the MQTT client worker task.

### Logging

You have to enable logging at compile time. This is done differently on ESP32 and ESP8266.

ESP8266:

- Enable logging for Arduino [see docs](https://arduino-esp8266.readthedocs.io/en/latest/Troubleshooting/debugging.html)
- Pass the `DEBUG_ESP_MQTT_CLIENT` flag to the compiler

ESP32

- Enable logging for Arduino [see docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/guides/tools_menu.html?#core-debug-level)

# Code samples

A number of examples are in the [examples](/examples) directory. These include basic operation on ESP8266 and ESP32. Please examine these to understand the basic operation of the MQTT client.

Below are examples on specific points for working with this library

### Printing payloads

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

### Assembling chunked messages

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
