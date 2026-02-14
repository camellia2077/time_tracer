from ..core.context import Context
from ..core.executor import kill_build_processes, run_command

class BuildCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    @staticmethod
    def _resolve_build_dir_name(tidy: bool, build_dir_name: str | None) -> str:
        if build_dir_name:
            return build_dir_name
        return "build_tidy" if tidy else "build_fast"

    @staticmethod
    def _has_cmake_definition(args: list[str], key: str) -> bool:
        prefixed = f"-D{key}="
        plain = f"{key}="
        token = f"{key}="
        for index, arg in enumerate(args):
            if arg.startswith(prefixed) or arg.startswith(plain):
                return True
            if arg == "-D" and index + 1 < len(args) and args[index + 1].startswith(token):
                return True
        return False

    def _resolve_toolchain_flags(self, user_args: list[str]) -> list[str]:
        existing = list(user_args)
        has_c_compiler = self._has_cmake_definition(existing, "CMAKE_C_COMPILER")
        has_cxx_compiler = self._has_cmake_definition(existing, "CMAKE_CXX_COMPILER")
        if has_c_compiler and has_cxx_compiler:
            return []

        build_cfg = self.ctx.config.build
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

    def configure(
        self,
        app_name: str,
        tidy: bool,
        extra_args: list = None,
        build_dir_name: str | None = None,
        kill_build_procs: bool = False,
    ):
        if kill_build_procs:
            kill_build_processes()

        app = self.ctx.get_app_metadata(app_name)
        resolved_build_dir_name = self._resolve_build_dir_name(tidy, build_dir_name)
        build_dir = self.ctx.get_app_dir(app_name) / resolved_build_dir_name
        source_dir = self.ctx.get_app_dir(app_name)

        
        build_dir.mkdir(parents=True, exist_ok=True)
        
        flags = app.cmake_flags[:]
        if tidy:
            flags.extend(["-D", "ENABLE_CLANG_TIDY=ON", "-D", "ENABLE_PCH=OFF"])
        else:
            flags.extend(["-D", "ENABLE_CLANG_TIDY=OFF", "-D", "ENABLE_PCH=ON"])
        
        filtered_args = [a for a in (extra_args or []) if a != "--"]
        toolchain_flags = self._resolve_toolchain_flags(flags + filtered_args)
        config_cmd = ["cmake", "-S", str(source_dir), "-B", str(build_dir)] + \
            flags + toolchain_flags + filtered_args
        return run_command(config_cmd, env=self.ctx.setup_env())

    def build(
        self,
        app_name: str,
        tidy: bool,
        extra_args: list = None,
        build_dir_name: str | None = None,
        kill_build_procs: bool = False,
    ):
        if kill_build_procs:
            kill_build_processes()

        resolved_build_dir_name = self._resolve_build_dir_name(tidy, build_dir_name)
        build_dir = self.ctx.get_app_dir(app_name) / resolved_build_dir_name
        filtered_args = [a for a in (extra_args or []) if a != "--"]
        build_cmd = ["cmake", "--build", str(build_dir), "-j"] + filtered_args
        return run_command(build_cmd, env=self.ctx.setup_env())

    def execute(
        self,
        app_name: str,
        tidy: bool,
        extra_args: list = None,
        build_dir_name: str | None = None,
        kill_build_procs: bool = False,
    ):
        if kill_build_procs:
            kill_build_processes()

        ret = self.configure(
            app_name,
            tidy,
            extra_args,
            build_dir_name=build_dir_name,
            kill_build_procs=False,
        )
        if ret != 0: return ret
        return self.build(
            app_name,
            tidy,
            extra_args,
            build_dir_name=build_dir_name,
            kill_build_procs=False,
        )
