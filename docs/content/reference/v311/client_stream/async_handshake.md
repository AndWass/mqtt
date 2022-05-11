---
title: "mqtt::v311::client_stream::async_handshake"
geekdocHidden: true
---

```cpp
template<class WriteHandler = boost::asio::default_completion_token_t<executor_type>>
DEDUCED
async_handshake(const connect_opts &opts,
                WriteHandler &&handler = boost::asio::default_completion_token_t<executor_type>{})
```

This function is used to asynchronously perform a handshake with a server.
This is done by first sending a `CONNECT` message and then waiting for a
message from the server. If the message received from the server isn't a `CONNACK`
message (or the `CONNACK` doesn't signal success) `ec` will be set to an error.
This call always returns immediately. The asynchronous operation
will continue until one of the following conditions is true:

* A message is received.
* An error occurs.

The algorithm, known as a composed asynchronous operation,
is implemented in terms of calls to the next layers async_write_some
function. The program must ensure that no other calls to `async_write`
are performed until this operation completes.

The connect options will be copied to an internal buffer used when sending
the connect message. The caller can therefore discard that data immediately after
the call has returned.

## Parameters

### `opts`

The options to use to build the `CONNECT` message.

### `handler`

The completion handler to invoke when the operation completes.
The implementation takes ownership of the handler by performing a decay-copy.
The equivalent function signature of the handler must be:

```cpp
void handler(
error_code const& ec,           // Result of operation
bool session_present            // Whether server response
                                // indicated session present or not.
);
```

Regardless of whether the asynchronous operation completes immediately or not,
the handler will not be invoked from within this function. Invocation of the
handler will be performed in a manner equivalent to using net::post. 
