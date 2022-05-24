# Runtime behaviour

A number of constants which influence the behaviour or the client can be set at compile time. You can set these options in the `Config.h` file or pass the values as compiler flags.

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

## Logging

You have to enable logging at compile time. This is done differently on ESP32 and ESP8266.

ESP8266:
* Enable logging for Arduino [see docs](https://arduino-esp8266.readthedocs.io/en/latest/Troubleshooting/debugging.html)
* Pass the `DEBUG_ESP_MQTT_CLIENT` flag to the compiler

ESP32
* Enable logging for Arduino [see docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/guides/tools_menu.html?#core-debug-level)
