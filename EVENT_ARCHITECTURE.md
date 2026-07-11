# Event-Based Architecture Overview

## What Changed

The system has been transformed from a **hook-based system** to a full **observer pattern event-driven architecture**. This provides better separation of concerns, scalability, and flexibility.

## Core Components

### 1. Event System
```c
enum EventType {
    EVENT_STREAM_DATA_AVAILABLE,  // Data received from stream
    EVENT_STREAM_EOF,              // End of file reached
    EVENT_STREAM_ERROR,            // Error occurred
    EVENT_QUIT                     // Quit signal
};

struct Event {
    enum EventType type;
    void* source;                  // Which stream generated it
    char data[STREAM_BUFFER_SIZE];
    size_t dataLength;
};
```

### 2. Event Dispatcher (Observer Pattern)
The `EventDispatcher` is the central hub:
- **Manages subscribers** for each event type
- **Queues events** from producers (streams)
- **Routes events** to all subscribers in dispatcher thread
- **Thread-safe** with mutexes and semaphores

Key functions:
- `EventDispatcherSubscribe()` - Register event handler
- `EventDispatcherPublish()` - Queue new event
- `EventDispatcherUnsubscribe()` - Deregister handler

### 3. Event Handlers
Pure functions that react to events:

```c
void logToFileEventHandler(struct Event* event, void* context);
void displayKeyPressEventHandler(struct Event* event, void* context);
void quitEventHandler(struct Event* event, void* context);
```

Handlers:
- Only process relevant event types
- Operate independently
- Can be added/removed dynamically
- No knowledge of other handlers

### 4. Streams (Simplified)
Streams now focus on I/O only:
- Read from file descriptors
- Write to buffers
- Publish `EVENT_STREAM_DATA_AVAILABLE` events
- No hook management

## Architecture Flow

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  Input Stream (Reader Thread)                              │
│  ├─ Uses select() to wait for data                         │
│  └─ Publishes EVENT_STREAM_DATA_AVAILABLE                  │
│           │                                                 │
│           ▼                                                 │
│  ┌──────────────────────────────┐                          │
│  │  Event Queue & Dispatcher    │                          │
│  │  ├─ Thread-safe queue        │                          │
│  │  ├─ Semaphore synchronization│                          │
│  │  └─ Subscriber lookup tables │                          │
│  └──────────────────────────────┘                          │
│           │                                                 │
│           ├─► logToFileEventHandler     ──► Log File       │
│           ├─► displayKeyPressEventHandler ──► Output       │
│           └─► quitEventHandler          ──► Done Semaphore│
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Advantages Over Hook-Based System

| Aspect | Hooks | Events |
|--------|-------|--------|
| Coupling | Tightly coupled | Decoupled |
| Dynamic handlers | Hard to manage | Easy subscribe/unsubscribe |
| Event types | Implicit (callbacks) | Explicit (EventType enum) |
| Scalability | O(n) hooks per stream | O(1) per subscriber type |
| Debugging | Hard to trace | Clear event flow |
| Testing | Difficult to mock | Easy to test handlers independently |

## How to Extend

### Add a New Event Type
1. Add enum value to `EventType`
2. Publish it from appropriate stream/code:
   ```c
   struct Event event = {
       .type = EVENT_YOUR_TYPE,
       .source = stream,
       .dataLength = size
   };
   EventDispatcherPublish(dispatcher, &event);
   ```

### Add a New Event Handler
1. Write handler function:
   ```c
   void myEventHandler(struct Event* event, void* context) {
       if (event->type != EVENT_YOUR_TYPE) return;
       // Process event...
   }
   ```
2. Subscribe in main:
   ```c
   EventDispatcherSubscribe(dispatcher, EVENT_YOUR_TYPE, 
                           myEventHandler, context_data);
   ```

### Dynamic Handler Management
```c
// Subscribe at runtime
EventDispatcherSubscribe(dispatcher, EVENT_STREAM_DATA_AVAILABLE,
                        newHandler, new_context);

// Unsubscribe
EventDispatcherUnsubscribe(dispatcher, EVENT_STREAM_DATA_AVAILABLE,
                          newHandler);
```

## Thread Safety

- **Event Dispatcher**: Mutex-protected queue, separate mutexes per subscriber list
- **Streams**: Mutex-protected buffers
- **Semaphores**: Used for event queue wakeup and application synchronization

All access is thread-safe without busy-waiting.

## Performance

- **No polling**: Uses `select()` for I/O and semaphores for event dispatch
- **Minimal lock contention**: Separate mutexes for different components
- **Low latency**: Events processed immediately by dispatcher thread
- **Scalable**: Adding handlers doesn't impact existing infrastructure

## Testing the System

```bash
# Run with shell script (piped input)
bash test_interactive.sh

# Run directly from terminal
./main
```

Both work correctly with the event-based architecture.
