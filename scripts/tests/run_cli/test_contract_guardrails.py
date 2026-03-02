from __future__ import annotations

from pathlib import Path
from unittest import TestCase

try:
    import tomllib
except ModuleNotFoundError:  # pragma: no cover
    import tomli as tomllib  # type: ignore


REPO_ROOT = Path(__file__).resolve().parents[3]
SUITE_ROOT = REPO_ROOT / "test" / "suites" / "tracer_windows_rust_cli"
if not SUITE_ROOT.exists():
    candidates = sorted((REPO_ROOT / "test" / "suites").glob("*windows*cli*"))
    if candidates:
        SUITE_ROOT = candidates[0]
SUITE_TOML = SUITE_ROOT / "tests.toml"

CONTRACT_ASSERTION_KEYS = (
    "expect_error_code",
    "expect_error_category",
    "expect_json_fields",
    "expect_stdout_regex",
    "expect_stdout_any_of",
    "expect_stdout_contains",
    "expect_stderr_contains",
    "expect_files",
    "expect_file_contains",
)

# 历史基线：这些命令当前仍只有最小断言（主要是 expect_exit）。
# 守门规则要求“新增命令”不得继续只做最小断言。
LEGACY_MINIMAL_CONTRACT_COMMAND_KEYS = {
    "export month",
    "export recent",
    "export week",
    "export year",
    "query data activity-suggest",
    "query data days",
    "query data months",
    "query data search",
    "query data tree",
    "query day",
    "query month",
    "query recent",
    "query week",
    "query year",
}


def _normalize_list(value) -> list[str]:
    if value is None:
        return []
    if isinstance(value, list):
        return [str(item) for item in value]
    return [str(value)]


def _load_toml(path: Path) -> dict:
    return tomllib.loads(path.read_text(encoding="utf-8"))


def _has_contract_assertion(item: dict) -> bool:
    for key in CONTRACT_ASSERTION_KEYS:
        value = item.get(key)
        if value in (None, ""):
            continue
        if isinstance(value, list) and not value:
            continue
        return True
    return False


def _command_key(args: list[str]) -> str:
    tokens: list[str] = []
    for arg in args:
        if arg.startswith("-"):
            break
        tokens.append(arg)
        if len(tokens) >= 4:
            break

    if not tokens:
        return "<empty>"

    if tokens[0] == "query" and len(tokens) >= 3 and tokens[1] == "data":
        return f"query data {tokens[2]}"
    if tokens[0] in {"query", "export", "crypto"} and len(tokens) >= 2:
        return f"{tokens[0]} {tokens[1]}"
    return tokens[0]


class TestContractGuardrails(TestCase):
    def _iter_suite_cases(self):
        suite = _load_toml(SUITE_TOML)
        includes = suite.get("includes", [])
        self.assertIsInstance(includes, list, f"{SUITE_TOML.as_posix()} includes must be a list.")

        for relative in includes:
            include_path = SUITE_ROOT / str(relative)
            self.assertTrue(
                include_path.exists(), f"Include file not found: {include_path.as_posix()}"
            )

            data = _load_toml(include_path)
            for section in ("commands", "command_groups"):
                items = data.get(section, [])
                if not isinstance(items, list):
                    continue
                for item in items:
                    if isinstance(item, dict):
                        yield include_path, section, item

    def test_new_command_requires_contract_assertions(self):
        has_rich_contract_by_command: dict[str, bool] = {}
        minimal_locations: dict[str, list[str]] = {}

        for include_path, section, item in self._iter_suite_cases():
            args = item.get("args")
            if section == "command_groups":
                args = item.get("template") or item.get("args")

            args_list = _normalize_list(args)
            self.assertTrue(
                args_list,
                f"{include_path.as_posix()} has empty args in section `{section}`: {item}",
            )

            key = _command_key(args_list)
            has_contract = _has_contract_assertion(item)

            current = has_rich_contract_by_command.get(key, False)
            has_rich_contract_by_command[key] = current or has_contract

            if not has_contract:
                location = f"{include_path.relative_to(REPO_ROOT).as_posix()}::{item.get('name', '<unnamed>')}"
                minimal_locations.setdefault(key, []).append(location)

        minimal_only_commands = {
            key for key, has_rich in has_rich_contract_by_command.items() if not has_rich
        }
        unexpected_minimal_only = sorted(
            minimal_only_commands - LEGACY_MINIMAL_CONTRACT_COMMAND_KEYS
        )

        if unexpected_minimal_only:
            lines: list[str] = []
            for key in unexpected_minimal_only:
                locations = ", ".join(minimal_locations.get(key, []))
                lines.append(f"{key}: {locations}")
            self.fail(
                "新增命令不能只有最小断言(expect_exit)。请为命令补充契约断言"
                "(error/json/stdout_regex/stdout_any_of 等)。"
                "\n违规命令:\n- " + "\n- ".join(lines)
            )
