---
trigger: always_on
---

# Repository Agent Entry

This file is the repo-level index and hard-rule set. Keep detailed workflows in
the app / domain docs below; use this file only when no closer instructions
exist.

## Hard Rules

- Use `pwsh` (PowerShell 7.6.0) as the default shell entry.
- Run `.sh` workflows only when explicitly requested.
- Use `python tools/run.py ...` as the default project build / verify /
  validate entry. Avoid ad-hoc `cmake`, `ninja`, Gradle, or native wrappers
  unless a local doc allows it or the user explicitly asks.
- Build / verify success is determined by process exit code: `0` passes,
  non-zero fails. On failure, report the command and key error lines.
- Do not infer validation scope from a dirty worktree. Use explicit focused
  paths for `validate`.
- Documentation-only changes may skip build / test by default unless they touch
  code, config, scripts, tests, or the user asks for verification.
- Heavy flows such as full matrices, `tidy-flow`, installer packaging, or push
  gates run only when requested or when the relevant local doc requires them.
- Never revert user changes in a dirty worktree unless explicitly requested.

## Command Entry

Use the tool itself as the command reference:

```powershell
python tools/run.py -h
python tools/run.py <subcommand> -h
python test/run.py -h
```

Core command semantics:

- `build`: compile artifacts only.
- `verify`: build plus Python checks, suites, native smoke, and quality gates.
- `validate`: focused path-scoped orchestration for local changes.

If a command returns `unrecognized arguments`, rerun the relevant `-h` before
retrying.

## Instruction Resolution

Prefer the closest local instructions:

1. `apps/<target_app>/AGENTS.md`
2. `apps/<target_app>/agent.md`
3. `apps/<target_app>/README.md`
4. This file for global defaults only

High-signal entry docs:

- Android: `apps/android/AGENTS.md`
- Windows CLI: `apps/cli/windows/AGENTS.md`
- Log generator: `apps/tools/log_generator/AGENTS.md`
- Core read-first:
  - `.agent/guides/docs/tracer_core-read-first.md`
  - `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`

## Domain Gates

- Before editing `libs/tracer_core/**` or core shell boundaries, read the core
  read-first docs above.
- Before editing C ABI symbols / signatures, read
  `docs/time_tracer/core/contracts/c_abi.md`.
- Before editing report chart fields / semantics, read
  `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md` and sync
  the adjacent stats schema / README docs it names.
- Keep application / domain layers free of exchange-format JSON dependencies:
  no `nlohmann/json` under `libs/tracer_core/src/domain/**` or
  `libs/tracer_core/src/application/**`; do not expose `nlohmann::json` as an
  application-layer public type.
- `assets/tracer_core/config` is the canonical shared runtime config source.
  App-local config directories are generated snapshots, not source-of-truth.

## Outputs

When reporting verification results, prefer these locations:

- Validation summary: `out/validate/<run_name>/summary.json`
- Validation logs: `out/validate/<run_name>/logs/output.log`
- Test summary: `out/test/<result_target>/result.json`
- Aggregated test log: `out/test/<result_target>/logs/output.log`

Common result targets:

- `tracer_android` -> `artifact_android`
- `log_generator` -> `artifact_log_generator`
- Core / Windows CLI targets -> `artifact_windows_cli`

## Working Discipline

- Keep refactors separate from feature changes unless the refactor is required
  to make the feature safe.
- For long files, stabilize boundaries in-place before splitting files.
- Store temporary files under repository `temp/` unless the user asks otherwise.
- If a request is incorrect, risky, or clearly suboptimal, say so and propose a
  safer path before executing.
