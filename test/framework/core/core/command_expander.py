from ..conf.definitions import CommandSpec, TestContext

_LEGACY_COMMAND_ALIASES = {
    "validate_structure": ["pipeline", "validate", "structure"],
    "validate_logic": ["pipeline", "validate", "logic"],
}


def normalize_command_tokens(args: list[str]) -> list[str]:
    if not args:
        return args

    normalized = _LEGACY_COMMAND_ALIASES.get(args[0])
    if normalized is None:
        return args

    return [*normalized, *args[1:]]


def expand_commands(
    context: TestContext,
    commands: list[CommandSpec],
) -> list[CommandSpec]:
    variables = {
        "data_path": str(context.source_data_path),
        "db_path": str(context.db_path) if context.db_path else "",
        "output_dir": str(context.output_dir),
        "export_output_dir": str(context.export_output_dir) if context.export_output_dir else "",
        "exe_dir": str(context.exe_path.parent),
        "processed_json_path": str(context.processed_json_path)
        if context.processed_json_path
        else "",
        "processed_json_dir": str(context.processed_json_dir) if context.processed_json_dir else "",
    }

    def safe_format(value: str) -> str:
        class SafeDict(dict):
            def __missing__(self, key):
                return "{" + key + "}"

        return value.format_map(SafeDict(variables))

    expanded = []
    for command in commands:
        formatted_args = [safe_format(str(arg)) for arg in command.args]
        if formatted_args and not command.raw_command:
            first_arg = formatted_args[0]
            if not first_arg.startswith("-"):
                formatted_args = normalize_command_tokens(formatted_args)

        expanded.append(
            CommandSpec(
                name=safe_format(command.name),
                args=formatted_args,
                stage=command.stage,
                expect_exit=command.expect_exit,
                raw_command=command.raw_command,
                add_output_dir=command.add_output_dir,
                stdin_input=safe_format(command.stdin_input) if command.stdin_input else None,
                expect_files=[safe_format(str(path)) for path in command.expect_files],
                expect_file_contains=[
                    safe_format(str(spec)) for spec in command.expect_file_contains
                ],
                expect_stdout_contains=[
                    safe_format(str(text)) for text in command.expect_stdout_contains
                ],
                expect_stdout_regex=[
                    safe_format(str(text)) for text in command.expect_stdout_regex
                ],
                expect_stdout_any_of=[
                    safe_format(str(text)) for text in command.expect_stdout_any_of
                ],
                expect_stderr_contains=[
                    safe_format(str(text)) for text in command.expect_stderr_contains
                ],
                expect_error_code=(
                    safe_format(str(command.expect_error_code))
                    if command.expect_error_code is not None
                    else None
                ),
                expect_error_category=(
                    safe_format(str(command.expect_error_category))
                    if command.expect_error_category is not None
                    else None
                ),
                expect_json_fields=[safe_format(str(spec)) for spec in command.expect_json_fields],
            )
        )
    return expanded
