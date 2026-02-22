from pathlib import Path

from ...core.config import BuildProfileConfig
from ...core.context import Context
from ...core.executor import kill_build_processes, run_command
from . import cmake as build_cmake, command_entries as build_command_entries, common as build_common


class BuildCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    # --- Config sync + path helpers ---
    def _resolve_platform_config_source_root(self) -> Path:
        return build_common.resolve_platform_config_source_root(self.ctx)

    def _resolve_platform_config_output_root(self, sync_target: str) -> Path:
        return build_common.resolve_platform_config_output_root(self.ctx, sync_target)

    def _resolve_platform_config_runner_path(self) -> Path | None:
        return build_common.resolve_platform_config_runner_path(self.ctx)

    def _resolve_config_sync_target(self, app_name: str) -> str | None:
        return build_common.resolve_config_sync_target(self.ctx, app_name)

    def _sync_platform_config_if_needed(self, app_name: str) -> int:
        return build_common.sync_platform_config_if_needed(
            self.ctx,
            app_name,
            run_command_fn=run_command,
        )

    # --- Windows/Android override helpers ---
    def _resolve_windows_config_cmake_args(self, app_name: str) -> list[str]:
        return build_common.resolve_windows_config_cmake_args(self.ctx, app_name)

    @staticmethod
    def _extract_cmake_definition_values(args: list[str], key: str) -> list[str]:
        return build_common.extract_cmake_definition_values(args, key)

    @staticmethod
    def _strip_cmake_definition(args: list[str], key: str) -> list[str]:
        return build_common.strip_cmake_definition(args, key)

    def _validate_windows_config_source_override(
        self,
        app_name: str,
        configure_args: list[str],
    ) -> tuple[bool, str]:
        return build_common.validate_windows_config_source_override(
            self.ctx,
            app_name,
            configure_args,
        )

    def _sync_windows_runtime_config_copy_if_needed(
        self,
        app_name: str,
        build_dir_name: str,
    ) -> int:
        return build_common.sync_windows_runtime_config_copy_if_needed(
            self.ctx,
            app_name,
            build_dir_name,
        )

    @staticmethod
    def _resolve_android_config_gradle_args(extra_args: list[str]) -> bool:
        return build_common.resolve_android_config_gradle_args(extra_args)

    @staticmethod
    def _extract_gradle_property_values(args: list[str], key: str) -> list[str]:
        return build_common.extract_gradle_property_values(args, key)

    def _validate_android_gradle_property_overrides(
        self,
        app_name: str,
        gradle_args: list[str],
    ) -> tuple[bool, str]:
        return build_common.validate_android_gradle_property_overrides(
            self.ctx,
            app_name,
            gradle_args,
        )

    def _resolve_android_config_gradle_property(self, app_name: str) -> list[str]:
        return build_common.resolve_android_config_gradle_property(self.ctx, app_name)

    # --- Backend/profile/build-dir helpers ---
    def _resolve_backend(self, app_name: str) -> str:
        return build_common.resolve_backend(self.ctx, app_name)

    @staticmethod
    def _resolve_build_dir_name(
        tidy: bool,
        build_dir_name: str | None,
        profile_build_dir: str | None = None,
    ) -> str:
        return build_common.resolve_build_dir_name(tidy, build_dir_name, profile_build_dir)

    def _resolve_profile(
        self,
        profile_name: str | None,
    ) -> tuple[str | None, BuildProfileConfig | None]:
        return build_common.resolve_profile(self.ctx, profile_name)

    def resolve_build_dir_name(
        self,
        tidy: bool,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        app_name: str | None = None,
    ) -> str:
        backend = ""
        if app_name:
            backend = self._resolve_backend(app_name)

        if backend == "gradle":
            # Android/Gradle build outputs are rooted under app/build.
            return "build"

        _, profile_cfg = self._resolve_profile(profile_name)
        profile_build_dir = ""
        if profile_cfg is not None:
            profile_build_dir = (getattr(profile_cfg, "build_dir", "") or "").strip()
        return self._resolve_build_dir_name(tidy, build_dir_name, profile_build_dir)

    def _profile_cmake_args(self, profile_name: str | None) -> list[str]:
        return build_common.profile_cmake_args(self.ctx, profile_name)

    def _profile_gradle_tasks(self, profile_name: str | None) -> list[str]:
        return build_common.profile_gradle_tasks(self.ctx, profile_name)

    def _resolve_gradle_tasks(self, app_name: str, profile_name: str | None) -> list[str]:
        return build_common.resolve_gradle_tasks(self.ctx, app_name, profile_name)

    def _resolve_gradle_wrapper(self, app_name: str) -> str:
        return build_common.resolve_gradle_wrapper(self.ctx, app_name)

    # --- CMake cache/toolchain helpers ---
    def _is_configured(
        self,
        app_name: str,
        tidy: bool,
        build_dir_name: str | None,
        profile_name: str | None,
    ) -> bool:
        return build_cmake.is_configured(
            ctx=self.ctx,
            app_name=app_name,
            tidy=tidy,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            resolve_backend_fn=self._resolve_backend,
            resolve_build_dir_name_fn=self.resolve_build_dir_name,
        )

    @staticmethod
    def _normalize_cache_path(path_value: str) -> str:
        return build_common.normalize_cache_path(path_value)

    def _needs_windows_config_reconfigure(self, app_name: str, build_dir: Path) -> bool:
        return build_cmake.needs_windows_config_reconfigure(
            ctx=self.ctx,
            app_name=app_name,
            build_dir=build_dir,
        )

    @staticmethod
    def _has_cmake_definition(args: list[str], key: str) -> bool:
        return build_common.has_cmake_definition(args, key)

    def _resolve_toolchain_flags(self, user_args: list[str]) -> list[str]:
        return build_common.resolve_toolchain_flags(self.ctx, user_args)

    # --- Public command entrypoints ---
    def configure(
        self,
        app_name: str,
        tidy: bool,
        extra_args: list[str] | None = None,
        cmake_args: list[str] | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        kill_build_procs: bool = False,
        sync_platform_config: bool = True,
    ) -> int:
        return build_command_entries.configure_entry(
            command=self,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
            sync_platform_config=sync_platform_config,
            run_command_fn=run_command,
            kill_build_processes_fn=kill_build_processes,
        )

    def build(
        self,
        app_name: str,
        tidy: bool,
        extra_args: list[str] | None = None,
        cmake_args: list[str] | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        kill_build_procs: bool = False,
    ) -> int:
        return build_command_entries.build_entry(
            command=self,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
            run_command_fn=run_command,
            kill_build_processes_fn=kill_build_processes,
        )

    def execute(
        self,
        app_name: str,
        tidy: bool,
        extra_args: list[str] | None = None,
        cmake_args: list[str] | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        kill_build_procs: bool = False,
    ) -> int:
        return build_command_entries.execute_entry(
            command=self,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
            kill_build_processes_fn=kill_build_processes,
        )
