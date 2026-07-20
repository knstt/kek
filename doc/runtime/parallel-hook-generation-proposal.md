# Generator-Assisted Parallel Hook Scheduling Proposal

Related documents:

- [Runtime Specification](specification.md)
- [Runtime Architecture](architecture.md)
- [Generator Specification](../generator/specification.md)

## Summary

The runtime can safely parallelize more hooks if the generator emits precise access metadata instead of only state-type-level `reads` and `writes`. The runtime should not infer behavior from arbitrary C hook bodies. It should schedule from generated declarations that describe which state types, slots, and fields a hook may read or write.

The current runtime only parallelizes read-only hooks with concrete `reads` metadata. This is safe but conservative. The next design should allow independent write hooks to run concurrently when their declared write targets cannot overlap and their writes do not affect another hook's reads.

## Schema Changes

Keep the existing simple form:

```json
{
  "name": "OnAgentMoved",
  "on": {"state": "Agent", "event": "changed", "fields": ["x", "y"]},
  "reads": ["Agent"],
  "writes": ["Telemetry"]
}
```

Add an optional precise access form for hooks that want maximum parallelism:

```json
{
  "name": "OnPlayerScoreChanged",
  "on": {"instance": "player_score", "event": "changed", "fields": ["score"]},
  "access": {
    "reads": [
      {"instance": "player_score", "fields": ["score"]},
      {"state": "GameClock", "scope": "declared"}
    ],
    "writes": [
      {"instance": "scoreboard", "fields": ["total", "rank"]}
    ],
    "creates": [],
    "deletes": []
  }
}
```

Access entries:

| Field | Meaning |
| --- | --- |
| `instance` | Exact schema-declared instance. Best precision; resolves to one slot at runtime binding. |
| `state` | Generated state type. Used for state-wide or dynamic-slot access. |
| `scope` | `declared`, `dynamic`, or `any`; defaults to `any` for state-level entries. |
| `fields` | Field names touched by the hook. Omitted means unknown/all fields. |
| `slot` | Optional future low-level slot selector for advanced generated bindings; normal schemas should prefer `instance`. |

Rules:

- `reads`/`writes` stay supported and are treated as state-level `scope: "any"` with unknown fields.
- `access` and legacy `reads`/`writes` may coexist during migration, but `access` is the scheduling authority when present.
- Missing `access` keeps current conservative behavior.
- Hooks that call arbitrary raw store APIs should declare broad access, or use a future `access.mode: "opaque"` escape hatch to force serial dispatch.

## Generated Runtime Metadata

Add a runtime access descriptor type:

```c
typedef enum KekHookAccessScope {
    KEK_HOOK_ACCESS_SCOPE_ANY = 0,
    KEK_HOOK_ACCESS_SCOPE_DECLARED = 1,
    KEK_HOOK_ACCESS_SCOPE_DYNAMIC = 2,
    KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT = 3
} KekHookAccessScope;

typedef enum KekHookAccessMode {
    KEK_HOOK_ACCESS_READ = 0,
    KEK_HOOK_ACCESS_WRITE = 1,
    KEK_HOOK_ACCESS_CREATE = 2,
    KEK_HOOK_ACCESS_DELETE = 3
} KekHookAccessMode;

typedef struct KekHookAccess {
    KekHookAccessMode mode;
    size_t state_type_id;
    size_t state_slot_id;
    KekHookAccessScope scope;
    uint64_t fields;
} KekHookAccess;
```

Extend `KekHookDescriptor`:

```c
const KekHookAccess* accesses;
size_t access_count;
uint32_t scheduling_flags;
```

Useful flags:

| Flag | Meaning |
| --- | --- |
| `KEK_HOOK_SCHEDULING_OPAQUE` | Force serial dispatch. |
| `KEK_HOOK_SCHEDULING_ALLOW_PARALLEL_WRITES` | Descriptor has enough precise write metadata for parallel write scheduling. |
| `KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE` | Runtime may merge disjoint field writes to the same slot. This should be opt-in only. |

Generator behavior:

- For `access.instance`, emit `state_slot_id = KEK_HOOK_UNRESOLVED_SLOT` in the static descriptor, then patch it to the resolved declared slot id during runtime binding, matching current instance-trigger behavior.
- For `access.state` with `scope: "declared"`, emit state type plus declared scope.
- For `access.state` with `scope: "dynamic"` or `any`, emit broad state-level access.
- Convert field names into the existing generated field mask macros.
- Preserve legacy `.reads`, `.writes`, `.read_count`, and `.write_count` for compatibility until the new runtime scheduler fully replaces them.

