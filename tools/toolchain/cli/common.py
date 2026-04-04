import argparse
import shlex
import sys
from collections.abc import Iterable

from ..core.context import Context
from ..commands.tidy import clang_tidy_config
from .model import ParserDefaults


def parse_cmake_args(raw_args: Iterable[str] | None) -> list[str]:
    flattened: list[str] = []
    for raw in raw_args or []:
        flattened.extend(shlex.split(raw, posix=False))
    return flattened


def print_cli_error(message: str) -> None:
    print(message, file=sys.stderr)


def apply_app_path_override(args: argparse.Namespace, ctx: Context) -> int:
    if getattr(args, "app_path", None):
        if not getattr(args, "app", None):
            print_cli_error("Error: --app-path requires --app.")
            return 2
        ctx.set_app_path_override(args.app, args.app_path)
    return 0


def resolve_fixed_build_dir(ctx: Context, app_name: str | None) -> str | None:
    if not app_name:
        return None
    fixed_build_dir = (getattr(ctx.get_app_metadata(app_name), "fixed_build_dir", "") or "").strip()
    if fixed_build_dir:
        return fixed_build_dir
    return None


def reject_unsupported_build_dir_override(
    *,
    ctx: Context,
    app_name: str | None,
    build_dir_name: str | None,
    command_name: str,
) -> int:
    normalized_build_dir = (build_dir_name or "").strip()
    if not normalized_build_dir:
        return 0
    fixed_build_dir = resolve_fixed_build_dir(ctx, app_name)
    if not fixed_build_dir:
        return 0
    print_cli_error(
        f"Error: `{command_name}` does not support `--build-dir` for app `{app_name}`. "
        f"This backend always uses `{fixed_build_dir}`."
    )
    return 2


def add_profile_arg(parser_obj: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg_with_options(parser_obj, defaults, allow_multiple=False)


def add_profile_arg_with_options(
    parser_obj: argparse.ArgumentParser,
    defaults: ParserDefaults,
    *,
    allow_multiple: bool,
) -> None:
    help_text = (
        "Build profile from tools/toolchain/config/*.toml."
    )
    if allow_multiple:
        help_text += " Repeat `--profile` to merge multiple profiles into one backend invocation when supported."
    if defaults.default_profile:
        help_text += f" Default profile: {defaults.default_profile}."
    argument_kwargs = {
        "default": None,
        "help": help_text,
    }
    if allow_multiple:
        argument_kwargs["action"] = "append"
    if defaults.profile_choices:
        argument_kwargs["choices"] = defaults.profile_choices
        parser_obj.add_argument("--profile", **argument_kwargs)
    else:
        parser_obj.add_argument("--profile", **argument_kwargs)


def normalize_profile_selection(
    raw_profiles: str | Iterable[str] | None,
) -> str | list[str] | None:
    if raw_profiles is None:
        return None
    if isinstance(raw_profiles, str):
        normalized = raw_profiles.strip()
        return normalized or None

    seen: set[str] = set()
    normalized_profiles: list[str] = []
    for raw_profile in raw_profiles:
        profile_name = str(raw_profile or "").strip()
        if not profile_name or profile_name in seen:
            continue
        seen.add(profile_name)
        normalized_profiles.append(profile_name)
    if not normalized_profiles:
        return None
    if len(normalized_profiles) == 1:
        return normalized_profiles[0]
    return normalized_profiles


def append_profiles_to_command(
    parts: list[str],
    profiles: str | Iterable[str] | None,
) -> None:
    normalized_profiles = normalize_profile_selection(profiles)
    if normalized_profiles is None:
        return
    if isinstance(normalized_profiles, str):
        parts.extend(["--profile", normalized_profiles])
        return
    for profile_name in normalized_profiles:
        parts.extend(["--profile", profile_name])


def add_build_dir_arg(
    parser_obj: argparse.ArgumentParser,
    *,
    help_text: str = (
        "Override build directory name for backends without a fixed build directory "
        "(for example CMake). Fixed-dir backends like `tracer_android` reject this flag."
    ),
) -> None:
    parser_obj.add_argument(
        "--build-dir",
        default=None,
        help=help_text,
    )


def add_kill_build_procs_args(parser_obj: argparse.ArgumentParser) -> None:
    parser_obj.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before configure/build (default: off)",
    )
    parser_obj.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )


