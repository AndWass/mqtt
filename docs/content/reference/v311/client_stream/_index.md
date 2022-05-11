---
title: "purple::v311::client_stream"
---

```cpp
template<class NextLayer>
class client_stream;
```

`purple::v311::client_stream` is a stream suitable for a client connection.
Internally it uses an [`purple::stream`]({{< ref "/reference/stream" >}}) but
for all intents and purposes this is hidden and should be treated mostly
as an implementation detail.

## Types

| Name                                 | Description                                     |
|--------------------------------------|-------------------------------------------------|
| [`executor_type`](executor_type)     | The type of executor associated with the stream |
| [`next_layer_type`](next_layer_type) | The type of the next layer stream               |

---

## Member functions

| Name                                 | Description                                       |
|--------------------------------------|---------------------------------------------------|
| [`async_handshake`](async_handshake) | Handshake with a server asynchronously.           |
| [`get_executor`](get_executor)       | Get the executor associated with the object.      |
| [`next_layer`](next_layer)           | Get a reference to the next layer.                |
| [`client_stream`](constructors)      | Constructor.                                      |
