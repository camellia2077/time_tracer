---
trigger: always_on
---

[ENVIRONMENT]

- OS: Windows (primary shell: PowerShell).
- Toolchain: GCC/Clang (MinGW-w64), CMake, Ninja.
- Repo root: `time_tracer_cpp` (run git commands here).
- PATH hint (PowerShell): prepend `C:\msys64\ucrt64\bin;C:\msys64\usr\bin` before `cmake`/`ninja`.

[WORKFLOW RULES]

- HARD_GATES (MUST): requested behavior must be correct; target app must configure/build after changes.
- TEST_POLICY: run tests when workflow/user requires, or when needed to validate behavior.
- STYLE_SCOPE (MUST_FOR_NEW_OR_TOUCHED_CODE): enforce naming/style/format only on new or modified code.
- LEGACY_POLICY: avoid large style-only cleanup on untouched legacy code unless requested.
- ON_UNCERTAIN: make minimal safe changes, get build green, then iterate.
- PREP: clarify ambiguities that affect correctness.
- INTERACTION: directly correct user mistakes and propose fixes.
- CODE_PHILOSOPHY: prefer simple, maintainable solutions; avoid over-engineering; keep files ~100-300 lines when practical.
- MANDATORY_BUILD_SYSTEM: use `python scripts/run.py configure/build --app <app>`.
- MANDATORY_BUILD_DIR: always use `build_fast`.
- AGENT_CONFIGURE_COMMAND: time_tracer=`python scripts/run.py configure --app time_tracer`; log_generator=`python scripts/run.py configure --app log_generator`.
- AGENT_BUILD_COMMAND: time_tracer=`python scripts/run.py build --app time_tracer`; log_generator=`python scripts/run.py build --app log_generator`.
- AGENT_TEST_COMMAND: time_tracer=`python test/run.py --suite time_tracer --agent --build-dir build_fast --concise`; log_generator=`python test/run.py --suite log_generator --agent --build-dir build_fast --concise`.
- TEST_POST_FORMAT_NOTE: `python test/run.py --suite <app> --agent ...` may auto-run `clang-format` after tests; formatting-only diffs from this step are expected and can be ignored unless explicitly requested.
- AFTER_CODE_CHANGE: run corresponding app configure/build.
- STYLE_AUTOMATION: prefer `clang-tidy`, `clang-format`, and project Python scripts for repeatable cleanup.
- SHELL_RESTRICTION:
  - default build shell is PowerShell with `scripts/run.py`.
  - avoid `msys2_shell.cmd` unless user explicitly asks.
  - do not use ad-hoc compile wrappers outside `scripts/run.py`.
  - run `.sh` only when user explicitly asks shell-script flow.
- PATH_LOGIC: use POSIX paths in UCRT64 shell commands; use native Windows paths in PowerShell.



[C++23 STYLE CONFIG]
- TOOL_ENFORCEMENT: Naming/safety/style checks are enforced by `.clang-tidy`; formatting is enforced by `.clang-format`.

IDIOMS:

- Inputs: prefer `std::string_view` / `std::span`.
- Outputs: prefer `std::expected`, `std::optional`, `T&` (mutable out).
- I/O: prefer `std::println`.
- Compile-time: use `consteval` where strict compile-time evaluation is required.

SAFETY:

- Ownership: `std::unique_ptr`; raw `T*` is non-owning view only.

STRUCTURE:

- Headers: use absolute include path from `src/`.



