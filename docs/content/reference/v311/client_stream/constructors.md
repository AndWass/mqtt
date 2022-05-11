---
title: "purple::v311::client_stream"
geekdocHidden: true
---

## Overload 1 of 2

```cpp
client_stream(client_stream&&);
```

If `purple::stream<NextLayer>` is move constructible, this function will move-construct a new stream from the existing stream.
After the move, the only valid operation on the moved-from object is destruction.

---

## Overload 2 of 2

```cpp
template<class... Args>
explicit client_stream(Args &&...args);
```

Create an MQTT stream and construct the `purple::stream<NextLayer>` type using`args...`.
See [`purple::stream::stream`]({{< ref "/reference/stream/constructors" >}}) for available
constructors.

### Exceptions

| Type                | Thrown on                                                       |
|---------------------|-----------------------------------------------------------------|
| Any                 | exceptions thrown by the `purple::stream<NextLayer>` constructor. |

### Parameters

#### `args`

The arguments to be passed to initialize the stream object.
The arguments are forwarded to the stream constructor.
