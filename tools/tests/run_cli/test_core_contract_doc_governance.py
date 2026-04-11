from __future__ import annotations

import subprocess
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]

CORE_CONTRACT_DOCS = {
    "docs/time_tracer/core/contracts/c_abi.md",
    "docs/time_tracer/core/contracts/error-model.md",
    "docs/time_tracer/core/contracts/error-codes.md",
    "docs/time_tracer/core/contracts/cli_surface_contract_v1.md",
}

TEST_RELATED_PREFIXES = (
    "tools/test_framework/",
    "tools/suites/",
    "tools/tests/",
)


def _run_git_and_collect_files(args: list[str]) -> set[str]:
    completed = subprocess.run(
        ["git", *args],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"git {' '.join(args)} failed with exit={completed.returncode}: {completed.stderr.strip()}"
        )

    files: set[str] = set()
    for line in completed.stdout.splitlines():
        path = line.strip().replace("\\", "/")
        if path:
            files.add(path)
    return files


def _collect_changed_files() -> set[str]:
    changed = set()
    changed |= _run_git_and_collect_files(["diff", "--name-only"])
    changed |= _run_git_and_collect_files(["diff", "--name-only", "--cached"])
    changed |= _run_git_and_collect_files(["ls-files", "--others", "--exclude-standard"])

    # 在 clean workspace 的 CI 场景下，以上集合通常为空，回退到 HEAD 变更集。
    if not changed:
        changed |= _run_git_and_collect_files(["show", "--name-only", "--pretty=format:", "HEAD"])
    return changed


class TestCoreContractDocGovernance(TestCase):
    def test_core_contract_doc_changes_must_include_test_changes(self):
        changed_files = _collect_changed_files()
        changed_core_docs = sorted(path for path in changed_files if path in CORE_CONTRACT_DOCS)

        if not changed_core_docs:
            return

        has_test_change = any(
            any(path.startswith(prefix) for prefix in TEST_RELATED_PREFIXES)
            for path in changed_files
        )
        self.assertTrue(
            has_test_change,
            "检测到 core 契约文档变更，但未检测到测试变更。"
            f"\ncore 文档变更: {changed_core_docs}"
            "\n请至少同步更新以下目录之一: " + ", ".join(TEST_RELATED_PREFIXES),
        )
