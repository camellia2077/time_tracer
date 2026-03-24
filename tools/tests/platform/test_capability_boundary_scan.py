from pathlib import Path
from unittest import TestCase


class TestCapabilityBoundaryScan(TestCase):
    _LEGACY_INCLUDE_TOKENS = (
        '#include "application/dto/core_requests.hpp"',
        '#include "application/dto/core_responses.hpp"',
        '#include "application/dto/tree_query_response.hpp"',
        '#include "application/interfaces/i_report_handler.hpp"',
        '#include "application/interfaces/i_report_exporter.hpp"',
        '#include "application/interfaces/i_report_query_service.hpp"',
        '#include "application/use_cases/i_tracer_core_runtime.hpp"',
        '#include "application/use_cases/tracer_core_runtime.hpp"',
    )

    def test_active_sources_do_not_include_legacy_boundary_wrappers(self):
        repo_root = Path(__file__).resolve().parents[3]
        scan_roots = [
            repo_root / "apps" / "tracer_core_shell",
            repo_root / "libs" / "tracer_core",
        ]

        offenders: list[str] = []
        for root in scan_roots:
            for path in root.rglob("*"):
                if path.suffix not in {".hpp", ".cpp", ".cppm", ".inc"}:
                    continue
                text = path.read_text(encoding="utf-8", errors="replace")
                for token in self._LEGACY_INCLUDE_TOKENS:
                    if token in text:
                        offenders.append(f"{path.relative_to(repo_root)}::{token}")

        self.assertEqual(offenders, [])

    def test_legacy_shim_files_are_removed(self):
        repo_root = Path(__file__).resolve().parents[3]
        removed_shims = [
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "dto"
            / "core_requests.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "dto"
            / "core_responses.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "dto"
            / "tree_query_response.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "interfaces"
            / "i_report_handler.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "interfaces"
            / "i_report_exporter.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "interfaces"
            / "i_report_query_service.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "use_cases"
            / "i_tracer_core_runtime.hpp",
            repo_root
            / "libs"
            / "tracer_core"
            / "src"
            / "application"
            / "use_cases"
            / "tracer_core_runtime.hpp",
        ]

        for path in removed_shims:
            self.assertFalse(path.exists(), msg=str(path))

    def test_non_reporting_capability_roots_do_not_include_compat_or_aggregate(self):
        repo_root = Path(__file__).resolve().parents[3]
        scan_targets = [
            repo_root / "libs" / "tracer_core" / "src" / "application" / "pipeline",
            repo_root / "libs" / "tracer_core" / "src" / "application" / "query",
            repo_root / "libs" / "tracer_core" / "src" / "infra" / "query",
            repo_root / "libs" / "tracer_core" / "src" / "infra" / "config",
            repo_root / "libs" / "tracer_core" / "src" / "infra" / "persistence",
            repo_root / "libs" / "tracer_core" / "src" / "infra" / "exchange",
            repo_root / "libs" / "tracer_core" / "src" / "infra" / "crypto",
            repo_root / "libs" / "tracer_core" / "src" / "application" / "use_cases" / "pipeline_api.cpp",
            repo_root / "libs" / "tracer_core" / "src" / "application" / "use_cases" / "query_api.cpp",
            repo_root / "libs" / "tracer_core" / "src" / "application" / "use_cases" / "tracer_exchange_api.cpp",
        ]
        forbidden_tokens = (
            '#include "application/dto/compat/',
            '#include "application/compat/',
            '#include "application/aggregate_runtime/',
        )

        offenders: list[str] = []
        for target in scan_targets:
            candidates = [target]
            if target.is_dir():
                candidates = [
                    path
                    for path in target.rglob("*")
                    if path.suffix in {".hpp", ".cpp", ".cppm", ".inc"}
                ]
            for path in candidates:
                text = path.read_text(encoding="utf-8", errors="replace")
                for token in forbidden_tokens:
                    if token in text:
                        offenders.append(f"{path.relative_to(repo_root)}::{token}")

        self.assertEqual(offenders, [])
