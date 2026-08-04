# LOC Scanner Refactoring Instructions

These instructions apply to refactoring the Python LOC Scanner under this
directory. They describe the scanner-specific refactoring workflow and the
documents needed to make a decision. They do not define the architecture of
the application code scanned by the tool.

## Required Documents

Before changing scanner code, read:

1. [Scanner implementation layout](docs/architecture.md)
   - Module ownership and current implementation boundaries.
2. [Scanner usage guide](README.md)
   - Supported commands, profiles, output files, and baseline behavior.
3. [Scanner configuration reference](docs/toml_config.md)
   - Required when changing TOML fields, profiles, thresholds, categories, or
     source-set classification.
4. [Scanner usage details](docs/usage.md)
   - Required when changing CLI behavior, profile behavior, reports, or
     baseline comparison.
5. [Shared refactoring guidance](../../../docs/time_tracer/architecture/refactoring_guidance.md)
   - Responsibility boundaries, coupling, cohesion, evidence, and prohibited
     mechanical splits.

Read the relevant tests before editing the behavior they cover:

- `tests/test_config_loading.py` for configuration parsing;
- `tests/test_path_resolution.py` for classification, summaries, guidance, and
  report contracts;
- `tests/test_comparison.py` for baseline comparison semantics;
- `tests/test_report_writer.py` for report artifacts and baseline files;
- `tests/test_reporter.py` for console output contracts.

## Refactoring Workflow

```text
LOC Scanner finds a scanner hotspot
    ↓
Agent maps responsibilities, callers, data flow, and change reasons
    ↓
Agent confirms a real responsibility boundary
    ↓
Add or identify characterization tests
    ↓
Split by responsibility, not by method count or LOC
    ↓
Run focused tests and a scanner smoke test
    ↓
Compare the baseline and inspect dependency/cohesion changes
```

## Decision Rules

- A large file is a review candidate, not proof that it must be split.
- Keep cohesive configuration models together when they change as one unit.
- Keep report persistence together when the artifact lifecycle is shared, but
  separate independent report renderers when their formats change for
  different reasons.
- Keep `LocConsoleReporter` as the public console-output facade unless callers
  can safely migrate to narrower renderer interfaces.
- Do not create one class per `print_*`, validation helper, or CRUD-like
  operation.
- Do not move methods into another large class without improving ownership,
  dependency direction, or independent testability.
- Do not change report JSON schema, baseline semantics, CLI exit codes, or
  output contracts without explicit evidence and tests.
- User-visible scanner prompts must remain in English to avoid console
  encoding problems. `config.py` is not part of a refactor unless the task
  explicitly includes configuration behavior or data-model changes.

## Validation Requirements

At minimum, run:

```bash
python -m pytest tools/devtools/loc_scanner/tests -q
python -m compileall -q tools/devtools/loc_scanner/src
```

For profile or report changes, also run:

```bat
tools\devtools\loc_scanner\scripts\profile\run_loc_scanner.bat --over 200
```

For a refactoring comparison, save a baseline before editing and compare it
after editing. Treat changed file counts and LOC as supporting evidence only;
the acceptance decision must explain responsibility ownership, dependency
direction, cohesion, behavior coverage, and remaining risk.
