from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class RuntimeGuardScenario:
    name: str
    description: str
    mutate: Callable[[Path], None]
    expect_success: bool
    expected_exits: tuple[int, ...]
    expected_tokens: list[str]
    unexpected_tokens: list[str]


def mutate_nop(_: Path) -> None:
    return


def remove_required(path: Path) -> None:
    if not path.exists():
        raise RuntimeError(f"Cannot mutate runtime guard case; file not found: {path}")
    path.unlink()


def build_scenarios() -> list[RuntimeGuardScenario]:
    return [
        RuntimeGuardScenario(
            name="baseline_ok",
            description="完整 runtime bundle 可正常通过 bootstrap guard。",
            mutate=mutate_nop,
            expect_success=True,
            expected_exits=(0,),
            expected_tokens=[],
            unexpected_tokens=[
                "runtime check failed",
                "configuration validation failed",
                "startup error",
            ],
        ),
        RuntimeGuardScenario(
            name="missing_core_dll",
            description="缺失 core dll 时，CLI 本地最小检查应 fail-fast。",
            mutate=lambda bin_dir: remove_required(bin_dir / "tracer_core.dll"),
            expect_success=False,
            expected_exits=(10,),
            expected_tokens=["tracer_core.dll"],
            unexpected_tokens=[
                "configuration validation failed",
                "startup error",
            ],
        ),
        RuntimeGuardScenario(
            name="missing_config_toml",
            description="缺失 config.toml 时，core runtime-check 应返回缺文件错误。",
            mutate=lambda bin_dir: remove_required(bin_dir / "config" / "config.toml"),
            expect_success=False,
            expected_exits=(5, 7),
            expected_tokens=["configuration file not found", "config.toml"],
            unexpected_tokens=[
                "configuration validation failed",
                "startup error",
            ],
        ),
        RuntimeGuardScenario(
            name="missing_reports_shared_dll",
            description="缺失 core 依赖 dll 时，core 动态加载应 fail-fast。",
            mutate=lambda bin_dir: remove_required(bin_dir / "reports_shared.dll"),
            expect_success=False,
            expected_exits=(10, 3221225781),
            expected_tokens=[],
            unexpected_tokens=[
                "configuration validation failed",
                "startup error",
            ],
        ),
    ]
