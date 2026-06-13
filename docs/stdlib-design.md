# Kek Standard Library Design

This document sketches a practical standard library shape for Kek while the
language is still bootstrapping through C. The design favors explicit storage,
explicit allocation, and generic specialization over hidden runtime behavior.

The first standard library should be small enough to self-host the compiler:
bytes, memory, strings, builders, files, stdio, and generic containers.

## Design Goals

- No hidden allocation. Any type that can grow stores or receives an allocator.
- Generic APIs should compile down to specialized C where possible.
- IO should be written against generic reader and writer capabilities, not
  directly against files.
- File handles, stdio handles, memory buffers, and string builders should share
  the same read/write vocabulary.
- Containers should expose simple value semantics where possible and explicit
  cleanup where ownership exists.
- Bootstrap APIs should work with the current compiler; nicer APIs can be added
  once the language has constraints, interfaces, tagged unions, and defer.

## Core Types

The standard library should move toward typed pointers, but the first pointer
model should stay intentionally small:

```kek
ptr<T> // typed pointer to T
ptr    // raw untyped pointer for C interop and opaque handles
```

APIs that can fail should return `Result<T>` or `Status`. Read-only access is
documented by API convention until the language has a stronger type system for
it.

```kek
alias:byte = u8;
alias:usize = u64;
alias:isize = i64;
alias:RawHandle = ptr;

enum:u8:Status {
	Ok,
	End,
	Invalid,
	NoMemory,
	NotFound,
	PermissionDenied,
	Interrupted,
	Unsupported,
	IoError,
};

struct:Result<T> {
	Status:status;
	T:value;
};

struct:Slice<T> {
	ptr<T>:data;
	usize:len;
};

struct:Span<T> {
	ptr<T>:data;
	usize:len;
};
```

`Slice<T>` is read-only by convention and `Span<T>` is mutable by convention.
This is a library contract, not a distinct pointer type. `Result<T>` is a
bootstrap-friendly substitute for a future tagged union.

## Allocation

Dynamic strings and containers use an allocator value. The bootstrap version can
wrap C `malloc`, `realloc`, and `free`.

```kek
struct:Allocator {
	ptr:context;
};

ptr<T>:Allocator::Alloc<T>(usize:count);
ptr<T>:Allocator::Resize<T>(ptr<T>:oldData, usize:oldCount, usize:newCount);
void:Allocator::Free<T>(ptr<T>:data, usize:count);

Allocator:DefaultAllocator();
```

Allocators are passed by value because the value is only a small context handle.
Types that own memory store the allocator they use so `Destroy` does not need an
extra argument.

## Generic IO

The current compiler supports generic functions and methods but not type
constraints. The bootstrap design therefore uses convention:

- A reader type implements `Read(Span<byte>:out) -> Result<usize>`.
- A writer type implements `Write(Slice<byte>:data) -> Result<usize>`.
- Optional helpers implement `Flush`, `Seek`, or `Close`.

Later, these conventions should become constraints:

```kek
constraint:Reader<T> {
	Result<usize>:T::Read(Span<byte>:out);
};

constraint:Writer<T> {
	Result<usize>:T::Write(Slice<byte>:data);
};
```

Bootstrap generic functions can still be written as:

```kek
Result<usize>:ReadAll<R, W>(R:reader, W:writer) {
	u8:buffer[4096];
	usize:total = 0;

	while (true) {
		Result<usize>:read = reader.Read(Span<byte>{ data = &buffer, len = len(buffer) });
		if (read.status == Status::End) {
			return(Result<usize>{ status = Status::Ok, value = total });
		}
		if (read.status != Status::Ok) {
			return(Result<usize>{ status = read.status, value = total });
		}

		Result<usize>:written = writer.Write(Slice<byte>{ data = &buffer, len = read.value });
		if (written.status != Status::Ok) {
			return(Result<usize>{ status = written.status, value = total });
		}

		total += written.value;
	}
}
```

This specializes for every pair of concrete reader and writer types.

## Files And Stdio

Files and stdio are concrete implementations of the reader/writer convention.

```kek
enum:u8:FileMode {
	Read,
	Write,
	Append,
	ReadWrite,
};

struct:File {
	RawHandle:handle;
	bool:owned;
};

Result<File>:FileOpen(Slice<byte>:path, FileMode:mode);
Result<usize>:File::Read(Span<byte>:out);
Result<usize>:File::Write(Slice<byte>:data);
Status:File::Flush();
Status:File::Close();

File:Stdin();
File:Stdout();
File:Stderr();
```

`Stdin`, `Stdout`, and `Stderr` return `File` values with `owned = false`, so
`Close` is a no-op or returns `Unsupported`. This lets generic code use stdio
and files without a separate code path:

