# Project Agent Rules

## Scope
This file defines working rules for coding agents in this repository.

## Documentation Synchronization Policy
Any change to runtime or generator implementation must be reflected in documentation in the same task.

### Runtime Changes Require Runtime Docs Updates
When changing files under `runtime/` (or other runtime behavior), update the matching files in:

- `doc/runtime/requirements.md` when requirements/expected behavior change.
- `doc/runtime/specification.md` when interfaces/behavioral rules/protocol details change.
- `doc/runtime/architecture.md` when module boundaries, data flow, or design decisions change.

### Generator Changes Require Generator Docs Updates
When changing generator logic (for example `tools/generate_states.py` or generator-related artifacts), update the matching files in:

- `doc/generator/requirements.md` when generator requirements/capabilities change.
- `doc/generator/specification.md` when emitted output rules or language behavior changes.
- `doc/generator/architecture.md` when generator structure or design decisions change.

## Definition Of Done For Runtime/Generator Work
A task that modifies runtime or generator code is complete only if:

1. Relevant docs are updated in the corresponding `doc/runtime/` or `doc/generator/` folder.
2. Code and docs describe the same behavior.
3. Any intentional divergence is called out explicitly in the final summary.

## Execution Guidance
- Prefer small, traceable edits.
- Keep docs and code changes in the same change set.
- If uncertain which doc file to update, update specification first, then requirements/architecture as needed.
