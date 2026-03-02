from .model import CommandSpec


def command_specs() -> list[CommandSpec]:
    # Local imports keep command registry cheap and avoid early import cycles.
    from .handlers import (
        build,
        clean,
        config_migrate,
        configure,
        post_change,
        rename,
    )
    from .handlers.quality import (
        artifact_size,
        format as format_handler,
        lint,
        refresh_golden,
        self_test,
        verify,
    )
    from .handlers.tidy import (
        tidy,
        tidy_batch,
        tidy_close,
        tidy_fix,
        tidy_flow,
        tidy_loop,
        tidy_refresh,
        tidy_split,
    )

    return [
        configure.COMMAND,
        build.COMMAND,
        verify.COMMAND,
        refresh_golden.COMMAND,
        artifact_size.COMMAND,
        format_handler.COMMAND,
        lint.COMMAND,
        tidy.COMMAND,
        tidy_split.COMMAND,
        tidy_fix.COMMAND,
        tidy_loop.COMMAND,
        tidy_flow.COMMAND,
        tidy_refresh.COMMAND,
        tidy_batch.COMMAND,
        tidy_close.COMMAND,
        clean.COMMAND,
        rename.RENAME_PLAN_COMMAND,
        rename.RENAME_APPLY_COMMAND,
        rename.RENAME_AUDIT_COMMAND,
        post_change.COMMAND,
        self_test.COMMAND,
        config_migrate.COMMAND,
    ]


def command_map() -> dict[str, CommandSpec]:
    return {spec.name: spec for spec in command_specs()}
