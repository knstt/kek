# Reactive State Machine Language Specification

## 1. Overview & Philosophy

This language combines **explicit pure functions** with **reactive state machines** to create a programming model where:

- **Purity is default**: All functions are deterministic, side-effect-free transformations
- **State is explicit**: Only transitions can change state; all state changes are observable and reactive
- **Safety is verified**: The compiler prevents race conditions (with warnings) and automatically verifies all state constraints
- **Async is implicit**: Independent state transitions run in parallel; the runtime handles threading
- **I/O is state**: Network, file, and user interactions are represented as state objects that change over time

The language targets **C++ code generation** and runs on an **async runtime** that executes transitions in response to state changes.

---

## 2. Core Concepts

### 2.1 Functions (Pure)

Functions are transformations on data with **no side effects**. They:
- Cannot access or modify any state objects
- Are deterministic and cacheable
- Can only take and return values
- Cannot perform I/O, trigger transitions, or change global state

```
fn calculate_discount(price: f64, discount_percent: i32) -> f64 {
  assert!(discount_percent <= 100)
  return price * (1.0 - discount_percent as f64 / 100.0)
}
```

### 2.2 Transitions (Impure)

Transitions are the only operations that change state. They:
- Map old state to new state: `Transition(old_state) -> new_state`
- Construct a **working instance** of the new state
- Verify the working instance against all constraints
- On success, commit the working instance as the new verified state
- On failure, automatically rollback and propagate the error

```
transition AddItem(cart: Cart, item: Item) -> Cart {
  let working_state = Cart {
    items: cart.items + [item],
    total: cart.total + item.price
  }
  return working_state  // Auto-verified before commit
}
```

### 2.3 State Objects

State objects are the only mutable data in the system. They:
- Declare all fields (fully initialized)
- Provide a default constructor
- Define verification rules (compile-time types + runtime assertions)
- Can change via explicit transitions only
- Trigger hooks when they change

```
state Cart {
  items: Vec<Item>,
  total: f64,
  
  default {
    items: [],
    total: 0.0
  }
  
  verify {
    total <= 10000,
    items.len() <= 1000,
    total == sum_prices(items)
  }
}
```

### 2.4 Reactive State Machine Model

When a state changes, it automatically triggers **on_state_change hooks**:

1. State S transitions from v1 → v2
2. Verification passes; v2 becomes the new verified state
3. All `on_state_change` hooks for S are triggered
4. Hooks can trigger new transitions on other states
5. If any cascading transition fails, **full rollback** occurs (S reverts to v1)
6. Error propagates to the root cause of the initial state change

```
on_state_change NetworkRequest {
  if status == Completed {
    transition ParseAndUpdateCart(response)
  }
  
  if status == Failed {
    transition HandleNetworkError(error)
  }
}
```

---

## 3. Type System

### 3.1 Primitive Types

- `i32, i64, u32, u64` (signed/unsigned integers)
- `f32, f64` (floating-point)
- `bool` (true/false)
- `String` (immutable strings)
- `()` (unit type)

### 3.2 Composite Types

- **Tuple**: `(T1, T2, ...)` — fixed-size, heterogeneous
- **Struct**: Named fields (only for non-state data)
- **Enum**: Tagged unions with variants

```
enum Status {
  Pending,
  Completed,
  Failed(String)  // Variants can carry data
}
```

- **Vec<T>**: Dynamic array (immutable outside transitions)
- **Option<T>**: `Some(T)` or `None`
- **Result<T, E>**: `Ok(T)` or `Err(E)` (automatic cascading on error)

### 3.3 State Types

State types are declared with the `state` keyword and can only be modified via transitions.

### 3.4 Default Constructors

Every type must have a default constructor. For state objects, this is explicit:

```
state Cart {
  items: Vec<Item>,
  total: f64,
  
  default {
    items: [],
    total: 0.0
  }
}

// Usage: create Cart with default
let my_cart = Cart::default()
```

