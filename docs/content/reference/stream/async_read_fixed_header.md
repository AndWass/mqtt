---
title: "mqtt::stream::async_read_fixed_header"
geekdocHidden: true
---

```cpp
template<class ReadHandler>
DEDUCED async_read_fixed_header(ReadHandler &&handler);
```

This function is used to asynchronously read the fixed header of a message.
This call always returns immediately. The asynchronous operation will
continue until one of the following conditions is true:

* A complete fixed header is received.
* An error occurs.

The algorithm, known as a composed asynchronous operation,
is implemented in terms of calls to the next layer's async_read_some.
The program must ensure that no other calls to `async_read` or
`async_read_fixed_header` are performed until this operation
completes.

## Parameters

### `handler`

The completion handler to invoke when the operation completes.
The implementation takes ownership of the handler by performing a
decay-copy. The equivalent function signature of the handler must be:

```cpp
void handler(
error_code const& ec,       // Result of operation
mqtt::fixed_header header   // The fixed header read
);
```

Regardless of whether the asynchronous operation completes
immediately or not, the handler will not be invoked from
within this function. Invocation of the handler will be
performed in a manner equivalent to using net::post.
