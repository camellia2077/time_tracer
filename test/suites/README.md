# suites

Canonical table-driven suite definitions.

- `tracer_windows_cli/`: integrated suite for core + Windows CLI (`apps/tracer_windows_cli` build target).
- `tracer_android/`: host-side verification checks for `apps/tracer_android`.
- `log_generator/`: config for `apps/log_generator`.

Each suite contains:

- `config.toml` (entry, includes `env.toml` + `tests.toml`)
- `env.toml` (environment / deployment paths)
- `tests.toml` (command table)

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
