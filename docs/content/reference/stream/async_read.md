---
title: "mqtt::stream::async_read"
geekdocHidden: true
---

## Overload 1 of 2

```cpp
template<class DynamicBuffer, class ReadHandler>
DEDUCED async_read(DynamicBuffer &buffer, ReadHandler &&handler);
```

This function is used to asynchronously read a complete message.
This call always returns immediately. The asynchronous operation will
continue until one of the following conditions is true:

* A complete message is received.
* An error occurs.

The algorithm, known as a composed asynchronous operation,
is implemented in terms of calls to the next layers async_read_some.
The program must ensure that no other calls to `async_read` are
performed until this operation completes.
Received message data is appended to the buffer, starting with the first byte
of the variable header.

### Parameters

#### `buffer`

A dynamic buffer to append the message to. The buffer will contain
the variable header and the payload. The buffer will not contain the
fixed header portion.

#### `handler`

The completion handler to invoke when the operation completes.
The implementation takes ownership of the handler by performing a
decay-copy. The equivalent function signature of the handler must be:

```cpp
void handler(
error_code const& ec,       // Result of operation
mqtt::fixed_header header   // The fixed header data of the message
);
```

Regardless of whether the asynchronous operation completes immediately or not,
the handler will not be invoked from within this function.
Invocation of the handler will be performed in a manner equivalent to
using `net::post`. 

## Overload 2 of 2

```cpp
template<class ReadHandler>
DEDUCED async_read(const boost::asio::mutable_buffer &buffer, ReadHandler &&handler);
```

Works the same as [Overload 1 of 2](#overload-1-of-2) but uses a fixed sized buffer
instead of a growable dynamic buffer.

If the message  doesn't fit in the provided buffer no bytes will
be read and `ec` will be set to `mqtt::error::message_too_large`.

### Parameters

#### `buffer`

A mutable buffer that the message will be read to. The buffer will contain the
variable header and the payload. The buffer will not contain the fixed header.

#### `handler`

The completion handler to invoke when the operation completes.
The implementation takes ownership of the handler by performing a
decay-copy. The equivalent function signature of the handler must be:

```cpp
void handler(
error_code const& ec,       // Result of operation
mqtt::fixed_header header   // The fixed header data of the message
);
```

Regardless of whether the asynchronous operation completes immediately or not,
the handler will not be invoked from within this function.
Invocation of the handler will be performed in a manner equivalent to
using `net::post`. 
