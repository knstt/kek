# Runtime Architecture

Related documents:

- [Runtime Requirements](requirements.md)
- [Runtime Specification](specification.md)
- [Generator Architecture](../generator/architecture.md)

## Overview

The runtime is a small single-threaded C framework built around a bounded event queue, rollback-safe generated state storage, an instance-aware generated state store, a fixed-size runtime state registry, and `select()`-based file descriptor readiness.

```mermaid
flowchart TB
    Runtime[KekRuntime]
    Dispatcher[KekEventDispatcher]
    StateRegistry[Runtime State Registry]
    StateStorage[Generated State Storage]
    StateInterface[KekRuntimeState callbacks]
    StreamState[KekStream]
    FD[File Descriptor]
    Subscriber[Event Subscribers]

    Runtime --> Dispatcher
    Runtime --> StateRegistry
    Runtime --> StateStorage
    StateRegistry --> StateInterface
    StateInterface --> StreamState
    StreamState --> FD
    StreamState --> Dispatcher
    Dispatcher --> Subscriber
```

## Module Responsibilities

| Module | Responsibility |
| --- | --- |
| `runtime/event.h` | Event types, event payload, subscriber lists, dispatcher declarations. |
| `runtime/event.c` | Event queue operations and subscriber dispatch. |
| `runtime/hook.h` | Hook context, hook descriptor, and registry declarations. |
| `runtime/hook.c` | Generated hook registry and event filtering. |
| `runtime/state.h` | Runtime state kind and callback interface. |
| `runtime/runtime.h` | Runtime object and public runtime API. |
| `runtime/runtime.c` | Runtime initialization, state registry, event loop, drain, raw mode. |
| `runtime/stream.h` | Stream state API and stream data structure. |
| `runtime/stream.c` | Stream state implementation over file descriptors. |
| `runtime/state_storage.h` | Rollback-safe generated state storage API. |
| `runtime/state_storage.c` | Double-buffered generated state storage and instance-aware state store implementation. |

## Dependency Direction

```mermaid
flowchart LR
    Event[event.*]
    State[state.h]
    Runtime[runtime.*]
    Stream[stream.*]
    Consumer[Runtime Consumer]

    Runtime --> Event
    Runtime --> State
    Stream --> Runtime
    Consumer --> Runtime
    Consumer --> Stream
```

The runtime does not depend on generated schema files. Generated code or application code may depend on the runtime.

## Event Dispatch Architecture

Events are published into a global ring buffer. Dispatch removes events from the ring buffer in FIFO order and invokes active subscribers for the event type. State-change events can identify the generated state type, concrete slot instance, and version.

Generated hooks attach to the same dispatcher through `KekHookRegistry`. The registry keeps one event subscription per event type and invokes only descriptors whose trigger matches the event.

```mermaid
flowchart LR
    Publisher[Publisher]
    Queue[Bounded Ring Queue]
    Dispatcher[Dispatch Pending]
    List[Subscriber List by Type]
    Handler[Handler Callback]

    Publisher --> Queue
    Queue --> Dispatcher
    Dispatcher --> List
    List --> Handler
```

## Runtime State Architecture

The runtime owns generic state slots. Each state implementation supplies callbacks that let the runtime remain generic.

## Generated State Store Architecture

Generated state objects can be stored independently in `KekStateStore`. Each slot points to a generated descriptor and owns two buffers for validate-before-swap updates.

```mermaid
flowchart LR
    Descriptor[KekStateDescriptor]
    SlotA[State Slot 0]
    SlotB[State Slot 1]
    Active[Active Buffer]
    Draft[Draft Buffer]
    Event[State Changed Event]

    Descriptor --> SlotA
    Descriptor --> SlotB
    SlotA --> Active
    SlotA --> Draft
    SlotA --> Event
```

Several slots may share one descriptor, enabling multiple instances of one state type.

## Hook Architecture

Hook descriptors declare their trigger plus read/write state type dependencies. The runtime hook registry filters events and invokes hook bodies supplied by application code.

```mermaid
flowchart LR
    Event[Runtime Event]
    Registry[KekHookRegistry]
    Descriptor[KekHookDescriptor]
    Body[Hook Body]
    Store[KekStateStore]

    Event --> Registry
    Registry --> Descriptor
    Descriptor --> Body
    Body --> Store
```

```mermaid
flowchart TB
    RuntimeLoop[Runtime Loop]
    Prepare[prepare callback]
    Select[select]
    Ready[ready callback]
    HasWork[has_work callback]
    Destroy[destroy callback]
    StateImpl[State Implementation]

    RuntimeLoop --> Prepare
    Prepare --> StateImpl
    RuntimeLoop --> Select
    Select --> Ready
    Ready --> StateImpl
    RuntimeLoop --> HasWork
    HasWork --> StateImpl
    RuntimeLoop --> Destroy
    Destroy --> StateImpl
```

## Stream Architecture

Streams adapt file descriptors to the runtime state interface.

Read streams add their fd to the read set and publish events after reads.

Write streams add their fd to the write set only when buffered data exists and flush buffered data after readiness.

```mermaid
flowchart LR
    FD[File Descriptor]
    Stream[KekStream]
    RuntimeState[KekRuntimeState]
    Runtime[KekRuntime]
    Events[KekEventDispatcher]

    FD <--> Stream
    Stream --> RuntimeState
    RuntimeState --> Runtime
    Stream --> Events
```

## Design Constraints

- Single-threaded execution.
- Fixed maximum number of runtime states.
- Fixed event queue capacity.
- Fixed subscriber capacity per event type.
- Fixed stream buffer capacity.
- Synchronous subscriber invocation.
- Generated state storage owns two caller-sized state buffers.
