# Exit Code Policy

`log_generator` uses unified, explicit process exit codes defined in `src/common/exit_code.hpp`.

## Exit Codes

- `0` (`kSuccess`): completed successfully.
- `2` (`kCliError`): invalid CLI arguments or CLI parse/usage error.
- `3` (`kMissingRequestConfig`): CLI action is run mode but config payload is missing.
- `4` (`kRuntimeConfigLoadFailed`): runtime config files cannot be loaded/parsed.
- `5` (`kGenerationFailed`): generation workflow failed.
- `10` (`kInternalError`): uncaught internal exception.

## Mapping Rules

- `main()` must convert `ExitCode` to process status via `to_status_code(...)`.
- `Application::run(...)` returns domain/application-level status (`kSuccess`, `kRuntimeConfigLoadFailed`, `kGenerationFailed`).
- CLI-level errors (`CliAction::kError`) are handled in `main()` and return `kCliError`.
- Any uncaught exception in `main()` must return `kInternalError`.

## Notes for Tests

- Prefer asserting exact status code values instead of only checking non-zero.
- Keep this file and `src/common/exit_code.hpp` synchronized when adding new failure modes.