---

## 4. Functions

### 4.1 Function Declaration

```
fn name(param1: Type1, param2: Type2) -> ReturnType {
  // function body
  return result
}
```

All functions are pure—they cannot access state objects or perform I/O directly.

### 4.2 Function Calls

```
let discount = calculate_discount(100.0, 20)
```

### 4.3 Automatic Return Verification

If a function returns a state object (or a type that contains one), the return value is **automatically verified** before returning:

```
fn create_cart() -> Cart {
  let cart = Cart {
    items: [],
    total: 0.0
  }
  return cart  // Auto-verified against Cart::verify
}
```

---

## 5. State System

### 5.1 State Declaration

```
state MyState {
  field1: Type1,
  field2: Type2,
  
  default {
    field1: value1,
    field2: value2
  }
  
  verify {
    constraint1,
    constraint2,
    constraint3
  }
}
```

### 5.2 Fields

All fields must be explicitly declared and have a default value. Fields are:
- Immutable within the state (only changed via transitions)
- Fully typed and required (no optional fields unless explicitly `Option<T>`)

### 5.3 Default Constructor

Every state must provide a `default` block that initializes all fields. This is used when creating a new state instance of this type.

### 5.4 Verification Rules

The `verify` block contains constraints that must hold for the state to be valid:

- **Type constraints**: Checked by the compiler at compile-time
- **Runtime assertions**: User-defined conditions checked every time the state is returned

```
verify {
  total <= 10000,           // Assertion: total must not exceed 10k
  items.len() <= 1000,      // Assertion: max 1000 items
  total == sum_prices(items) // Derived: total must match sum
}
```

If verification fails, the state change is rejected and an error is propagated.

---

## 6. Transitions

### 6.1 Transition Declaration

```
transition TransitionName(param1: Type1, param2: Type2) 
  [priority: Priority]
  -> ReturnType {
  
  // Construct working instance
  let working_state = TargetState { ... }
  
  // Return triggers automatic verification
  return working_state
}
```

### 6.2 Working Instance Pattern

Transitions follow a strict pattern:

1. **Construct**: Build a new state instance from input parameters
2. **Verify**: Return statement triggers automatic verification against all constraints
3. **Commit**: On success, the working instance becomes the verified new state
4. **Rollback**: On failure, discard the working instance and propagate the error

```
transition AddItem(cart: Cart, item: Item) -> Cart {
  let working_state = Cart {
    items: cart.items + [item],
    total: cart.total + item.price
  }
  return working_state  // Triggers verification
}
```

### 6.3 Multiple Return Values

Transitions can return tuples of states:

```
transition ProcessBoth(s1: State1, s2: State2) -> (State1, State2) {
  let w1 = State1 { ... }
  let w2 = State2 { ... }
  return (w1, w2)
}
```

### 6.4 Priority

Transitions can specify an explicit priority for queuing:

```
transition HighPriorityFetch(request: NetworkRequest)
  priority: high
  -> NetworkRequest {
  
  let working = NetworkRequest { ... }
  return working
}
```

**Priority levels** (default: `normal`):
- `high` — Queued before normal transitions
- `normal` — Default; FIFO ordering
- `low` — Queued after normal transitions

If no priority is specified, `normal` is assumed.

### 6.5 State Lifecycle

States can be created and deleted via transitions:

```
// Create a new state
transition CreateRequest(url: String) -> NetworkRequest {
  let working = NetworkRequest {
    url: url,
    status: Pending,
    response: "",
    error: null
  }
  return working
}

// Delete a state
transition ClearCart(cart: Cart) -> () {
  delete cart
  return ()
}
```

---

## 7. Reactive State Machine Model

### 7.1 State Change Triggers

When a transition succeeds and commits a new state, the state machine automatically:

1. Triggers all `on_state_change` hooks for that state type
2. Hooks can trigger new transitions (cascading)
3. If any cascading transition fails, **full rollback** occurs

