# Runtime Transaction Design Options

This note captures the rollback and copy strategy options for hook transactions.
It is intentionally exploratory; it is not a committed specification.

## Context

Hooks run inside a transaction because a hook can fail after changing state or
publishing events. On failure, the runtime must restore state and event queue
contents to the point before that hook invocation.

The current state store already uses two buffers per slot for normal updates:

- `active_index` points at the committed buffer.
- An update copies active data into the inactive buffer.
- The update callback mutates the inactive buffer.
- On success, the inactive buffer becomes active.
- On validation failure, the active buffer remains unchanged.

That model handles a single update well. Hook transactions need more machinery
because one hook can make multiple writes, write the same slot more than once,
create slots, delete slots, and publish events.

## Option 1: Full Store Snapshot

This was the original hook transaction behavior.

- Before every hook, copy both buffers for every live state slot.
- If the hook succeeds, free the snapshot.
- If the hook fails, replace the whole store with the snapshot.

Pros:

- Simple correctness model.
- Handles update, repeated update, create, and delete uniformly.
- Easy rollback implementation.

Cons:

- Cost scales with total live slots, not with what the hook actually writes.
- Allocates and frees heavily under hook-heavy workloads.
- Stress trace showed about `895k` malloc/free calls and `83 MB` allocated/freed.

## Option 2: Snapshot Only Declared Writable Types

This is the optimization currently implemented.

- Each hook descriptor already declares `writes`.
- Before a hook runs, snapshot only slots whose state type is in `writes`.
- Keep metadata for all pre-existing slots.
- On rollback:
  - Restore snapshotted writable slots.
  - Keep unsnapshotted slots intact.
  - Remove hook-created slots.
  - Restore empty slots that were reused during the hook.

Pros:

- Preserves current public behavior.
- Uses existing hook write authorization as the snapshot boundary.
- Low design risk.
- Stress trace dropped to about `7.3k` malloc/free calls and `369 KB`
  allocated/freed.

Cons:

- Still snapshots all live slots of a writable type.
- If a hook writes one `Telemetry` slot but there are many `Telemetry` slots,
  it still snapshots all of them.
- Still allocates before knowing whether the hook will actually write anything.

## Option 3: Lazy Write Journal

This option defers snapshots until the hook actually writes.

- Begin hook transaction with no buffer copies.
- On first mutation/delete of a slot, record that slot's original metadata and
  buffers in a transaction journal.
- On slot create, record that the slot did not exist before the transaction.
- On success, discard the journal.
- On failure, replay the journal backward to restore only touched slots.

Pros:

- Cost scales with touched slots.
- Hooks that inspect state and do not write allocate nothing for state rollback.
- Hooks that write one slot snapshot only one slot.
- Natural place to track create/delete rollback.

Cons:

- More complex than declared-type snapshots.
- Every mutating state-store API must participate in the journal.
- Repeated writes to the same slot must be coalesced so only the original state is
  preserved.
- Rollback ordering matters for create/delete/reuse cases.

## Option 4: Copy-On-Write Hook Transaction

This is a tighter version of the lazy journal that uses the existing two-buffer
slot model where possible.

- At transaction begin, record per-slot metadata such as original active index.
- Do not copy any slot immediately.
- On first write to a slot:
  - Protect the original active buffer for rollback.
  - Use the inactive buffer as the transaction draft.
  - Mark the slot dirty in the transaction.
- Repeated writes to the same slot update the transaction draft, not the original
  buffer.
- On success, mark each dirty transaction draft as active.
- On failure, restore each dirty slot's original active index.
- Use a small journal for create/delete cases.

Pros:

- Avoids copying unchanged writable slots.
- Can avoid heap allocation for many update-only transactions.
- Builds on the current double-buffer storage model.
- Likely best performance path for hook-heavy workloads.

Cons:

- The current update path flips `active_index` immediately on each successful
  update. That behavior must change inside hook transactions.
- The original active buffer must stay protected for the full hook transaction.
- Repeated writes to the same slot need clear transaction-draft semantics.
- Create/delete/reuse still need a journal.
- Event publication must stay transactionally aligned with state commits.

## Key Edge Cases

- Hook writes the same slot twice, then fails.
- Hook creates a slot, then fails.
- Hook deletes a slot, then fails.
- Hook deletes a slot, reuses the same slot id for a new state, then fails.
- Hook writes multiple slots and a later write fails validation.
- Hook publishes events and then fails.
- Hook triggers nested events whose hooks perform additional writes.
- Hook has `writes = []` and should not pay state rollback costs.

## Current Recommendation

Keep the declared-writable-type snapshot optimization as the conservative
baseline. Consider the copy-on-write hook transaction as the next major design
step if stress workloads continue to be dominated by transaction allocation or
state copy costs.

Before implementing copy-on-write transactions, define the exact transaction
semantics for repeated writes, create/delete/reuse, and event publication.
