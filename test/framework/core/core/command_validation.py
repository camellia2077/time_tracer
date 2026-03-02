import re
import subprocess
from pathlib import Path

from ..conf.definitions import AppExitCode, CommandSpec, TestContext

_ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;]*m")
_COMMAND_LINE_RE = re.compile(r"^([a-z][a-z0-9-]*)\s{2,}.+$")
_ALIASES_RE = re.compile(r"\[aliases?:\s*([^\]]+)\]", re.IGNORECASE)
_COMMAND_BLOCK_START_HEADERS = {
    "available commands:",
    "commands:",
    "subcommands:",
}
_COMMAND_BLOCK_END_HEADERS = {
    "global options:",
    "options:",
    "flags:",
    "arguments:",
}


def strip_ansi_codes(text: str) -> str:
    if not text:
        return ""
    return _ANSI_ESCAPE_RE.sub("", text)


def is_cli_subcommand_token(token: str) -> bool:
    stripped = (token or "").strip()
    if not stripped:
        return False
    if stripped.startswith("-") or stripped.startswith("/"):
        return False
    return bool(re.match(r"^[a-z][a-z0-9-]*$", stripped))


def _parse_commands_from_help_output(output_text: str) -> set[str]:
    commands: set[str] = set()
    in_command_block = False

    for raw_line in output_text.splitlines():
        line = raw_line.strip()
        normalized = line.lower()

        if normalized in _COMMAND_BLOCK_START_HEADERS:
            in_command_block = True
            continue

        if not in_command_block:
            continue

        if normalized in _COMMAND_BLOCK_END_HEADERS:
            break
        if not line or line.endswith(":"):
            continue

        match = _COMMAND_LINE_RE.match(line)
        if match:
            commands.add(match.group(1))
            alias_match = _ALIASES_RE.search(line)
            if alias_match:
                aliases_raw = alias_match.group(1)
                for alias in re.split(r"[,\s]+", aliases_raw.strip()):
                    alias_token = alias.strip()
                    if is_cli_subcommand_token(alias_token):
                        commands.add(alias_token)

    if commands:
        return commands

    skip_tokens = {
        "usage",
        "command",
        "commands",
        "options",
        "flags",
        "arguments",
        "global",
        "available",
    }
    for raw_line in output_text.splitlines():
        line = raw_line.strip()
        if not line or line.endswith(":"):
            continue
        match = _COMMAND_LINE_RE.match(line)
        if not match:
            continue
        token = match.group(1)
        if token in skip_tokens:
            continue
        commands.add(token)
        alias_match = _ALIASES_RE.search(line)
        if alias_match:
            aliases_raw = alias_match.group(1)
            for alias in re.split(r"[,\s]+", aliases_raw.strip()):
                alias_token = alias.strip()
                if is_cli_subcommand_token(alias_token):
                    commands.add(alias_token)

    return commands


def discover_available_commands(exe_path: Path, cache: set[str] | None = None) -> set[str]:
    if cache is not None:
        return set(cache)

    result = subprocess.run(
        [str(exe_path), "--help"],
        cwd=str(exe_path.parent),
        capture_output=True,
        text=True,
        check=False,
        encoding="utf-8",
        errors="ignore",
    )
    if result.returncode != 0:
        stderr_tail = strip_ansi_codes(result.stderr or "").strip()
        raise RuntimeError(
            f"Failed to fetch CLI command list via --help (exit={result.returncode}). {stderr_tail}"
        )

    output_text = strip_ansi_codes((result.stdout or "") + "\n" + (result.stderr or ""))
    return _parse_commands_from_help_output(output_text)


def validate_command_specs(
    context: TestContext,
    commands: list[CommandSpec],
    cache: set[str] | None = None,
) -> set[str] | None:
    errors: list[str] = []
    candidates: list[tuple[CommandSpec, str]] = []

    for spec in commands:
        if spec.raw_command:
            continue
        if not spec.args:
            errors.append(f"{spec.name}: empty args")
            continue

        command_name = spec.args[0].strip()
        if not is_cli_subcommand_token(command_name):
            continue
        candidates.append((spec, command_name))

    if not candidates:
        if errors:
            details = "\n".join(f"  - {item}" for item in errors)
            raise ValueError(f"Command spec mismatch with CLI --help:\n{details}")
        return cache

    available_commands = discover_available_commands(context.exe_path, cache)
    if not available_commands:
        return set()

    for spec, command_name in candidates:
        if command_name not in available_commands:
            if int(spec.expect_exit) == AppExitCode.COMMAND_NOT_FOUND:
                continue
            available_str = ", ".join(sorted(available_commands))
            errors.append(
                f"{spec.name}: unknown command '{command_name}' (available: {available_str})"
            )

    if errors:
        details = "\n".join(f"  - {item}" for item in errors)
        raise ValueError(f"Command spec mismatch with CLI --help:\n{details}")
    return set(available_commands)
