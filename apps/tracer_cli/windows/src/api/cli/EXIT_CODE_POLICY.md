# Exit Code Policy

Windows CLI uses typed exit codes (`AppExitCode`) instead of a single generic
`1`, so scripts can identify failure class reliably.

## Exit Code Map

- `0`: `kSuccess`
- `1`: `kGenericError`
- `2`: `kCommandNotFound`
- `3`: `kInvalidArguments`
- `4`: `kDatabaseError`
- `5`: `kIoError`
- `6`: `kLogicError`
- `7`: `kConfigError`
- `8`: `kMemoryError`
- `9`: `kUnknownError`
- `10`: `kDllCompatibilityError`

## Bootstrap Mapping

`AppRunner::Run` maps runtime validation failures to typed exit codes:

- missing runtime dependency (e.g. `tracer_core.dll`): `10`
- configuration missing/invalid (e.g. `config/config.toml`): `7`
- runtime I/O failures: `5`
- invalid bootstrap argument failures: `3`
- other validation failures: `1`

## Command Implementation Rule

- New commands should throw typed exceptions (`DatabaseError`, `IoError`,
  `ConfigError`, `DllCompatibilityError`, `LogicError`) where possible.
- `std::invalid_argument` maps to `3`, other `std::exception` maps to `1`.
- Do not swallow failures in command handlers unless you return an equivalent
  non-zero status.
- `CliApplication::Execute()` is the single mapping point for command execution
  exceptions.
