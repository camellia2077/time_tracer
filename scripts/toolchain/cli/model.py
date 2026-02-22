import argparse
from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal

from ..core.context import Context

AppMode = Literal["required", "optional", "none"]
RegisterFn = Callable[[argparse.ArgumentParser, "ParserDefaults"], None]
RunFn = Callable[[argparse.Namespace, Context], int]


@dataclass(frozen=True)
class ParserDefaults:
    app_choices: list[str]
    profile_choices: list[str]
    default_profile: str
    post_change_default_run_tests: str
    post_change_default_build_dir: str
    post_change_default_script_changes: str

    @classmethod
    def from_context(cls, ctx: Context) -> "ParserDefaults":
        default_profile = (ctx.config.build.default_profile or "").strip()
        post_change_default_run_tests = (
            ctx.config.post_change.run_tests
            if ctx.config.post_change.run_tests in {"auto", "always", "never"}
            else "auto"
        )
        post_change_default_build_dir = (
            ctx.config.post_change.default_build_dir.strip()
            if ctx.config.post_change.default_build_dir
            else "build_fast"
        )
        if not post_change_default_build_dir:
            post_change_default_build_dir = "build_fast"
        post_change_default_script_changes = (
            ctx.config.post_change.script_changes
            if ctx.config.post_change.script_changes in {"auto", "build", "skip"}
            else "build"
        )
        return cls(
            app_choices=list(ctx.config.apps.keys()),
            profile_choices=sorted(ctx.config.build.profiles.keys()),
            default_profile=default_profile,
            post_change_default_run_tests=post_change_default_run_tests,
            post_change_default_build_dir=post_change_default_build_dir,
            post_change_default_script_changes=post_change_default_script_changes,
        )


@dataclass(frozen=True)
class CommandSpec:
    name: str
    register: RegisterFn
    run: RunFn
    app_mode: AppMode = "required"
    add_app_path: bool = True
    help: str | None = None