```kek
File:input = Stdin();
File:output = Stdout();
ReadAll<File, File>(input, output);
```

## Memory IO

Memory-backed IO makes tests and string formatting easy.

```kek
struct:MemoryReader {
	Slice<byte>:data;
	usize:pos;
};

Result<usize>:MemoryReader::Read(Span<byte>:out);

struct:MemoryWriter {
	Span<byte>:data;
	usize:pos;
};

Result<usize>:MemoryWriter::Write(Slice<byte>:data);
usize:MemoryWriter::Written();
```

`MemoryWriter` does not allocate. It returns `NoMemory` when the fixed span is
full.

## Strings

Strings should be byte strings first. Unicode policy can be layered on top later
with UTF-8 validation and scalar iteration.

```kek
struct:String {
	ptr<byte>:data;
	usize:len;
};

struct:OwnedString {
	ptr<byte>:data;
	usize:len;
	usize:cap;
	Allocator:allocator;
};

String:StringFromBytes(Slice<byte>:bytes);
Slice<byte>:String::Bytes();
bool:String::Equals(String:other);
bool:String::StartsWith(String:prefix);
bool:String::EndsWith(String:suffix);

Status:OwnedString::Destroy();
String:OwnedString::View();
```

`String` is a borrowed view. `OwnedString` owns heap memory. Keeping these
separate avoids accidental frees and keeps function signatures honest.

## Writer And Builder

A string builder is both a growable byte buffer and a writer. This lets generic
formatting code write to files, stdio, fixed buffers, or strings.

```kek
struct:StringBuilder {
	ptr<byte>:data;
	usize:len;
	usize:cap;
	Allocator:allocator;
};

StringBuilder:StringBuilderNew(Allocator:allocator);
Status:StringBuilder::Destroy();
Status:StringBuilder::Reserve(usize:additional);
Status:StringBuilder::Clear();
Result<usize>:StringBuilder::Write(Slice<byte>:data);
String:StringBuilder::View();
Result<OwnedString>:StringBuilder::ToOwnedString();
```

Generic writing helpers should target `W` where `W` implements the writer
convention:

```kek
Status:WriteByte<W>(W:writer, byte:value) {
	u8:buffer[1] = { value };
	Result<usize>:result = writer.Write(Slice<byte>{ data = &buffer, len = 1 });
	return(result.status);
}

Status:WriteString<W>(W:writer, String:text) {
	Result<usize>:result = writer.Write(text.Bytes());
	return(result.status);
}
```

When the language gains method receivers by reference or pointer, writer helpers
should pass mutable writers explicitly so builder/file state is updated
predictably.

## Formatting

Formatting should be generic over writers and should avoid building temporary
strings unless requested.

```kek
Status:FormatI64<W>(W:writer, i64:value);
Status:FormatU64<W>(W:writer, u64:value, u8:base = 10);
Status:FormatBool<W>(W:writer, bool:value);

Status:Print<W>(W:writer, String:text) {
	return(WriteString<W>(writer, text));
}
```

Later, a formatter protocol can let user types provide:

```kek
Status:T::Format<W>(W:writer);
```

## Dynamic Array

`Array<T>` is the primary growable contiguous container. It is the building
block for strings, token buffers, AST buffers, and compiler tables.

```kek
struct:Array<T> {
	ptr<T>:data;
	usize:len;
	usize:cap;
	Allocator:allocator;
};

Array<T>:ArrayNew<T>(Allocator:allocator);
Status:Array<T>::Destroy();
Status:Array<T>::Reserve(usize:additional);
Status:Array<T>::Push(T:value);
Result<T>:Array<T>::Pop();
Result<T>:Array<T>::Get(usize:index);
Status:Array<T>::Set(usize:index, T:value);
Span<T>:Array<T>::Span();
Slice<T>:Array<T>::Slice();
```

The name `Array<T>` is preferred over `List<T>` for contiguous storage. If both
names exist, `List<T>` should mean a higher-level sequence abstraction or be an
alias to `Array<T>` during bootstrap.

## Fixed Array Helpers

Fixed arrays are already language-level values. Library helpers should expose
views instead of copying:

```kek
Slice<T>:FixedSlice<T>(ptr<T>:data, usize:len);
Span<T>:FixedSpan<T>(ptr<T>:data, usize:len);
```

Once the compiler can pass fixed arrays to generics with length information,
these can become:

```kek
Slice<T>:SliceFromArray<T, [comptime]usize:N>(T:data[N]);
```

## Linked Containers

Linked containers should be secondary. They are useful for stable node pointers,
queues with splicing, and compiler worklists, but they are worse defaults than
contiguous arrays.