### 7.2 on_state_change Hooks

```
on_state_change StateType {
  if condition1 {
    transition Transition1(args...)
  }
  
  if condition2 {
    transition Transition2(args...)
  }
}
```

Hooks are triggered every time a state of type `StateType` successfully transitions.

### 7.3 Error Propagation & Rollback

If a cascading transition fails:

1. The working instance is discarded
2. The initiating state reverts to its pre-transition value
3. The error propagates back through the hook call stack
4. The original transition is marked as failed

Example:
```
State A: v1 → v2 (succeeds)
  Hook triggered → Transition on State B
    State B: w1 → w2 (fails verification)
  Cascade failure detected
    State A: reverts to v1
    Error propagated to original transition caller
```

### 7.4 Error Handling

Errors use an implicit **Result<T, E>** type:

- If a transition succeeds, the new state is returned as `Ok(state)`
- If a transition fails, an error is returned as `Err(error)`
- The compiler automatically propagates errors up the call stack
- Cascading errors cause full rollback

Users can optionally handle errors explicitly:

```
match some_transition(args) {
  Ok(new_state) => { /* handle success */ },
  Err(error) => { /* handle error */ }
}
```

---

## 8. Concurrency & Parallelism

### 8.1 Implicit Parallelism

**Independent** state transitions run in parallel automatically:

```
transition ProcessBoth(cart1: Cart, cart2: Cart, item: Item) 
  -> (Cart, Cart) {
  
  // These run in parallel (different state objects)
  let c1 = AddItem(cart1, item)
  let c2 = AddItem(cart2, item)
  
  return (c1, c2)
}
```

Transitions on different state objects have no ordering guarantees and execute concurrently.

### 8.2 Per-State Transition Queuing

Transitions for the **same state object** are queued and executed sequentially:

```
// Both transitions try to modify cart
transition A(cart: Cart) -> Cart { ... }
transition B(cart: Cart) -> Cart { ... }

// A and B form a queue on cart:
// 1. A acquires cart, transitions it, commits
// 2. Hook triggers, cascades occur
// 3. B waits until A completes
// 4. B acquires cart, transitions it, commits
```

**Priority-based queuing** allows reordering:

```
transition HighPriority(cart: Cart) priority: high -> Cart { ... }
transition Normal(cart: Cart) -> Cart { ... }
transition Low(cart: Cart) priority: low -> Cart { ... }

// Queue order: HighPriority → Normal → Low
```

### 8.3 Race Condition Detection

The compiler analyzes code for potential race conditions:

- If two transitions could modify the same state object concurrently, the compiler **warns** the programmer
- This is not a hard error—it's a hint that data management may be wrong

Example warning:
```
⚠️  Potential race condition: Transitions A and B both modify state Cart
    Consider: Only one transition should modify a given state at a time
```

### 8.4 Memory Model

**Stack allocation** (for local working states during transitions):
- Working state instances are stack-allocated during transition execution
- Automatically deallocated when the transition returns

**Reference counting** (for long-lived global states):
- Global state objects use atomic reference counting
- When reference count reaches 0, the state is garbage collected
- No manual memory management required

---

## 9. I/O as State

### 9.1 Predefined I/O States

The runtime provides standard I/O state types:

```
state NetworkRequest {
  url: String,
  method: String,  // GET, POST, etc.
  status: Pending | Completed | Failed,
  response: String,
  error: Error?
  
  default {
    url: "",
    method: "GET",
    status: Pending,
    response: "",
    error: null
  }
  
  verify {
    (status == Completed) => response.len() > 0,
    url.len() > 0
  }
}

state FileIO {
  path: String,
  operation: Read | Write,
  status: Pending | Completed | Failed,
  content: String,
  error: Error?
  
  default { ... }
}
```

### 9.2 I/O Hooks

When an I/O state completes, hooks can trigger application logic:

```
on_state_change NetworkRequest {
  if status == Completed {
    transition ParseResponse(response)
  }
  
  if status == Failed {
    transition LogError(error)
  }
}
```

### 9.3 Resource Limits

Global runtime configuration controls I/O resource limits:

```
runtime_config {
  max_workers: 10,        // Max concurrent async tasks
  max_memory_mb: 512,     // Max memory for state objects
  worker_stack_size_kb: 64 // Stack size per worker
}
```

When limits are reached:
- New transitions **wait** (backpressure) until resources become available
- No transitions are rejected; all are eventually executed
- Limits are enforced globally across all state objects

---

## 10. Memory Management

### 10.1 Stack Allocation

Local working states during transitions are allocated on the stack:

```
transition AddItem(cart: Cart, item: Item) -> Cart {
  let working_state = Cart { ... }  // Stack-allocated
  return working_state              // Moved to global state or caller
}
```

### 10.2 Reference Counting

Global state objects use atomic reference counting:

- Each state object has a reference count
- When a transition returns a new state, a reference is created
- When a reference is dropped (end of scope), the count decrements
- When count reaches 0, the state is garbage collected

```
transition CreateAndReturn() -> Cart {
  let cart = Cart { ... }  // refcount = 1
  return cart              // refcount passed to caller
}  // cart refcount incremented in caller scope

// Later, when cart goes out of scope:
// refcount decrements; if 0, cart is freed
```

### 10.3 No Manual Memory Management

Users never explicitly allocate or deallocate memory. The runtime handles:
- Stack allocation for local working states
- Reference counting for global states
- Garbage collection when refcount = 0

---

## 11. Control Flow

### 11.1 If/Else

```
if condition {
  // ...
} else if other_condition {
  // ...
} else {
  // ...
}
```

### 11.2 Pattern Matching

```
match value {
  pattern1 => expression1,
  pattern2 => expression2,
  _ => default_expression
}
```

Pattern matching on enums:

```
match status {
  Pending => { /* handle pending */ },
  Completed => { /* handle completed */ },
  Failed(err) => { /* handle error with err */ }
}
```

### 11.3 Loops

```
// For loop (iteration)
for i in 0..10 {
  // i goes from 0 to 9
}

// For loop (collection iteration)
for item in items {
  // iterate over items
}

// While loop
while condition {
  // ...
}
```

### 11.4 Early Return

```
fn example() -> i32 {
  if error_condition {
    return Err("Something went wrong")  // Error cascades back
  }
  return Ok(result)
}
```

---

## 12. Syntax Summary

### 12.1 Type Declarations

```
// State (mutable, changed via transitions)
state StateName {
  field1: Type1,
  field2: Type2,
  
  default {
    field1: value1,
    field2: value2
  }
  
  verify {
    constraint1,
    constraint2
  }
}

// Struct (immutable, used in functions)
struct StructName {
  field1: Type1,
  field2: Type2
}

// Enum
enum EnumName {
  Variant1,
  Variant2(Type),
  Variant3 { field: Type }
}
```

### 12.2 Function Definition

```
fn function_name(param1: Type1, param2: Type2) -> ReturnType {
  let variable: Type = expression
  return result
}
```

### 12.3 Transition Definition

```
transition TransitionName(param1: Type1, param2: Type2)
  [priority: Priority]
  -> ReturnType {
  
  let working_state = TargetState { ... }
  return working_state
}
```

### 12.4 Hooks

```
on_state_change StateType {
  if condition {
    transition TransitionName(args...)
  }
}
```

---

## 13. Compiler Architecture

### 13.1 Compilation Pipeline

1. **Parsing**: Source code → Abstract Syntax Tree (AST)
2. **Type Checking**: Verify all types are valid, functions are pure
3. **State Analysis**: Verify all state transitions maintain invariants
4. **Race Condition Analysis**: Warn if concurrent access to same state detected
5. **Code Generation**: AST → C++ code
6. **C++ Compilation**: C++ → Machine code

