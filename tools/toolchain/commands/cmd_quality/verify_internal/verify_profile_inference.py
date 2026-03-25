from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import subprocess


_FOCUSED_PROFILE_ORDER = [
    "cap_pipeline",
    "cap_query",
    "cap_reporting",
    "cap_exchange",
    "cap_config",
    "cap_persistence_runtime",
    "cap_persistence_write",
    "shell_aggregate",
]

_OWNER_PROFILE_RULES: tuple[tuple[str, str], ...] = (
    ("apps/tracer_core_shell/api/c_api/capabilities/pipeline/", "cap_pipeline"),
    ("apps/tracer_core_shell/api/c_api/capabilities/query/", "cap_query"),
    ("apps/tracer_core_shell/api/c_api/capabilities/reporting/", "cap_reporting"),
    ("apps/tracer_core_shell/api/c_api/capabilities/exchange/", "cap_exchange"),
    ("apps/tracer_core_shell/api/c_api/capabilities/config/", "cap_config"),
    ("apps/tracer_core_shell/tests/integration/tracer_core_c_api_query_", "cap_query"),
    ("apps/tracer_core_shell/tests/integration/tracer_core_c_api_reporting_", "cap_reporting"),
    ("apps/tracer_core_shell/tests/integration/tracer_core_c_api_pipeline_", "cap_pipeline"),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/android_runtime_query_",
        "cap_query",
    ),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/android_runtime_reporting_",
        "cap_reporting",
    ),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/android_runtime_pipeline_",
        "cap_pipeline",
    ),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/android_runtime_config_",
        "cap_config",
    ),
    ("apps/tracer_core_shell/tests/platform/infrastructure/tests/data_query/", "cap_query"),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_",
        "cap_exchange",
    ),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/file_crypto/file_crypto_exchange_test_main.cpp",
        "cap_exchange",
    ),
    ("libs/tracer_core/src/application/pipeline/", "cap_pipeline"),
    ("libs/tracer_core/src/application/workflow", "cap_pipeline"),
    ("libs/tracer_core/src/application/use_cases/pipeline_api.cpp", "cap_pipeline"),
    (
        "libs/tracer_core/src/application/use_cases/tracer_core_api_pipeline.module.cpp",
        "cap_pipeline",
    ),
    ("libs/tracer_core/src/application/dto/pipeline_", "cap_pipeline"),
    ("libs/tracer_core/src/application/query/", "cap_query"),
    ("libs/tracer_core/src/application/use_cases/query_api.cpp", "cap_query"),
    (
        "libs/tracer_core/src/application/use_cases/tracer_core_api_query.module.cpp",
        "cap_query",
    ),
    ("libs/tracer_core/src/application/dto/query_", "cap_query"),
    ("libs/tracer_core/src/application/reporting/", "cap_reporting"),
    ("libs/tracer_core/src/application/use_cases/report_api.cpp", "cap_reporting"),
    ("libs/tracer_core/src/application/use_cases/report_api_support.cpp", "cap_reporting"),
    (
        "libs/tracer_core/src/application/use_cases/tracer_core_api_report.module.cpp",
        "cap_reporting",
    ),
    ("libs/tracer_core/src/application/dto/reporting_", "cap_reporting"),
    ("libs/tracer_core/src/application/dto/exchange_", "cap_exchange"),
    ("libs/tracer_core/src/application/use_cases/tracer_exchange_api.cpp", "cap_exchange"),
    (
        "libs/tracer_core/src/application/use_cases/tracer_core_api_exchange.module.cpp",
        "cap_exchange",
    ),
    ("libs/tracer_core/src/infra/query/", "cap_query"),
    ("libs/tracer_core/src/infra/reporting/", "cap_reporting"),
    ("libs/tracer_core/src/infra/exchange/", "cap_exchange"),
    ("libs/tracer_core/src/infra/config/", "cap_config"),
    (
        "libs/tracer_core/src/infra/persistence/sqlite_database_health_checker",
        "cap_persistence_runtime",
    ),
    ("libs/tracer_core/src/infra/persistence/importer/", "cap_persistence_write"),
    ("libs/tracer_core/src/infra/persistence/repositories/", "cap_persistence_write"),
    (
        "libs/tracer_core/src/infra/persistence/sqlite_time_sheet_repository",
        "cap_persistence_write",
    ),
    ("libs/tracer_core/tests/application/tests/application_pipeline_", "cap_pipeline"),
    ("libs/tracer_core/tests/application/tests/application_query_", "cap_query"),
    ("libs/tracer_core/tests/application/tests/application_workflow_", "cap_pipeline"),
    (
        "libs/tracer_core/tests/application/tests/application_aggregate_runtime_",
        "shell_aggregate",
    ),
    (
        "libs/tracer_core/tests/infra/tests/infrastructure_modules_smoke_query_",
        "cap_query",
    ),
    (
        "libs/tracer_core/tests/infra/tests/infrastructure_modules_smoke_reporting_",
        "cap_reporting",
    ),
    (
        "libs/tracer_core/tests/infra/tests/infrastructure_modules_smoke_exchange_",
        "cap_exchange",
    ),
    (
        "libs/tracer_core/tests/infra/tests/infrastructure_modules_smoke_config_",
        "cap_config",
    ),
    (
        "libs/tracer_core/tests/infra/tests/infrastructure_modules_smoke_persistence_runtime_",
        "cap_persistence_runtime",
    ),
    (
        "libs/tracer_core/tests/infra/tests/infrastructure_modules_smoke_persistence_write_",
        "cap_persistence_write",
    ),
    ("libs/tracer_core/tests/infra/tests/modules_smoke/query_", "cap_query"),
    ("libs/tracer_core/tests/infra/tests/modules_smoke/reports.cpp", "cap_reporting"),
    ("libs/tracer_core/tests/infra/tests/modules_smoke/crypto_exchange.cpp", "cap_exchange"),
    ("libs/tracer_core/tests/infra/tests/modules_smoke/logging_platform_config.cpp", "cap_config"),
    (
        "libs/tracer_core/tests/infra/tests/modules_smoke/persistence_runtime.cpp",
        "cap_persistence_runtime",
    ),
    (
        "libs/tracer_core/tests/infra/tests/modules_smoke/persistence_write.cpp",
        "cap_persistence_write",
    ),
    ("tools/toolchain/config/validate/tracer_core/pipeline.toml", "cap_pipeline"),
    ("tools/toolchain/config/validate/tracer_core/query.toml", "cap_query"),
    ("tools/toolchain/config/validate/tracer_core/reporting.toml", "cap_reporting"),
    ("tools/toolchain/config/validate/tracer_core/exchange.toml", "cap_exchange"),
    ("tools/toolchain/config/validate/tracer_core/config.toml", "cap_config"),
    (
        "tools/toolchain/config/validate/tracer_core/persistence_runtime.toml",
        "cap_persistence_runtime",
    ),
    (
        "tools/toolchain/config/validate/tracer_core/persistence_write.toml",
        "cap_persistence_write",
    ),
    ("test/suites/tracer_windows_rust_cli/config_cap_pipeline.toml", "cap_pipeline"),
    ("test/suites/tracer_windows_rust_cli/config_cap_query.toml", "cap_query"),
    ("test/suites/tracer_windows_rust_cli/config_cap_reporting.toml", "cap_reporting"),
    ("test/suites/tracer_windows_rust_cli/config_cap_exchange.toml", "cap_exchange"),
    ("test/suites/tracer_windows_rust_cli/config_cap_config.toml", "cap_config"),
    (
        "test/suites/tracer_windows_rust_cli/config_cap_persistence_runtime.toml",
        "cap_persistence_runtime",
    ),
    (
        "test/suites/tracer_windows_rust_cli/config_cap_persistence_write.toml",
        "cap_persistence_write",
    ),
    ("test/suites/tracer_windows_rust_cli/tests/commands_pipeline.toml", "cap_pipeline"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_query_data.toml", "cap_query"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_reporting.toml", "cap_reporting"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_exchange.toml", "cap_exchange"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_config.toml", "cap_config"),
    (
        "test/suites/tracer_windows_rust_cli/tests/commands_persistence_runtime.toml",
        "cap_persistence_runtime",
    ),
    (
        "test/suites/tracer_windows_rust_cli/tests/commands_persistence_write.toml",
        "cap_persistence_write",
    ),
    ("test/golden/report_formatter_parity/", "cap_reporting"),
)

