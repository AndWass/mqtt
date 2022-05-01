---
title: "mqtt::stream"
---

```cpp
template<class NextLayer>
class stream;
```

`mqtt::stream` is a low level stream to read and write MQTT messages on
the binary format specified by
the [MQTT standard](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718018).

No validation of the validity of messages is done, it simply makes sure
to send data using the correct wire format.

## Types

| Name                                 | Description                                     |
|--------------------------------------|-------------------------------------------------|
| [`executor_type`](executor_type)     | The type of executor associated with the stream |
| [`next_layer_type`](next_layer_type) | The type of the next layer stream               |

---

## Member functions

| Name                                       | Description                                       |
|--------------------------------------------|---------------------------------------------------|
| [`async_read`](async_read)                 | Read a complete message asynchronously.           |
| [`async_write`](async_write)               | Write a complete message asynchronously.          |
| [`get_executor`](get_executor)             | Get the executor associated with the object.      |
| [`next_layer`](next_layer)                 | Get a reference to the next layer.                |
| [`reset`](reset)                           | Reset the stream to prepare for a new connection. |
| [`stream`](constructors)                   | Constructor.                                      |

