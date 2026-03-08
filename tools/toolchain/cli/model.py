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
    source_scope_choices: list[str]
    default_profile: str

    @classmethod
    def from_context(cls, ctx: Context) -> "ParserDefaults":
        default_profile = (ctx.config.build.default_profile or "").strip()
        return cls(
            app_choices=list(ctx.config.apps.keys()),
            profile_choices=sorted(
                name
                for name in ctx.config.build.profiles.keys()
                if not str(name).startswith("_")
            ),
            source_scope_choices=sorted(
                name
                for name in ctx.config.tidy.source_scopes.keys()
                if not str(name).startswith("_")
            ),
            default_profile=default_profile,
        )


@dataclass(frozen=True)
class CommandSpec:
    name: str
    register: RegisterFn
    run: RunFn
    app_mode: AppMode = "required"
    add_app_path: bool = True
    help: str | None = None
