# Exit Code Policy

All CLI commands must use unified exit codes:

- Success: process exit code `0`.
- Failure: process exit code non-zero (use `1` for general failure).

Implementation rule:

- New commands must signal failures (e.g., invalid arguments, missing data,
  runtime errors) by throwing `std::runtime_error` (or a derived exception).
- Do not swallow errors inside command handlers unless you rethrow or otherwise
  cause a non-zero exit code.
- `CliApplication::execute()` returns the command status code and `main()` must
  return that value to the OS.

If you add a new command, ensure it follows this policy so automated tests can
reliably detect failures.
