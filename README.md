**WARNING**

This library is under development. You know what this means...

# espMqttClient

MQTT client library for the Espressif devices ESP8266 and ESP32 on the Arduino framework.
Aims to be a non-blocking fully compliant MQTT 3.1.1 client.

# Features

- MQTT 3.1.1 compliant library
- Sending and receiving at all QoS levels
- TCP and TCP/TLS using standard WiFiClient and WiFiClientSecure connections
- Virtually unlimited incoming ~~and outgoing~~ payload sizes
- Readable and understandable code
- No dependencies outside the Arduino framework

** strikethrough features are in development

# Runtime behaviour

A normal cycle of an MQTT client goes like this:
1. setup the client
2. connect to the broker
3. subscribe/publish/receive
4. disconnect/reconnect when disconnected

## Setup

Setting up the client means to tell which host and port to connect to, possible credentials to use and so on. espMqttClient has a set of methods to configure the client. Setup is generally done in the `setup()` function of the Arduino framework. 
One important thing to remember is that there are a number of settings that are not stored inside the library: `username`, `password`, `willTopic`, `willPayload`, `clientId` and `host`. Make sure these variables stay available during the lifetime of the `espMqttClient`.

For TLS secured connections, the relevant methods from `WiFiClientSecure` have been made available.

## Connecting

After setting up the client, you are ready to connect. A simple call to `connect()` does the trick. If you set a `OnConnectCallback`, you will be notified when the connection has been made. On failure, `OnDisconnectCallback` will be called.

## Subscribing, publishing and receiving

Once connected, you can subscribe, publish and receive. The methods to do this return the packetId of the generated packet or `1` for packets without ID. In case of an error, the method returns `0`. When the client is not connected, you cannot subscribe, unsubscribe or publish.

Receiving packets is done via the `onMessage`-callback. This callback gives you the topic, properties (qos, dup, retain, packetId) and payload. For the payload, you get a pointer to the data, the index, length and total length. On long payloads it is normal that you get multiple callbacks for the same packet. This way, you can receive payloads longer than what could fit in the microcontroller's RAM-memory.

> Beware that MQTT payloads are binary. MQTT payloads are **not** c-strings unless explicitely constructed like that. You therefore can **not** print the payload to your Serial monitor without supporting code.

## Disconnecting

You can disconnect from the broker by calling `disconnect()`. If you do not force-disconnect, the client will first send the remaining messages that are in the queue and disconnect afterwards. During this period however, no new incoming PUBLISH messages will be processed.

## Limitations

### Warning!

> Because of a timeout bug in `WiFiClientSecure` in the current Arduino framework release you cannot reconnect once disconnected. This has been solved in Git but hasn't made it into a next release yet.

### MQTT 3.1.1 Compliancy

Outgoing messages and session data are not stored in non-volatile memory. Any events like loss of power or sudden resets result in loss of data. Despite this limitation, one could still consider this library as fully complaint based on the non normative remark in point 4.1.1 of the specification.

### Non-blocking

This library aims to be fully non-blocking. It is however limited by the underlying `WiFiClient` library this has a blocking `connect` method. On ESP32 it is not an issue since the call is offloaded to a separate task. On ESP8266 however, connecting will block until succesful or until the connection timeouts.

# Bugs and feature requests

Please use Github's facilities to get in touch.

# License

This library is released under the MIT Licence. A copy is included in the repo.
Parts of this library are based upon [Async MQTT client for ESP8266 and ESP32](https://github.com/marvinroger/async-mqtt-client).