### 13.2 Compiler Guarantees

- **Type Safety**: All types checked at compile-time
- **Purity Enforcement**: Functions cannot access state
- **Verification Guarantee**: Every state return is verified
- **Cascade Safety**: Rollbacks prevent partial state mismatches
- **Race Detection**: Compiler warns about potential concurrent modifications

---

## 14. Runtime Architecture

### 14.1 Async Executor

The runtime is built on an async event loop:

- Maintains a queue of pending transitions (per-state, with priority)
- Detects state changes and triggers hooks
- Executes transitions concurrently where possible (different states)
- Enforces per-state serial execution (same state queues)

### 14.2 State Change Detection

When a transition commits a new state:

1. State change is recorded
2. All `on_state_change` hooks are queued
3. Hooks execute in order
4. If a hook triggers a transition, it's queued
5. Hook execution completes when all triggered transitions complete

### 14.3 Error Propagation

If a transition fails:

1. Working instance is discarded
2. If initiated by a hook, the original state is rolled back
3. Error is propagated to the caller or root initiator
4. Execution stops until the error is handled

---

## 15. Example Programs

### Example 1: Pure Functions Only

```
fn add(a: i32, b: i32) -> i32 {
  return a + b
}

fn main() -> i32 {
  let result = add(5, 10)
  return result
}
```

No state, no transitions—just pure computation.

### Example 2: Simple State & Transitions

```
state Counter {
  value: i32,
  
  default {
    value: 0
  }
  
  verify {
    value >= 0,
    value <= 1000
  }
}

transition Increment(counter: Counter) -> Counter {
  let working = Counter {
    value: counter.value + 1
  }
  return working
}

transition Reset(counter: Counter) -> Counter {
  let working = Counter {
    value: 0
  }
  return working
}

fn main() {
  let counter = Counter::default()
  let c1 = Increment(counter)
  let c2 = Increment(c1)
  let c3 = Reset(c2)
}
```

### Example 3: Reactive State Machine with I/O

```
state AppState {
  user_id: i64,
  user_data: String,
  loading: bool,
  
  default {
    user_id: 0,
    user_data: "",
    loading: false
  }
  
  verify {
    user_id >= 0
  }
}

state NetworkRequest {
  url: String,
  status: Pending | Completed | Failed,
  response: String,
  error: Error?
  
  default {
    url: "",
    status: Pending,
    response: "",
    error: null
  }
  
  verify {
    (status == Completed) => response.len() > 0
  }
}

fn parse_user_json(json: String) -> String {
  // Pure function: parse JSON
  return json
}

transition FetchUser(app: AppState, user_id: i64) -> AppState {
  let working = AppState {
    user_id: user_id,
    user_data: app.user_data,
    loading: true
  }
  return working
}

transition SetUserData(app: AppState, data: String)
  priority: high
  -> AppState {
  
  let working = AppState {
    user_id: app.user_id,
    user_data: data,
    loading: false
  }
  return working
}

on_state_change NetworkRequest {
  if status == Completed {
    let user_data = parse_user_json(response)
    transition SetUserData(user_data)
  }
  
  if status == Failed {
    transition LogFetchError(error)
  }
}

transition LogFetchError(app: AppState, error: Error) -> AppState {
  let working = AppState {
    user_id: app.user_id,
    user_data: "",
    loading: false
  }
  return working
}

fn main() {
  let app = AppState::default()
  let app_loading = FetchUser(app, 42)
}
```

---

## 16. Future Extensions

- **Modules & Imports**: Organizing code into reusable modules
- **Generics & Templates**: Parameterized types and functions
- **Custom Type Constraints**: User-defined type predicates
- **Debugging & Tracing**: Built-in state change logging and replay
- **Distributed Runtime**: Multi-machine state coordination
- **Testing Framework**: Mocking state changes and transitions

---

