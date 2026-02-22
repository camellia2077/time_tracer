from ....core.context import Context


def has_cmake_definition(args: list[str], key: str) -> bool:
    prefixed = f"-D{key}="
    plain = f"{key}="
    token = f"{key}="
    for index, arg in enumerate(args):
        if arg.startswith(prefixed) or arg.startswith(plain):
            return True
        if arg == "-D" and index + 1 < len(args) and args[index + 1].startswith(token):
            return True
    return False


def resolve_toolchain_flags(ctx: Context, user_args: list[str]) -> list[str]:
    existing = list(user_args)
    has_c_compiler = has_cmake_definition(existing, "CMAKE_C_COMPILER")
    has_cxx_compiler = has_cmake_definition(existing, "CMAKE_CXX_COMPILER")
    if has_c_compiler and has_cxx_compiler:
        return []

    build_cfg = ctx.config.build
    c_compiler = build_cfg.c_compiler
    cxx_compiler = build_cfg.cxx_compiler

    compiler_key = (build_cfg.compiler or "default").strip().lower()
    if not c_compiler and not cxx_compiler:
        if compiler_key == "clang":
            cxx_compiler = "clang++"
        elif compiler_key == "gcc":
            cxx_compiler = "g++"

    flags: list[str] = []
    if c_compiler and not has_c_compiler:
        flags += ["-D", f"CMAKE_C_COMPILER={c_compiler}"]
    if cxx_compiler and not has_cxx_compiler:
        flags += ["-D", f"CMAKE_CXX_COMPILER={cxx_compiler}"]
    return flags
