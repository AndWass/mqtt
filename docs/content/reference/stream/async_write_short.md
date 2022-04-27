---
title: "mqtt::stream::async_write_short"
geekdocHidden: true
---

```cpp
template<class WriteHandler>
DEDUCED async_write_short(uint8_t first_byte,
                          std::array<uint8_t, 3> payload,
                          uint8_t payload_length,
                          WriteHandler &&handler)
```

This is a helper function that utilizes the internal fixed header write buffer
to write short messages, with a payload no greater than 3 bytes.

This is useful for sending things
like [PINGREQ](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081)
or other short control frames that can be sent without having to go through a secondary
buffer.

## Exceptions

| Type                | Thrown on                                       |
|---------------------|-------------------------------------------------|
| `std::length_error` | If `payload_length > payload.size()`.           |

## Parameters

### `first_byte`

The first byte of the MQTT message. The first byte contains the control packet type
and flags.

### `payload`

The payload to send.

### `payload_length`

Number of payload bytes to actually send.

### `handler`

The completion handler to invoke when the operation completes.
The implementation takes ownership of the handler by performing a decay-copy.
The equivalent function signature of the handler must be:

```cpp
void handler(
error_code const& ec,           // Result of operation
std::size_t bytes_transferred   // Total number of bytes sent
                                // including complete fixed header.
);
```

Regardless of whether the asynchronous operation completes immediately or not,
the handler will not be invoked from within this function. Invocation of the
handler will be performed in a manner equivalent to using net::post.