from pathlib import Path
from typing import Iterable, Sequence

from ..conf.definitions import LogRoutingRule


class LogRoutingManager:
    def __init__(
        self,
        log_dir: Path,
        module_name: str,
        rules: Iterable[LogRoutingRule] | None = None,
    ):
        self.log_dir = log_dir
        self.module_name = module_name.strip().lower()
        self.rules = self._filter_rules_for_module(rules or [])
        self._organize_legacy_logs()

    def _filter_rules_for_module(
        self, rules: Iterable[LogRoutingRule]
    ) -> list[LogRoutingRule]:
        matched_rules: list[LogRoutingRule] = []
        for rule in rules:
            if rule.stage.strip().lower() == self.module_name:
                matched_rules.append(rule)
        return matched_rules

    def resolve_subdir(self, command_args: Sequence[str]) -> str:
        if not self.rules:
            return ""

        normalized_args = [str(arg).strip().lower() for arg in command_args]
        for rule in self.rules:
            prefix = [item.strip().lower() for item in rule.command_prefix]
            if prefix and normalized_args[: len(prefix)] != prefix:
                continue
            return rule.subdir
        return ""

    def _organize_legacy_logs(self) -> None:
        if not self.rules:
            return

        for rule in self.rules:
            (self.log_dir / rule.subdir).mkdir(parents=True, exist_ok=True)

        for log_file in self.log_dir.glob("*.log"):
            target_rule = self._find_rule_for_legacy_log(log_file.name)
            if target_rule is None:
                continue

            target_path = self.log_dir / target_rule.subdir / log_file.name
            if target_path.exists():
                log_file.unlink()
            else:
                log_file.replace(target_path)

    def _find_rule_for_legacy_log(self, filename: str) -> LogRoutingRule | None:
        lower_name = filename.strip().lower()
        for rule in self.rules:
            if any(token in lower_name for token in rule.legacy_name_contains):
                return rule
        return None
