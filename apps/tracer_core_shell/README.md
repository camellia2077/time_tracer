# tracer_core_shell

Internal shell host for `tracer_core` app identity migration.

Current phase:
- Preferred semantic app id: `tracer_core_shell`.
- Keep external compatibility app id as `tracer_core`.
- Shell source root now uses top-level folders:
  - `api/`
  - `host/`
  - `tests/`
- CMake configure root now also lives directly at `apps/tracer_core_shell`.
