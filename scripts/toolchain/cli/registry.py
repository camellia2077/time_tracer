from .model import CommandSpec


def command_specs() -> list[CommandSpec]:
    # Local imports keep command registry cheap and avoid early import cycles.
    from .handlers import (
        artifact_size,
        build,
        clean,
        config_migrate,
        configure,
        format as format_handler,
        post_change,
        rename_apply,
        rename_audit,
        rename_plan,
        self_test,
        tidy,
        tidy_batch,
        tidy_close,
        tidy_fix,
        tidy_flow,
        tidy_loop,
        tidy_refresh,
        tidy_split,
        verify,
    )

    return [
        configure.COMMAND,
        build.COMMAND,
        verify.COMMAND,
        artifact_size.COMMAND,
        format_handler.COMMAND,
        tidy.COMMAND,
        tidy_split.COMMAND,
        tidy_fix.COMMAND,
        tidy_loop.COMMAND,
        tidy_flow.COMMAND,
        tidy_refresh.COMMAND,
        tidy_batch.COMMAND,
        tidy_close.COMMAND,
        clean.COMMAND,
        rename_plan.COMMAND,
        rename_apply.COMMAND,
        rename_audit.COMMAND,
        post_change.COMMAND,
        self_test.COMMAND,
        config_migrate.COMMAND,
    ]


def command_map() -> dict[str, CommandSpec]:
    return {spec.name: spec for spec in command_specs()}