_NON_OWNER_SHELL_PROFILE_RULES: tuple[tuple[str, str], ...] = (
    ("apps/tracer_core_shell/api/c_api/runtime/", "shell_aggregate"),
    ("apps/tracer_core_shell/api/c_api/tracer_core_c_api.cpp", "shell_aggregate"),
    ("apps/tracer_core_shell/api/c_api/tracer_core_c_api.h", "shell_aggregate"),
    ("apps/tracer_core_shell/api/android_jni/cmakelists.txt", "shell_aggregate"),
    ("apps/tracer_core_shell/api/c_api/cmakelists.txt", "shell_aggregate"),
    ("apps/tracer_core_shell/api/android_jni/", "shell_aggregate"),
    ("apps/tracer_core_shell/host/bootstrap/", "shell_aggregate"),
    ("apps/tracer_core_shell/host/exchange/", "shell_aggregate"),
    ("apps/tracer_core_shell/host/native_bridge_progress.cpp", "shell_aggregate"),
    ("apps/tracer_core_shell/tests/integration/tracer_core_c_api_runtime_", "shell_aggregate"),
    (
        "apps/tracer_core_shell/tests/integration/tracer_core_c_api_stability_internal.hpp",
        "shell_aggregate",
    ),
    ("apps/tracer_core_shell/tests/integration/tracer_core_c_api_error_tests.cpp", "shell_aggregate"),
    ("apps/tracer_core_shell/tests/integration/tracer_core_c_api_smoke_tests.cpp", "shell_aggregate"),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/android_runtime_shell_",
        "shell_aggregate",
    ),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/file_crypto/file_crypto_service_",
        "shell_aggregate",
    ),
    (
        "apps/tracer_core_shell/tests/platform/infrastructure/tests/file_crypto/file_crypto_test_main.cpp",
        "shell_aggregate",
    ),
    ("test/suites/tracer_windows_rust_cli/config_shell_aggregate.toml", "shell_aggregate"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_shell_aggregate.toml", "shell_aggregate"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_failure_modes.toml", "shell_aggregate"),
    ("test/suites/tracer_windows_rust_cli/tests/commands_tree_version.toml", "shell_aggregate"),
)

