---
trigger: always_on
---

[WORKFLOW RULES]
TASK: Compile success required. SKIP runtime/testing.
PREP: Analyze context deeply. Clarify ambiguities BEFORE coding.
INTERACTION: Correct user errors directly.
CODE_PHILOSOPHY: Keep simple & maintainable. AVOID over-defensive logic.

[C++23 STYLE CONFIG]
NAMING: 
- Types/Funcs=PascalCase; 
- Vars/Params: snake_case (MIN_LEN: 3, except i/j/k in loops)
- Members=snake_case_(suffix _); 
- Consts: kPascalCase (NO MAGIC NUMBERS: all literals > 1 must be named)
- Macros=AVOID.

IDIOMS:
- Inputs: std::string_view(str), std::span(vec).
- Outputs: std::expected(error), std::optional(maybe), T&(mutable).
- IO: std::println.
- Compile: consteval(strict).
SAFETY:
- Types: NO IMPLICIT CONVERSIONS (use static_cast or std::bit_cast).
- Logic: Explicit null checks (ptr != nullptr).
- Memory: No new/delete/C-casts.
- Pointers: unique_ptr(own), T*(view only).
- Defaults: nullptr, [[nodiscard]], explicit, override, in-class-init.
STRUCTURE:
- Headers absolute path from src/.