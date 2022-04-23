---
title: "mqtt::stream::async_write"
geekdocHidden: true
---

```cpp
template<class ConstBufferSequence, class WriteHandler>
DEDUCED async_write(uint8_t first_byte,
                    const ConstBufferSequence &buffer,
                    WriteHandler &&handler);
```

This function is used to asynchronously write a complete message.
This call always returns immediately. The asynchronous operation
will continue until one of the following conditions is true:

* The complete message is written.
* An error occurs.

The algorithm, known as a composed asynchronous operation,
is implemented in terms of calls to the next layers async_write_some
function. The program must ensure that no other calls to `async_write`
are performed until this operation completes.

The variable length field of the fixed header is calculated internally,
therefore the first byte of `buffer` must be the first byte of the variable header
(if any) or payload (if any).

## Exceptions

| Type               | Thrown on                                      |
|--------------------|------------------------------------------------|
| std::runtime_error | If `beast::buffer_bytes(buffer) > 268'435'455` |

## Parameters

### `first_byte`

The first byte of the MQTT message. The first byte contains the control packet type
and flags.

### `buffer`

A buffer sequence containing the variable header and payload.
The implementation will make copies of this object as needed,
but ownership of the underlying memory is not transferred.
The caller is responsible for ensuring that the memory locations pointed to
by buffers remains valid until the completion handler is called.

### `handler`

The completion handler to invoke when the operation completes.
The implementation takes ownership of the handler by performing a decay-copy.
The equivalent function signature of the handler must be:

```cpp
void handler(
error_code const& ec,           // Result of operation
std::size_t bytes_transferred   // Total number of bytes sent
                                // including complete fixed header.
                                // If no errors occurred this will be greater
                                // than the buffer_size.
);
```

Regardless of whether the asynchronous operation completes immediately or not,
the handler will not be invoked from within this function. Invocation of the
handler will be performed in a manner equivalent to using net::post. 

