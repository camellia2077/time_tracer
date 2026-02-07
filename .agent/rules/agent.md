---
trigger: always_on
---

[ENVIRONMENT]

 -OS: Windows (Primary shell for Agent is MSYS2 UCRT64 BASH).
 -Toolchain: GCC/Clang (MinGW-w64), CMake, Ninja.
 -**CRITICAL**: The Agent MUST NOT run `.sh` or `bash` commands directly in PowerShell (pwsh). Avoid using the default Windows `bash.exe` (System32) which triggers WSL errors.

[WORKFLOW RULES]

 -TASK: Compile success required. SKIP runtime/testing.
 -PREP: Analyze context deeply. Clarify ambiguities BEFORE coding.
 -INTERACTION: Correct user errors directly. Do not follow flawed logic; provide constructive correction based on facts.
 -CODE_PHILOSOPHY: Keep simple & maintainable. AVOID over-defensive logic. Use physical separation for logic (keep files 100-300 lines) but avoid over-engineering.
 -**SHELL_RESTRICTION**:
 -All script executions (.sh) and build commands (CMake, Ninja) MUST be wrapped for UCRT64.
 -If the Agent MUST run commands via PowerShell, it MUST use the explicit path: `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c "your_command"`.


 -**PATH_LOGIC**:
 -Use POSIX-style paths (e.g., `/c/Computer/my_github/...`) for all shell-based operations.



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





