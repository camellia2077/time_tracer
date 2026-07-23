# tracer_core Boundary Checklist

Use this checklist after `libs/tracer_core/AGENTS.md`. It supplements the local
contract with review questions; it does not redefine task routing, validation,
or completion requirements.

## Before Editing

1. Identify the owning capability, public surface, direct consumers, contract,
   and focused regression coverage.
2. Locate the boundary being changed: domain, application, transport, bridge,
   adapter, shell runtime, or presentation host.
3. Read only the task-specific contracts routed by the local `AGENTS.md`.

## Boundary Checks

- Business semantics belong in `tracer_core`; JSON envelopes and codecs belong
  in `tracer_transport`; filesystem/process mechanics belong in adapters.
- Bridges map stable boundaries. They must not acquire business policy or
  presentation behavior.
- Aggregate runtime APIs may compose capability-owned operations, but wiring
  and host lifecycle stay outside the domain layer.
- C ABI, JNI, and public transport headers are explicit stable boundaries.
  Do not replace an include with a module import mechanically.

## Ingest And Persistence

- Preserve authored facts through parsing; validate semantic timelines before
  persistence.
- A rejected input must not partially mutate the database.
- Keep authored event timing separate from canonical activity-path mapping and
  downstream query/report aggregation.

## Evidence

- Inspect the direct producer and consumer on both sides of a changed boundary.
- Keep the relevant contract and a focused regression aligned with the code.
- Use `libs/tracer_core/AGENTS.md` for the exact validation choice and completion
  bar.
