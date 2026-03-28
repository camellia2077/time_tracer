from .model import CommandSpec


def command_specs() -> list[CommandSpec]:
    # Local imports keep command registry cheap and avoid early import cycles.
    from .handlers import (
        analyze,
        analyze_split,
        build,
        clean,
        config_migrate,
        configure,
        rename,
        validate,
    )
    from .handlers.quality import (
        artifact_size,
        format as format_handler,
        lint,
        report_markdown_gate,
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
        tidy_step,
        tidy_task_fix,
        tidy_task_patch,
        tidy_task_suggest,
    )

    return [
        configure.COMMAND,
        build.COMMAND,
        analyze.COMMAND,
        analyze_split.COMMAND,
        verify.COMMAND,
        report_markdown_gate.COMMAND,
        refresh_golden.COMMAND,
        artifact_size.COMMAND,
        format_handler.COMMAND,
        lint.COMMAND,
        tidy.COMMAND,
        tidy_split.COMMAND,
        tidy_fix.COMMAND,
        tidy_task_fix.COMMAND,
        tidy_task_patch.COMMAND,
        tidy_task_suggest.COMMAND,
        tidy_step.COMMAND,
        tidy_loop.COMMAND,
        tidy_flow.COMMAND,
        tidy_refresh.COMMAND,
        tidy_batch.COMMAND,
        tidy_close.COMMAND,
        clean.COMMAND,
        rename.RENAME_PLAN_COMMAND,
        rename.RENAME_APPLY_COMMAND,
        rename.RENAME_AUDIT_COMMAND,
        validate.COMMAND,
        self_test.COMMAND,
        config_migrate.COMMAND,
    ]


def command_map() -> dict[str, CommandSpec]:
    return {spec.name: spec for spec in command_specs()}
