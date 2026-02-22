from .definitions import CommandSpec, PipelineConfig


def _normalize_list(value):
    if value is None:
        return []
    if isinstance(value, list):
        return value
    return [value]


def _safe_format(template: str, values: dict) -> str:
    class SafeDict(dict):
        def __missing__(self, key):
            return "{" + key + "}"

    return template.format_map(SafeDict(values))


def _parse_command_item(item: dict) -> CommandSpec:
    name = item.get("name")
    args = _normalize_list(item.get("args"))
    if not name or not args:
        raise ValueError("Each command must define non-empty 'name' and 'args'.")

    return CommandSpec(
        name=name,
        args=[str(arg) for arg in args],
        stage=item.get("stage", "commands"),
        expect_exit=int(item.get("expect_exit", 0)),
        raw_command=bool(item.get("raw_command", False)),
        add_output_dir=bool(item.get("add_output_dir", False)),
        stdin_input=item.get("stdin_input"),
        expect_files=[str(path) for path in _normalize_list(item.get("expect_files"))],
        expect_file_contains=[
            str(path) for path in _normalize_list(item.get("expect_file_contains"))
        ],
        expect_stdout_contains=[
            str(text) for text in _normalize_list(item.get("expect_stdout_contains"))
        ],
        expect_stderr_contains=[
            str(text) for text in _normalize_list(item.get("expect_stderr_contains"))
        ],
    )


def _expand_command_groups(toml_data) -> list[CommandSpec]:
    groups = toml_data.get("command_groups", [])
    if not isinstance(groups, list):
        return []

    expanded: list[CommandSpec] = []
    for group in groups:
        if not isinstance(group, dict):
            continue

        name = group.get("name", "group")
        stage = group.get("stage", "commands")
        template = _normalize_list(group.get("template") or group.get("args"))
        if not template:
            continue

        matrix = group.get("matrix", {})
        if not isinstance(matrix, dict):
            matrix = {}

        keys = list(matrix.keys())
        value_lists = [_normalize_list(matrix[key]) for key in keys]

        if not keys:
            combos = [()]
        else:
            combos = [[]]
            for values in value_lists:
                combos = [combo + [value] for combo in combos for value in values]

        name_template = group.get("name_template")
        for combo in combos:
            variables = dict(zip(keys, combo))
            args = [_safe_format(str(item), variables) for item in template]

            if name_template:
                cmd_name = _safe_format(str(name_template), variables)
            else:
                suffix = ", ".join(f"{key}={variables[key]}" for key in keys)
                cmd_name = f"{name} ({suffix})" if suffix else name

            expanded.append(
                CommandSpec(
                    name=cmd_name,
                    args=args,
                    stage=stage,
                    expect_exit=int(group.get("expect_exit", 0)),
                    raw_command=bool(group.get("raw_command", False)),
                    add_output_dir=bool(group.get("add_output_dir", False)),
                    stdin_input=group.get("stdin_input"),
                    expect_files=[
                        _safe_format(str(path), variables)
                        for path in _normalize_list(group.get("expect_files"))
                    ],
                    expect_file_contains=[
                        _safe_format(str(path), variables)
                        for path in _normalize_list(group.get("expect_file_contains"))
                    ],
                    expect_stdout_contains=[
                        _safe_format(str(text), variables)
                        for text in _normalize_list(group.get("expect_stdout_contains"))
                    ],
                    expect_stderr_contains=[
                        _safe_format(str(text), variables)
                        for text in _normalize_list(group.get("expect_stderr_contains"))
                    ],
                )
            )

    return expanded


def _load_commands(toml_data, pipeline_cfg: PipelineConfig) -> list[CommandSpec]:
    commands: list[CommandSpec] = []
    for item in toml_data.get("commands", []):
        if isinstance(item, dict):
            commands.append(_parse_command_item(item))

    commands.extend(_expand_command_groups(toml_data))

    if pipeline_cfg.MODE == "staged":
        commands.insert(
            0,
            CommandSpec(
                name="Validate Structure",
                stage="pipeline",
                args=["validate-structure", "{data_path}"],
                expect_exit=0,
            ),
        )
        commands.insert(
            1,
            CommandSpec(
                name="Convert",
                stage="pipeline",
                args=["convert", "{data_path}"],
                expect_exit=0,
            ),
        )
        commands.insert(
            2,
            CommandSpec(
                name="Import",
                stage="pipeline",
                args=["import", "{processed_json_dir}"],
                expect_exit=0,
            ),
        )
    elif pipeline_cfg.MODE == "ingest":
        commands.insert(
            0,
            CommandSpec(
                name="Ingest Full Pipeline",
                stage="pipeline",
                args=["ingest", "{data_path}"],
                expect_exit=0,
                add_output_dir=False,
            ),
        )

    return commands
