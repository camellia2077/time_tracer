---
trigger: always_on
---

[ENVIRONMENT]

 -OS: Windows (Primary shell for Agent is MSYS2 UCRT64 BASH).
 -Toolchain: GCC/Clang (MinGW-w64), CMake, Ninja.
 -Git repo root: `time_tracer_cpp` (run git commands from this directory).
 -**CRITICAL**: The Agent MUST NOT run `.sh` or `bash` commands directly in PowerShell (pwsh). Avoid using the default Windows `bash.exe` (System32) which triggers WSL errors.

[WORKFLOW RULES]

 -TASK: Prefer compile success. SKIP runtime/testing unless requested.
 -PREP: Analyze context and clarify ambiguities when they affect correctness.
 -INTERACTION: Correct user errors directly and propose a fix.
 -CODE_PHILOSOPHY: Keep simple & maintainable. Avoid over-defensive logic. Use physical separation for logic (keep files 100-300 lines) but avoid over-engineering.
 -**SHELL_RESTRICTION (Flexible)**:
 -Prefer running script/build commands in UCRT64.
 -If UCRT64 fails (e.g., MSYS2 errors), ask the user and fall back to PowerShell-native commands or Python scripts.
 -Only run `.sh` via a shell when needed; otherwise prefer Python entrypoints.
 -Recommended UCRT64 wrapper: `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c "your_command"`.


 -**PATH_LOGIC**:
 -Prefer POSIX-style paths for UCRT64 shell commands. Use native Windows paths for PowerShell commands.



[C++23 STYLE CONFIG]
NAMING:
- Types/Funcs=PascalCase; (MANDATORY: NEVER use camelCase or snake_case for functions. e.g., 'Validate', NOT 'validate' or 'validate_all').
- Vars/Params: snake_case (MIN_LEN: 3, except i/j/k in loops).
- Members=snake_case_ (MUST end with underscore, e.g., 'is_valid_').
- Consts: kPascalCase (MUST start with 'k', e.g., 'kMaxRetry').
- Macros=AVOID.

IDIOMS:

- Inputs: std::string_view(str), std::span(vec).
 -Outputs: std::expected(error), std::optional(maybe), T&(mutable).
 -IO: std::println.
 -Compile: consteval(strict).

SAFETY:

 -Types: NO IMPLICIT CONVERSIONS (use static_cast or std::bit_cast).
 -Logic: Explicit null checks (ptr != nullptr).
 -Memory: No new/delete/C-casts.
 -Pointers: unique_ptr(own), T*(view only).
 -Defaults: nullptr, [[nodiscard]], explicit, override, in-class-init.

STRUCTURE:

 -Headers: Use absolute path from src/.



