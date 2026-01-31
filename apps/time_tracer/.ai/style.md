[C++23 STYLE CONFIG]

NAMING: Types/Funcs=PascalCase; Vars/Params=snake_case; Members=snake_case_(suffix _); Consts=kPascalCase; Macros=AVOID.

IDIOMS:

- Inputs: std::string_view(str), std::span(vec).

- Outputs: std::expected(error), std::optional(maybe), T&(mutable).

- IO: std::println.

- Compile: consteval(strict).

SAFETY:

- No new/delete/C-casts.

- Pointers: unique_ptr(own), T*(view only).

- Defaults: nullptr, [[nodiscard]], explicit, override, in-class-init.

STRUCTURE: Headers absolute path from src/.