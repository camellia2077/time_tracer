# tracer_windows_cli Structure

This document defines stable ownership boundaries for `apps/tracer_cli/windows`.
Goal: keep future changes in "feature extension" mode instead of frequent architecture rewrites.

## 1. Module Ownership

1. Bootstrap layer: `apps/tracer_cli/windows/src/bootstrap/`
   - Runtime factory loading/proxy/check.
   - Dynamic loading of core C ABI and runtime wiring.
   - Should not contain command argument semantics.

2. CLI app layer: `apps/tracer_cli/windows/src/api/cli/impl/app/`
   - Entry orchestration (`app_runner`, `cli_application`).
   - Global parser policy, command dispatch, top-level exception/exit mapping.
   - Should not contain business details of each command.

3. Command layer: `apps/tracer_cli/windows/src/api/cli/impl/commands/`
   - Command definitions, parameter validation, process orchestration.
   - Grouped by domain: `chart/`, `crypto/`, `export/`, `general/`, `pipeline/`, `query/`.
   - `register_all_commands.cpp` is the single command registration aggregator.

4. Presentation layer: `apps/tracer_cli/windows/src/api/cli/impl/presentation/`
   - User-facing output rendering only.
   - Current focus:
     - `presentation/report_chart/` for chart HTML rendering.
     - `presentation/progress/` for crypto progress rendering.
   - Should not call core runtime directly.

5. CLI framework layer: `apps/tracer_cli/windows/src/api/cli/framework/`
   - Reusable parser/registry contracts and infrastructure.
   - No command-specific business logic.

6. Utilities layer: `apps/tracer_cli/windows/src/api/cli/impl/utils/`
   - Shared formatting/console/path helpers.
   - Keep stateless as much as possible.

## 2. Stable Execution Flow

1. `main.cpp` -> `AppRunner::Run(...)`
2. `CliApplication` parses args and builds runtime (`bootstrap` factory).
3. `RegisterAllCommands()` registers command creators to `CommandRegistry`.
4. `CommandRegistry` resolves command by name and executes command instance.
5. Command calls core runtime API (via app context/runtime factory output).
6. Presentation modules render output/progress/help text.

## 3. Change Routing (Where To Modify)

1. Add a new command:
   - Add file under `impl/commands/<group>/<name>_command.cpp`
   - Register in `impl/commands/register_all_commands.cpp`
   - Keep output logic in `impl/presentation/*` when output is non-trivial.

2. Change command output style/colors:
   - Command text + presentation modules in `impl/presentation/*`
   - Must follow:
     - `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
     - `docs/time_tracer/clients/windows_cli/specs/console-color.md`

3. Change runtime/core call adaptation:
   - `src/bootstrap/cli_runtime_factory_proxy.cpp`
   - Related loader/check files in `src/bootstrap/`

4. Change global parser/help/exit behavior:
   - `impl/app/cli_application.cpp`
   - `EXIT_CODE_POLICY.md` when exit semantics change

## 4. Guardrails

1. `commands/` should orchestrate, not own rendering complexity.
2. `presentation/` should render, not own business decisions.
3. `bootstrap/` should adapt runtime, not parse command semantics.
4. Cross-cutting text/format changes should update style docs in the same change.

## 5. Minimal Verification

Run from repository root:

```powershell
python scripts/run.py verify --app tracer_core --quick
```

Optional split flow:

```powershell
python scripts/run.py configure --app tracer_windows_cli --build-dir build_fast
python scripts/run.py build --app tracer_windows_cli --build-dir build_fast
python test/run.py --suite tracer_windows_cli --build-dir build_fast --agent --concise
```