## Runtime Scheduling Rules

For each dispatched event:

1. Collect matching hook descriptors in descriptor order.
2. Convert each descriptor's accesses into normalized read/write/create/delete sets.
3. Partition adjacent descriptors into parallel waves.
4. Run descriptors in a wave concurrently only when every pair is independent.
5. Commit worker results on the main runtime thread in descriptor order.

Two hooks conflict when any of these are true:

- One writes, creates, or deletes a state/slot the other reads.
- Both write, create, or delete the same exact slot.
- Both write the same state type with `scope: "any"` or overlapping `declared`/`dynamic` scopes.
- One deletes a state/slot the other reads or writes.
- Either descriptor is opaque.
- Either descriptor has unknown/all fields and the other touches the same slot.
- They write overlapping fields on the same exact slot.

Possible parallel cases:

| Hook A | Hook B | Parallel? |
| --- | --- | --- |
| Reads `Agent`, writes `Telemetry` declared slot `telemetry_a` | Reads `Agent`, writes `AuditLog` declared slot `audit` | Yes, if neither reads the other's write target. |
| Writes `PlayerScore.score` on declared slot `player_score` | Writes `EnemyScore.score` on declared slot `enemy_score` | Yes. |
| Writes `Telemetry.load` | Writes `Telemetry.hook_hits` on the same slot | Only if `KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE` is set and merge semantics are implemented. |
| Writes dynamic `Agent` | Reads any `Agent` | No. |
| Deletes any `Packet` | Reads or writes `Packet` | No. |

## State Commit Model

To parallelize write hooks, workers need isolated mutation results rather than direct mutation of the live store.

Required runtime work:

- Add a per-hook isolated transaction result that records slot changes, created slots, deleted slots, buffered events, quit requests, and trace timing.
- Let workers read from a committed snapshot and write to private drafts.
- Validate drafts in the worker.
- Apply successful results on the main thread in descriptor order.
- Before applying each result, verify that every read and write precondition still matches the snapshot version used by the worker. If a precondition changed, rerun that hook serially or fail the wave and fall back to serial replay.

Recommended first commit policy:

- Exact-slot writes can be applied in descriptor order when no conflict was scheduled.
- Creates/deletes should remain serial initially unless the schema declares exact non-overlapping state scopes.
- Same-slot disjoint-field merging should be a later opt-in feature, because it requires field-level patch application instead of whole-slot buffer replacement.

## Generator Helper Restrictions

The generator can unlock more parallelism if hook bodies use generated helpers with known targets.

Recommended convention:

- Instance-specific helpers are considered precise access-compatible.
- State-wide dynamic helpers are broad access unless schema access says otherwise.
- Raw `KekStateStore` mutation helpers remain allowed but should require broad or opaque access declarations.

Optional future strict mode:

```json
{
  "hooks": {
    "access_validation": "strict"
  }
}
```

In strict mode, generated hook headers can expose only access-compatible helper families for each hook, making accidental broad writes harder. This is a larger API change and should not be required for the first implementation.

## Implementation Plan

1. Add parser/model support for optional `hook.access`.
2. Validate that every `instance`, `state`, and `field` reference exists.
3. Emit `KekHookAccess` arrays beside existing generated read/write arrays.
4. Patch instance access `state_slot_id` values during generated runtime binding init.
5. Extend `KekHookDescriptor` with access metadata while preserving legacy fields.
6. Update runtime conflict detection to prefer access metadata and fall back to legacy conservative scheduling.
7. Add isolated write transaction results and main-thread apply logic.
8. Enable parallel write hooks only for exact-slot, non-overlapping access sets.
9. Add generator and runtime tests for exact-slot writes, state-wide writes, dynamic writes, deletes, failures, event order, tracing, and serial fallback.

## Acceptance Criteria

- Existing schemas continue to generate and run unchanged.
- Existing read-only hook parallelism remains unchanged.
- Hooks with precise, non-overlapping write access run concurrently.
- Hooks with ambiguous access remain serial.
- Failed write hooks roll back their state changes and buffered events.
- Worker-produced events are replayed in descriptor order.
- Tracing remains valid and aggregated on the main runtime.
- Stress coverage includes both read-only parallel hooks and independent write hooks.

