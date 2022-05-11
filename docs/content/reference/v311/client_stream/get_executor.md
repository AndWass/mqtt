---
title: "mqtt::v311::client_stream::get_executor"
geekdocHidden: true
---

```cpp
executor_type get_executor();
```

This function may be used to obtain the executor object that the stream uses
to dispatch handlers for asynchronous operations.

### Return value

A copy of the executor that the stream will use to dispatch handlers.