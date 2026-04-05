from __future__ import annotations

from pathlib import Path
from typing import Any

from ..models import AutoFixRule, FixContext, FixIntent, FixOperation, RuleMetadata


class RuleBase:
    metadata: RuleMetadata
    rule_id: str
    action_kind: str
    supported_checks: tuple[str, ...]
    engine_id: str
    preview_only: bool
    risk_level: str

    def __init_subclass__(cls) -> None:
        super().__init_subclass__()
        if not hasattr(cls, "metadata"):
            return
        metadata = cls.metadata
        cls.rule_id = metadata.rule_id
        cls.action_kind = metadata.action_kind
        cls.supported_checks = metadata.supported_checks
        cls.engine_id = metadata.engine_id
        cls.preview_only = metadata.preview_only
        cls.risk_level = metadata.risk_level

    def supports(self, check: str) -> bool:
        return check in self.supported_checks

    def build_intent(
        self,
        *,
        intent_id: str,
        check: str,
        file_path: str,
        line: int,
        col: int,
        operation: FixOperation,
    ) -> FixIntent:
        return FixIntent(
            intent_id=intent_id,
            check=check,
            file_path=file_path,
            line=line,
            col=col,
            metadata=self.metadata,
            operation=operation,
            preview_only=self.preview_only,
        )

    @staticmethod
    def resolve_source_file(context: FixContext, diagnostic: Any) -> Path:
        return Path(diagnostic.file or context.parsed.source_file)


__all__ = ["AutoFixRule", "RuleBase"]
