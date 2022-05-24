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

### Payloads and strings

> Beware that MQTT payloads are binary. MQTT payloads are **not** c-strings unless explicitely constructed like that. You therefore can **not** print the payload to your Serial monitor without supporting code.

## Disconnecting

You can disconnect from the broker by calling `disconnect()`. If you do not force-disconnect, the client will first send the remaining messages that are in the queue and disconnect afterwards. During this period however, no new incoming PUBLISH messages will be processed.