_SHARED_FALLBACK_PREFIXES: tuple[str, ...] = (
    "apps/tracer_core_shell/cmake/",
    ".github/workflows/",
    "tools/toolchain/",
)


@dataclass(frozen=True)
class VerifyProfileInference:
    changed_paths: tuple[str, ...]
    profiles: tuple[str, ...]
    fallback_to_fast: bool
    reason: str


def _normalize_path(path: str) -> str:
    return str(path).replace("\\", "/").strip().lower()


def classify_changed_paths(paths: list[str] | tuple[str, ...]) -> VerifyProfileInference:
    normalized_paths = tuple(
        normalized
        for normalized in (_normalize_path(path) for path in paths)
        if normalized
    )
    if not normalized_paths:
        return VerifyProfileInference(
            changed_paths=(),
            profiles=("fast",),
            fallback_to_fast=True,
            reason="no changed paths detected",
        )

    matched_profiles: list[str] = []
    shared_fallback_paths: list[str] = []
    unmatched_paths: list[str] = []
    for path in normalized_paths:
        if any(path.startswith(prefix) for prefix in _SHARED_FALLBACK_PREFIXES):
            shared_fallback_paths.append(path)
            continue

        matched_profile = None
        for prefix, profile in _OWNER_PROFILE_RULES:
            if path.startswith(prefix):
                matched_profile = profile
                break
        if matched_profile is None:
            for prefix, profile in _NON_OWNER_SHELL_PROFILE_RULES:
                if path.startswith(prefix):
                    matched_profile = profile
                    break
        if matched_profile is None:
            unmatched_paths.append(path)
            continue
        if matched_profile not in matched_profiles:
            matched_profiles.append(matched_profile)

    if shared_fallback_paths:
        sample = ", ".join(shared_fallback_paths[:3])
        return VerifyProfileInference(
            changed_paths=normalized_paths,
            profiles=("fast",),
            fallback_to_fast=True,
            reason=f"shared/build-system paths require profile `fast`: {sample}",
        )

    if unmatched_paths:
        sample = ", ".join(unmatched_paths[:3])
        return VerifyProfileInference(
            changed_paths=normalized_paths,
            profiles=("fast",),
            fallback_to_fast=True,
            reason=f"unmapped paths require profile `fast`: {sample}",
        )

    ordered_profiles = tuple(
        profile for profile in _FOCUSED_PROFILE_ORDER if profile in matched_profiles
    )
    if not ordered_profiles:
        return VerifyProfileInference(
            changed_paths=normalized_paths,
            profiles=("fast",),
            fallback_to_fast=True,
            reason="no focused capability profile matched changed paths",
        )
    return VerifyProfileInference(
        changed_paths=normalized_paths,
        profiles=ordered_profiles,
        fallback_to_fast=False,
        reason="all changed paths mapped to focused profiles",
    )


def _run_git(repo_root: Path, args: list[str]) -> list[str]:
    completed = subprocess.run(
        ["git", *args],
        cwd=repo_root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    if completed.returncode != 0:
        return []
    return [line for line in completed.stdout.splitlines() if line.strip()]


def list_worktree_changed_paths(repo_root: Path) -> list[str]:
    paths: list[str] = []
    seen: set[str] = set()
    for raw_line in _run_git(
        repo_root,
        ["status", "--porcelain=v1", "--untracked-files=all"],
    ):
        line = raw_line[3:] if len(raw_line) > 3 else raw_line
        destination = line.split(" -> ")[-1]
        normalized = _normalize_path(destination)
        if not normalized or normalized in seen:
            continue
        seen.add(normalized)
        paths.append(normalized)
    return paths


def list_branch_changed_paths(repo_root: Path, base_ref: str = "origin/main") -> list[str]:
    return [
        _normalize_path(path)
        for path in _run_git(repo_root, ["diff", "--name-only", f"{base_ref}...HEAD"])
        if _normalize_path(path)
    ]


def infer_verify_profiles(repo_root: Path) -> VerifyProfileInference:
    worktree_paths = list_worktree_changed_paths(repo_root)
    if worktree_paths:
        return classify_changed_paths(worktree_paths)

    branch_paths = list_branch_changed_paths(repo_root)
    if branch_paths:
        return classify_changed_paths(branch_paths)

    return VerifyProfileInference(
        changed_paths=(),
        profiles=("fast",),
        fallback_to_fast=True,
        reason="git worktree is clean and no branch diff versus origin/main was found",
    )
