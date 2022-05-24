# API reference
espMqttClient
## espMqttClient() - espMqttClientSecure()

Instantiate a new espMqttClient or espMqttSecure object.
On ESP32, two optional parameters are available: `espMqttClient(uint8_t priority = 1, uint8_t core = 1)`. This will change the priority of the MQTT client task and the core on which it runs.

## Configuration

### espMqttClient& setKeepAlive(uint16_t `keepAlive`)

Set the keep alive. Defaults to 15 seconds.

* **`keepAlive`**: Keep alive in seconds

### espMqttClient& setClientId(const char\* `clientId`)

Set the client ID. Defaults to `esp8266<chip ID on 6 hex caracters>`.
The library only stores a pointer to the client ID. Make sure the variable pointed to stays available throughout the lifetime of espMqttClient.

* **`clientId`**: Client ID, expects a null-terminated char array (c-string)

### espMqttClient& setCleanSession(bool `cleanSession`)

Set the CleanSession flag. Defaults to `true`.

* **`cleanSession`**: clean session wanted or not

### espMqttClient& setCredentials(const char\* `username`, const char\* `password`)

Set the username/password. Defaults to non-auth.
The library only stores a pointer to the username and password. Make sure the variable to pointed stays available throughout the lifetime of espMqttClient.

* **`username`**: Username, expects a null-terminated char array (c-string)
* **`password`**: Password, expects a null-terminated char array (c-string)

### espMqttClient& setWill(const char\* `topic`, uint8_t `qos`, bool `retain`, const uint8_t\* `payload`, size_t `length`)

Set the Last Will Testament. Defaults to none.
The library only stores a pointer to the topic and payload. Make sure the variable pointed to stays available throughout the lifetime of espMqttClient.

* **`topic`**: Topic of the LWT, expects a null-terminated char array (c-string)
* **`qos`**: QoS of the LWT
* **`retain`**: Retain flag of the LWT
* **`payload`**: Payload of the LWT.
* **`length`**: Payload length

### espMqttClient& setWill(const char\* `topic`, uint8_t `qos`, bool `retain`, const char\* `payload`)

Set the Last Will Testament. Defaults to none.
The library only stores a pointer to the topic and payload. Make sure the variable pointed to stays available throughout the lifetime of espMqttClient.

* **`topic`**: Topic of the LWT, expects a null-terminated char array (c-string)
* **`qos`**: QoS of the LWT
* **`retain`**: Retain flag of the LWT
* **`payload`**: Payload of the LWT, expects a null-terminated char array (c-string). It's lenght will be calculated using `strlen(payload)`

### espMqttClient& setServer(IPAddress `ip`, uint16_t `port`)

Set the server. Mind that when using `espMqttClientSecure` with a certificate, the hostname will be chacked against the certificate. OFten IP-addresses are not valid and the connection will fail.

* **`ip`**: IP of the server
* **`port`**: Port of the server

### espMqttClient& setServer(const char\* `host`, uint16_t `port`)

Set the server.

* **`host`**: Host of the server, expects a null-terminated char array (c-string)
* **`port`**: Port of the server

### Options for TLS connections

All common options from WiFiClientSecure to setup an encrypted connection are made available. These include:

* espMqttClientSecure& setInsecure()
* espMqttClientSecure& setCACert(const char* rootCA) (ESP32 only)
* espMqttClientSecure& setCertificate(const char* clientCa) (ESP32 only)
* espMqttClientSecure& setPrivateKey(const char* privateKey) (ESP32 only)
* espMqttClientSecure& setPreSharedKey(const char* pskIdent, const char* psKey) (ESP32 only)
* espMqttClientSecure& setFingerprint(const uint8_t fingerprint[20]) (ESP8266 only)
* espMqttClientSecure& setTrustAnchors(const X509List *ta) (ESP8266 only)
* espMqttClientSecure& setClientRSACert(const X509List *cert, const PrivateKey *sk) (ESP8266 only)
* espMqttClientSecure& setClientECCert(const X509List *cert, const PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type) (ESP8266 only)
* espMqttClientSecure& setCertStore(CertStoreBase *certStore) (ESP8266 only)

For documenation, please go to [ESP8266's documentation](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#bearssl-client-secure-and-server-secure) or [ESP32's documentation](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure)

### Events handlers

#### espMqttClient& onConnect(espMqttClientTypes::OnConnectCallback `callback`)

Add a connect event handler.

* **`callback`**: Function to call

#### espMqttClient& onDisconnect(espMqttClientTypes::OnDisconnectCallback `callback`)

Add a disconnect event handler.

* **`callback`**: Function to call

#### espMqttClient& onSubscribe(espMqttClientTypes::OnSubscribeCallback `callback`)

Add a subscribe acknowledged event handler.

* **`callback`**: Function to call

#### espMqttClient& onUnsubscribe(espMqttClientTypes::OnUnsubscribeCallback `callback`)

Add an unsubscribe acknowledged event handler.

* **`callback`**: Function to call

#### espMqttClient& onMessage(espMqttClientTypes::OnMessageCallback `callback`)

Add a publish received event handler.

* **`callback`**: Function to call

#### espMqttClient& onPublish(espMqttClientTypes::OnPublishCallback `callback`)

Add a publish acknowledged event handler.

* **`callback`**: Function to call

### Operation functions

#### bool connected()

Return if the client is currently connected to the broker or not.

#### void connect()

Connect to the server.

#### void disconnect(bool `force` = false)

Disconnect from the server.
when disconnecting with `force` false, the client first tries to handle all the outgoing messages in the queue and disconnect cleanly afterwards. during this time, no incoming PUBLISH messages are handled.

* **`force`**: Whether to force the disconnection. Defaults to `false` (clean disconnection).

#### uint16_t subscribe(const char\* `topic`, uint8_t `qos`)

Subscribe to the given topic at the given QoS. Return the packet ID or 0 if failed.

* **`topic`**: Topic, expects a null-terminated char array (c-string)
* **`qos`**: QoS

#### uint16_t unsubscribe(const char\* `topic`)

Unsubscribe from the given topic. Return the packet ID or 0 if failed.

* **`topic`**: Topic, expects a null-terminated char array (c-string)

#### uint16_t publish(const char\* `topic`, uint8_t `qos`, bool `retain`, const uint8\* `payload`, size_t `length`)

Publish a packet. Return the packet ID (or 1 if QoS 0) or 0 if failed. The topic and payload will be buffered by the library.

* **`topic`**: Topic, expects a null-terminated char array (c-string)
* **`qos`**: QoS
* **`retain`**: Retain flag
* **`payload`**: Payload
* **`length`**: Payload length

#### uint16_t publish(const char\* `topic`, uint8_t `qos`, bool `retain`, const char\* `payload`)

Publish a packet. Return the packet ID (or 1 if QoS 0) or 0 if failed. The topic and payload will be buffered by the library.

* **`topic`**: Topic, expects a null-terminated char array (c-string)
* **`qos`**: QoS
* **`retain`**: Retain flag
* **`payload`**: Payload, expects a null-terminated char array (c-string). It's lenght will be calculated using `strlen(payload)`

#### void clearQueue()

When disconnected, clears all queued messages.
Keep in mind that this also deletes any session data and therefore is no MQTT compliant.

#### void loop()

This is the worker function of the MQTT client. For ESP8266 you must call this function in the Arduino loop. For ESP32 this function is only used internally and is not available in the API.

#### const char* getClientId() const

Retuns the client ID.
