# Windows CLI Refactoring Requirements

This document supplements the shared refactoring workflow in
`docs/time_tracer/architecture/refactoring_guidance.md`. It is the
component-specific decision standard for `apps/cli/windows/rust/**`.

## Purpose

The Windows CLI is a thin Rust client hosted by `tracer_core.dll`. Its job is
to translate user intent into canonical runtime calls and translate results
into terminal/file output. Refactoring must make those responsibilities easier
to see and test without moving core business semantics into the client.

## Ownership requirements

| Area | CLI owns | CLI must not own |
| --- | --- | --- |
| `src/cli/**` | clap command/argument models, defaults, help shape | validation rules, canonical domain models, business decisions |
| `src/commands/**` | dispatch, command orchestration, filesystem and terminal UX | TXT, alias, report, ingest, or exchange semantics already owned by Core |
| `src/core/runtime/**` | session bootstrap, capability-client calls, ABI transport, callback adaptation | a second business service layer, semantic caches, or a second Runtime protocol |
| `src/error/**` | exit-code and user-facing diagnostic shaping | recovery by reimplementing Core rules |
| `tracer_core.dll` / `libs/**` | canonical parsing, validation, persistence, query, report and exchange semantics | — |

The dependency direction remains:

```text
cli models -> command handlers -> capability clients -> invoke/ABI boundary -> Core
                         \-> error shaping
```

`core/runtime/invoke/**` is the only Rust-side layer that talks directly to C
ABI function pointers. Any extraction around it must preserve that single
transport seam.

## Refactoring requirements

1. Establish the responsibility and reason for change before moving code. A
   large file or a scanner hotspot is only a reading candidate.
2. Split by an independently changing responsibility: command model, command
   orchestration, request preparation, result presentation, filesystem
   coordination, runtime capability client, or transport/callback adaptation.
3. Keep one owner for each request/response contract. Do not introduce a
   duplicate DTO, semantic cache, protocol representation, or pass-through
   facade merely to reduce file size.
4. Keep command handlers thin. Date/path normalization, validation, alias
   resolution, report semantics, ingest ordering, and exchange semantics must
   remain delegated to the canonical Core contract unless the CLI-only part is
   explicitly presentation or input-shape adaptation.
5. Keep filesystem mutation and runtime invocation sequencing explicit. In
   particular, failed ingest must retain the no-database/no-sidecar invariant;
   CLI cleanup must not compensate for a broken Core persistence boundary.
6. Preserve command names, flags, exit codes, stderr wording contracts,
   generated runtime config snapshots, C ABI payloads, and black-box suite
   behavior unless the change explicitly includes those contracts.
7. Prefer narrow, testable seams over a new large coordinator. A new module is
   justified only when its owner, inputs/outputs, and independent validation
   path are clear.

## Hotspot review focus

The following are review prompts for the current CLI hotspots, not mandatory
split instructions:

- `src/cli/{mod,root,tests}.rs`: keep the root command model and
  dispatch-facing enums cohesive, while keeping the parser characterization
  suite outside the production model facade. Move a command family only when
  its argument model changes independently.
- `src/commands/handlers/alias.rs`: distinguish file/TOML editing UX,
  Core-backed canonical migration orchestration, and result presentation before
  extracting anything. Canonical alias semantics remain in Core.
- `src/commands/handlers/report/{mod,tests,requests,dates,formats}.rs` and
  `report/**`: retain `report/mod.rs` as the session/dispatch facade; keep
  parser/request construction, date normalization, format/path handling, and
  render/export/chart presentation separated by reason to change. Do not
  split `ReportSession` methods merely by operation while they share the same
  runtime bootstrap and reporting contract.
- `src/core/runtime/invoke/**`: preserve the single FFI transport boundary;
  keep ABI response DTOs, transport/error mapping, and capability-specific
  calls inside the runtime invoke subtree rather than spreading unsafe calls
  into command handlers.
- `src/commands/handlers/txt/{mod,view,append,format,tests}.rs`,
  `exchange/**`, and `pipeline/**`: keep input collection, runtime
  orchestration, and terminal/file presentation distinct while preserving
  atomicity and canonical Core semantics.
- `src/commands/handlers/exchange/{mod,tests}.rs`: retain `exchange/mod.rs` as
  the session/prompt/progress/dispatch facade. Treat export, import, unpack,
  and inspect as one exchange contract family until their bootstrap, prompt,
  and progress lifecycles can change independently.

## Acceptance evidence

Every CLI refactoring proposal or implementation must record:

- the current owner, callers, dependencies, and reason-to-change map;
- the boundary being introduced and why it is a real responsibility boundary;
- the affected command, Runtime/ABI, config, output, and persistence contracts;
- focused tests for changed behavior, plus the owning black-box suite case;
- the dependency/cohesion result after the change; lower LOC alone is not
  acceptance evidence.

Required validation for Rust CLI changes is:

```powershell
python tools/run.py verify --app tracer_core --concise
```

If Core/native code changes, rebuild Core before rebuilding the Rust CLI. For
release-bundle confirmation use the core-then-CLI order from
`apps/cli/windows/AGENTS.md`. Inspect
`out/test/artifact_windows_cli/result.json` and its `output.log` when the
black-box suite is applicable.

## Explicit non-goals

- Do not split every method, CRUD-like operation, or output line formatter
  into a class/module.
- Do not move business logic from `libs/**` into the CLI to make a handler look
  smaller.
- Do not add compatibility aliases or legacy flags while refactoring.
- Do not accept a lower scanner score, fewer hotspots, or fewer lines as proof
  that the architecture improved.
