---
title: "AsyncDefaultConnectableStream"
---

A type modelling `AsyncDefaultConnectableStream` meets all the following requirements:
  * [`AsyncStream`]
  * The requirements for [`async_connect`](#async_connect) below.

## `async_connect`

`a.async_connect(h)`

### Return type

The return type is determined according to the requirements for an
[asynchronous operation] with a completion signature of `void(error_code ec)`.

### Semantics

Initiates an asynchronous operation to connect stream to its *default* endpoint. How this
endpoint is chosen is decided by the implementor.

If the connection succeeds then `!ec` is `true` otherwise `!ec` is `false`.

[`AsyncStream`]: https://www.boost.org/doc/libs/1_79_0/libs/beast/doc/html/beast/concepts/streams.html#beast.concepts.streams.AsyncStream
[asynchronous operation]: https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/asynchronous_operations.html