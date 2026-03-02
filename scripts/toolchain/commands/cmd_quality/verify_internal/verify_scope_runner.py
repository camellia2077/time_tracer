from __future__ import annotations

from collections.abc import Callable
from typing import Literal

VerifyScope = Literal["task", "unit", "artifact", "batch"]


def run_scope_checks(
    *,
    verify_scope: VerifyScope,
    run_task_scope_checks: Callable[[], int],
    run_unit_scope_checks: Callable[[], int],
    run_artifact_scope_checks: Callable[[], int],
) -> int:
    if verify_scope == "task":
        return run_task_scope_checks()
    if verify_scope == "unit":
        return run_unit_scope_checks()
    if verify_scope == "artifact":
        return run_artifact_scope_checks()

    unit_ret = run_unit_scope_checks()
    if unit_ret != 0:
        return unit_ret
    return run_artifact_scope_checks()
