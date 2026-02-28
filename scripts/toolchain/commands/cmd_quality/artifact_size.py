from pathlib import Path

from ...core.context import Context
from ...services.artifact_size import collect_artifact_sizes, write_artifact_size_json
from ..cmd_build import BuildCommand


class ArtifactSizeCommand:
    _DEFAULT_BUILD_DIR = "build_artifact_size"
    _DEFAULT_PROFILE = "release_safe"
    _DEFAULT_RESULT_JSON_NAME = "artifact_size.json"
    _DEFAULT_ARTIFACT_GLOB = "time_tracer_cli.exe"
    _DEFAULT_TRACKED_PE_SECTIONS = [".text", ".rdata"]

    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        targets: list[str],
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        result_json: str | None = None,
        artifact_glob: str | None = None,
        exclude_substrings: list[str] | None = None,
        cmake_args: list[str] | None = None,
    ) -> int:
        if not targets:
            print("Error: artifact-size requires at least one build target.")
            return 2

        app_meta = self.ctx.get_app_metadata(app_name)
        backend = (getattr(app_meta, "backend", "cmake") or "cmake").strip().lower()
        if backend != "cmake":
            print(f"Error: artifact-size only supports CMake apps, got backend `{backend}`.")
            return 2

        effective_build_dir_name = (build_dir_name or "").strip() or self._DEFAULT_BUILD_DIR
        effective_profile_name = (profile_name or "").strip() or self._DEFAULT_PROFILE
        merged_cmake_args = [arg for arg in (cmake_args or []) if arg != "--"]

        print(
            "--- artifact-size: build targets "
            f"(app={app_name}, build_dir={effective_build_dir_name}, "
            f"profile={effective_profile_name}, "
            f"targets={','.join(targets)})."
        )

        build_ret = BuildCommand(self.ctx).build(
            app_name=app_name,
            tidy=False,
            extra_args=["--target", *targets],
            cmake_args=merged_cmake_args,
            build_dir_name=effective_build_dir_name,
            profile_name=effective_profile_name,
            kill_build_procs=False,
        )
        if build_ret != 0:
            return build_ret

        bin_dir = self.ctx.get_app_dir(app_name) / effective_build_dir_name / "bin"
        try:
            artifacts, total_bytes = collect_artifact_sizes(
                bin_dir=bin_dir,
                artifact_glob=artifact_glob or self._DEFAULT_ARTIFACT_GLOB,
                exclude_substrings=exclude_substrings,
                tracked_pe_sections=self._DEFAULT_TRACKED_PE_SECTIONS,
            )
        except FileNotFoundError as exc:
            print(f"Error: {exc}")
            return 1

        if not artifacts:
            effective_glob = artifact_glob or self._DEFAULT_ARTIFACT_GLOB
            print(f"Error: no artifacts matched under {bin_dir} (glob={effective_glob}).")
            return 1

        if result_json and result_json.strip():
            result_path = Path(result_json.strip())
            if not result_path.is_absolute():
                result_path = self.ctx.repo_root / result_path
        else:
            result_path = (
                self.ctx.get_app_dir(app_name)
                / effective_build_dir_name
                / self._DEFAULT_RESULT_JSON_NAME
            )

        write_artifact_size_json(
            output_path=result_path,
            scope="artifacts",
            bin_dir=bin_dir,
            artifacts=artifacts,
            total_bytes=total_bytes,
        )
        print(f"--- artifact-size: JSON written: {result_path}")
        return 0
