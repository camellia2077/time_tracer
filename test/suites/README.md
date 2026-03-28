# suites

Canonical table-driven artifact suite definitions.

## Artifact suite naming

Logical suite name -> physical folder:

- `artifact_windows_cli` -> `tracer_windows_rust_cli/`
- `artifact_android` -> `tracer_android/`
- `artifact_log_generator` -> `log_generator/`

## Suite folders

- `tracer_windows_rust_cli/`: integrated suite for core + Windows CLI (`apps/cli/windows` build target).
- `tracer_android/`: host-side verification checks for `apps/android`.
- `log_generator/`: config for `apps/log_generator`.

Each suite contains:

- `config.toml` (entry, includes `env.toml` + `tests.toml`)
- `env.toml` (environment / deployment paths)
- `tests.toml` (command table)

Recommended layering inside each suite:

- aggregate entry:
  - `config.toml`
  - `tests.toml`
- capability / profile entry:
  - `config_cap_<capability>.toml`
  - `config_<profile>.toml`
- base / responsibility command files:
  - `tests/base*.toml`
  - `tests/commands_*.toml`

Naming rule:

- use `commands_*.toml` for responsibility-scoped command files
- do not introduce new `command_groups*.toml` filenames
- file names express responsibility, even when the TOML body uses `[[command_groups]]`

`tracer_android` also provides profile-specific entries:

- `config_android_style.toml`
- `config_android_ci.toml`
- `config_android_device.toml`

Optional launcher default:

- `paths.default_build_dir`: default source build folder when `--build-dir` is not passed.
  - Example: `build`, `build_fast`, `build_agent`, `build_check`

Path fields in suite TOML support:

- Relative paths (resolved from the TOML file directory)
- `${repo_root}` variable expansion

Schema validation is enforced before test execution starts:

- Required sections/fields are checked.
- Unresolved `${...}` variables are rejected.
- Command placeholders are validated against allowed runtime/matrix variables.

Manual lint command:

- `python test/suites/lint_suites.py`

Further organization guide:

- `docs/toolchain/test/suite_toml_organization.md`

