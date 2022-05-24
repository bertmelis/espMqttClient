**WARNING**

This library is under development. You know what this means...

# espMqttClient

MQTT client library for the Espressif devices ESP8266 and ESP32 on the Arduino framework.
Aims to be a non-blocking fully compliant MQTT 3.1.1 client.

![examples](https://github.com/bertmelis/espMqttClient/actions/workflows/build_examples.yml/badge.svg)
![linting](https://github.com/bertmelis/espMqttClient/actions/workflows/lint.yml/badge.svg)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/bertmelis/library/espMqttClient.svg)](https://registry.platformio.org/libraries/bertmelis/espMqttClient)

# Features

- MQTT 3.1.1 compliant library
- Sending and receiving at all QoS levels
- TCP and TCP/TLS using standard WiFiClient and WiFiClientSecure connections
- Virtually unlimited incoming ~~and outgoing~~ payload sizes
- Readable and understandable code
- No dependencies outside the Arduino framework

** strikethrough features are in development

# Documentation

See [documentation](docs)

## Limitations

### Warning!

> Because of a timeout bug in `WiFiClientSecure` in the current Arduino framework release you cannot reconnect once disconnected. This has been solved in Git but hasn't made it into a next release yet.

### MQTT 3.1.1 Compliancy

Outgoing messages and session data are not stored in non-volatile memory. Any events like loss of power or sudden resets result in loss of data. Despite this limitation, one could still consider this library as fully complaint based on the non normative remark in point 4.1.1 of the specification.

### Non-blocking

This library aims to be fully non-blocking. It is however limited by the underlying `WiFiClient` library this has a blocking `connect` method. On ESP32 it is not an issue since the call is offloaded to a separate task. On ESP8266 however, connecting will block until succesful or until the connection timeouts.

# Bugs and feature requests

Please use Github's facilities to get in touch.

# About this library

This client wouldn't exist without [Async-mqtt-client](https://github.com/marvinroger/async-mqtt-client). It has been my go-to MQTT client for many years. It was fast and -despite other opinions- reliable and had features that were non-existing in alternative libraries. However, the underlying async TCP libraries are lacking updates so I decided to write my own library, from scratch.

The result is an almost non-blocking library with no external dependencies. Except for a few type differences, the library is a drop-in replacement for the async-mqtt-client.

# License

This library is released under the MIT Licence. A copy is included in the repo.
Parts of this library are based on [Async MQTT client for ESP8266 and ESP32](https://github.com/marvinroger/async-mqtt-client).