def add_concise_arg(
    parser_obj: argparse.ArgumentParser,
    *,
    help_text: str = "Use concise top-level output with backend logs written to a log file.",
) -> None:
    parser_obj.add_argument(
        "--concise",
        action="store_true",
        help=help_text,
    )


def add_source_scope_arg(
    parser_obj: argparse.ArgumentParser,
    defaults: ParserDefaults,
    *,
    help_suffix: str = "",
) -> None:
    help_text = "Named clang-tidy source scope preset from tools/toolchain/config/*.toml."
    if defaults.source_scope_choices:
        help_text += " Available: " + ", ".join(defaults.source_scope_choices) + "."
    if help_suffix:
        help_text += f" {help_suffix}"
    if defaults.source_scope_choices:
        parser_obj.add_argument(
            "--source-scope",
            choices=defaults.source_scope_choices,
            default=None,
            help=help_text,
        )
    else:
        parser_obj.add_argument("--source-scope", default=None, help=help_text)


def add_tidy_build_dir_arg(
    parser_obj: argparse.ArgumentParser,
    *,
    help_text: str = "Override tidy workspace directory name (default: build_tidy).",
) -> None:
    parser_obj.add_argument(
        "--tidy-build-dir",
        default=None,
        help=help_text,
    )


def add_tidy_config_args(parser_obj: argparse.ArgumentParser) -> None:
    tidy_config_group = parser_obj.add_mutually_exclusive_group()
    tidy_config_group.add_argument(
        "--config-file",
        default=None,
        help=(
            "Use an explicit clang-tidy config file. Relative paths are resolved from "
            "the repo root."
        ),
    )
    tidy_config_group.add_argument(
        "--strict-config",
        action="store_true",
        help=(
            "Use the repo-root strict clang-tidy profile "
            f"(`{clang_tidy_config.STRICT_CONFIG_FILE}`)."
        ),
    )


def append_tidy_config_to_command(
    parts: list[str],
    *,
    config_file: str | None,
    strict_config: bool,
) -> None:
    parts.extend(
        clang_tidy_config.build_cli_args(
            config_file=config_file,
            strict_config=strict_config,
        )
    )


def add_tidy_task_view_arg(
    parser_obj: argparse.ArgumentParser,
    *,
    default: str | None = None,
    help_text: str | None = None,
    choices: tuple[str, ...] | None = None,
) -> None:
    effective_choices = list(choices or ("json", "text", "toon", "text+toon"))
    effective_help = (
        help_text
        or "Optional extra task artifact view(s). Canonical task_*.json is always written; "
        "when omitted, reuse the current queue contract or fall back to `toon`."
    )
    parser_obj.add_argument(
        "--task-view",
        choices=effective_choices,
        default=default,
        help=effective_help,
    )


def add_task_selector_args(parser_obj: argparse.ArgumentParser) -> None:
    parser_obj.add_argument(
        "--task-log",
        default=None,
        help=(
            "Explicit task artifact path (.json/.toon/.log). Preferred after refresh/rebase; "
            "overrides --batch-id/--task-id selection."
        ),
    )
    parser_obj.add_argument(
        "--batch-id",
        default=None,
        help="Queue batch identifier under tasks/ (e.g. 2, 002, batch_002).",
    )
    parser_obj.add_argument(
        "--task-id",
        default=None,
        help="Task identifier (e.g. 11, 011). When omitted, use the smallest pending task.",
    )


def add_required_task_log_arg(
    parser_obj: argparse.ArgumentParser,
    *,
    help_text: str | None = None,
) -> None:
    parser_obj.add_argument(
        "--task-log",
        required=True,
        help=(
            help_text
            or "Canonical task artifact path (.json/.toon/.log). "
            "This command resolves app and tidy workspace from the task path and always "
            "uses the matching task_*.json contract."
        ),
    )
