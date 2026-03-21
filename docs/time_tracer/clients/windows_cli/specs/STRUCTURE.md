# Windows CLI Structure

## Purpose

Describe the stable responsibility split for the Windows Rust CLI.

## When To Open

- You are deciding where a Windows CLI change belongs.
- You are refactoring CLI/runtime boundaries.
- You are adding a new command or presenter family.

## What This Doc Does Not Cover

- Exact output strings for every command
- Core runtime internals under `libs/tracer_core/**`
- Historical migration steps

## Stable Boundaries

- `src/cli/*`
  - clap-facing command model only
- `src/commands/mod.rs`
  - top-level dispatch only
- `src/commands/handlers/pipeline/*`
  - convert / import / ingest / validate orchestration
- `src/commands/handlers/query/*`
  - semantic data query + tree presentation
- `src/commands/handlers/report/*`
  - report render/export orchestration and shared report request helpers
- `src/commands/handlers/exchange/*`
  - passphrase/input/output handling for tracer exchange flows
- `src/commands/handlers/chart/*`
  - chart presenter logic
- `src/core/runtime.rs`
  - public Rust-side runtime session boundary
- `src/core/runtime/invoke.rs`
  - transport/FFI execution only
- `src/core/runtime/*_client.rs`
  - capability clients over the raw runtime handle
- `src/error/mod.rs`
  - exit-code and stderr contract shaping

## Dependency Direction

- `cli` -> `commands`
- `commands` -> `core::runtime` and `error`
- capability handlers depend on one runtime client at a time
- runtime clients depend on `invoke.rs`
- `invoke.rs` is the only Rust-side layer that talks to C ABI function pointers
- external CLI surface should only use canonical family commands
- do not reintroduce removed compat aliases such as `blink`, `zen`, `--database`, or `--project`

## Change Routing

- Change command names, subcommands, help, or flag shape:
  - start in `src/cli/*`
  - then inspect `src/main.rs`
- Change pipeline/query/report/exchange behavior:
  - start in the matching `src/commands/handlers/<family>/*`
- Change runtime bootstrap, session construction, or capability boundaries:
  - start in `src/core/runtime.rs` and `src/core/runtime/bootstrap.rs`
- Change JSON request/response transport handling:
  - start in `src/core/runtime/invoke.rs`
  - then verify C ABI expectations in `docs/time_tracer/core/contracts/c_abi.md`
