---
title: "mqtt::stream::stream"
geekdocHidden: true
---

## Overload 1 of 2

```cpp
stream(stream&&);
```

If `NextLayer` is move constructible, this function will move-construct a new stream from the existing stream.
After the move, the only valid operation on the moved-from object is destruction.

---

## Overload 2 of 2

```cpp
template<class...Args>
explicit stream(Args&&...args);
```

Create an MQTT stream and construct the `NextLayer` type using `args...`.

### Exceptions

| Type | Thrown on                                       |
|------|-------------------------------------------------|
| Any  | exceptions thrown by the NextLayer constructor. |

### Parameters

#### `args`

The arguments to be passed to initialize the next layer object.
The arguments are forwarded to the next layers constructor.

---
