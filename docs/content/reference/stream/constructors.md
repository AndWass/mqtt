---
title: "mqtt::stream::stream"
geekdocHidden: true
---

## Overload 1 of 3

```cpp
stream(stream&&);
```

If `NextLayer` is move constructible, this function will move-construct a new stream from the existing stream.
After the move, the only valid operation on the moved-from object is destruction.

---

## Overload 2 of 3

```cpp
template<class...Args>
explicit stream(size_t read_buffer_size, size_t write_buffer_size, Args&&...args);
```

Create an MQTT stream and construct the `NextLayer` type using `args...`. The size
of the internal read buffer is controlled by `read_buffer_size`, and the size of
the internal write buffer is controlled by `write_buffer_size`.

### Exceptions

| Type                | Thrown on                                           |
|---------------------|-----------------------------------------------------|
| `std::length_error` | If `read_buffer_size < 5 or write_buffer_size < 5`. |
| Any                 | exceptions thrown by the NextLayer constructor.     |

### Parameters

#### `read_buffer_size`

The size of the internal read buffer. Must be at least 5.

#### `write_buffer_size`

The size of the internal write buffer. Must be at least 5.

#### `args`

The arguments to be passed to initialize the next layer object.
The arguments are forwarded to the next layers constructor.

---

## Overload 3 of 3

```cpp
template<class...Args>
explicit stream(Args&&...args);
```

Create an MQTT stream and construct the `NextLayer` type using `args...`. Uses default
internal buffers of 1024 bytes.

### Exceptions

| Type | Thrown on                                       |
|------|-------------------------------------------------|
| Any  | exceptions thrown by the NextLayer constructor. |

### Parameters

#### `args`

The arguments to be passed to initialize the next layer object.
The arguments are forwarded to the next layers constructor.

---