```kek
struct:ListNode<T> {
	T:value;
	ptr<ListNode<T>>:next;
	ptr<ListNode<T>>:prev;
};

struct:LinkedList<T> {
	ptr<ListNode<T>>:first;
	ptr<ListNode<T>>:last;
	usize:len;
	Allocator:allocator;
};

LinkedList<T>:LinkedListNew<T>(Allocator:allocator);
Status:LinkedList<T>::Destroy();
Status:LinkedList<T>::PushBack(T:value);
Status:LinkedList<T>::PushFront(T:value);
Result<T>:LinkedList<T>::PopBack();
Result<T>:LinkedList<T>::PopFront();
```

## Map And Set

Hash containers need generic hooks for hash and equality. Until the language has
generic value parameters or constraints, use function-pointer fields or explicit
policy structs.

```kek
alias:HashFunc<K> = fn(K) -> u64;
alias:EqualsFunc<K> = fn(K, K) -> bool;

struct:HashPolicy<K> {
	HashFunc<K>:hash;
	EqualsFunc<K>:equals;
};

struct:HashMap<K, V> {
	ptr<HashEntry<K, V>>:entries;
	usize:len;
	usize:cap;
	HashPolicy<K>:policy;
	Allocator:allocator;
};

HashMap<K, V>:HashMapNew<K, V>(Allocator:allocator, HashPolicy<K>:policy);
Status:HashMap<K, V>::Destroy();
Status:HashMap<K, V>::Insert(K:key, V:value);
Result<V>:HashMap<K, V>::Get(K:key);
bool:HashMap<K, V>::Contains(K:key);
Status:HashMap<K, V>::Remove(K:key);
```

`HashSet<T>` can be implemented as `HashMap<T, bool>` during bootstrap, then
specialized later.

## Other Containers

These are useful but should follow after `Array<T>`, `StringBuilder`, and
`HashMap<K, V>`:

- `Deque<T>`: ring buffer for queues.
- `Stack<T>`: thin wrapper around `Array<T>`.
- `Queue<T>`: thin wrapper around `Deque<T>`.
- `PriorityQueue<T>`: binary heap with comparator policy.
- `Arena<T>` or `Arena`: monotonic allocation for compiler phases.
- `Pool<T>`: stable indexed storage with reuse.
- `BitSet`: packed booleans for semantic flags.

Comparator-based containers should use a policy object like `HashPolicy<T>`:

```kek
alias:CompareFunc<T> = fn(T, T) -> i32;

struct:ComparePolicy<T> {
	CompareFunc<T>:compare;
};
```

## Ownership Pattern

Owned types should consistently provide:

- `New` or `Init` constructor.
- `Destroy` method.
- `Clear` method when reuse is cheap.
- `Reserve` method when the type grows.
- `Slice` or `View` method for borrowed access.

Borrowed types should never free memory. Handles that may or may not be owned,
such as stdio files, carry an `owned` flag.

## Proposed Package Layout

```text
std/core.kek       aliases, Status, Result, Slice, Span
std/mem.kek        Allocator, memory copy/set helpers
std/io.kek         reader/writer conventions and generic helpers
std/file.kek       File, FileOpen, stdio handles
std/string.kek     String, OwnedString, StringBuilder
std/array.kek      Array<T>
std/list.kek       LinkedList<T>
std/hash.kek       HashMap<K, V>, HashSet<T>
std/collections.kek re-exports common containers
```

## Bootstrap Order

1. Add `Status`, `Result<T>`, `Slice<T>`, `Span<T>`, and `Allocator`.
2. Implement `Array<byte>` and `StringBuilder` first, then generalize to
   `Array<T>` once generic struct methods are reliable.
3. Wrap C `FILE*` as `File` and implement `Read`, `Write`, `Flush`, `Close`.
4. Add `MemoryReader` and `MemoryWriter` for tests.
5. Write generic IO helpers: `ReadAll`, `WriteString`, integer formatting.
6. Build `OwnedString` on top of `Array<byte>` or `StringBuilder`.
7. Add `HashMap<K, V>` after function pointer aliases or policy structs are
   stable enough.

## Language Features To Add Later

These are not required for the first implementation, but they would make the
standard library much cleaner:

- Generic constraints for `Reader<T>`, `Writer<T>`, `Hashable<T>`, and
  `Comparable<T>`.
- Tagged unions for `Result<T, E>` instead of a status-plus-value struct.
- Defer for deterministic cleanup at every return path.
- Generic compile-time value parameters for fixed-capacity containers.
- Borrowing or explicit pointer receivers for mutating generic writers.
