# test/conf/loader.py
from pathlib import Path

from .definitions import GlobalConfig
from .loader_commands import _load_commands
from .loader_sections import (
    _load_cleanup_params,
    _load_cli_names,
    _load_log_routing,
    _load_paths,
    _load_pipeline,
    _load_run_control,
    _load_test_params,
)
from .loader_toml import _detect_repo_root, _load_toml_with_includes
from .schema_validator import validate_suite_schema


def _build_config_payload(
    toml_data: dict,
    pipeline_cfg,
) -> GlobalConfig:
    paths_cfg = _load_paths(toml_data)
    if pipeline_cfg.MODE == "staged" and not paths_cfg.PROCESSED_JSON_DIR:
        raise ValueError(
            "Config error: [paths].processed_json_dir is required when pipeline.mode = 'staged'."
        )

    return GlobalConfig(
        paths=paths_cfg,
        cli_names=_load_cli_names(toml_data),
        test_params=_load_test_params(toml_data),
        cleanup=_load_cleanup_params(toml_data),
        log_routing=_load_log_routing(toml_data),
        run_control=_load_run_control(toml_data),
        pipeline=pipeline_cfg,
        commands=_load_commands(toml_data, pipeline_cfg),
    )


def load_config(
    config_path: Path | None = None,
    build_dir_name: str | None = None,
    bin_dir: str | None = None,
) -> GlobalConfig:
    """加载 config.toml 并返回统一的 GlobalConfig 对象。"""
    target_path = config_path or Path("config.toml")
    try:
        print(f"Loading config from: {target_path.absolute()}")

        repo_root = _detect_repo_root(target_path)
        toml_data = _load_toml_with_includes(
            target_path,
            variables={
                "repo_root": str(repo_root),
                "test_root": str(repo_root / "test"),
            },
        )
        validate_suite_schema(toml_data, target_path.resolve())

        if build_dir_name:
            toml_data["_build_dir_name"] = build_dir_name
        if bin_dir:
            toml_data["_bin_dir"] = bin_dir

        pipeline_cfg = _load_pipeline(toml_data)
        return _build_config_payload(toml_data, pipeline_cfg)
    except FileNotFoundError:
        raise FileNotFoundError(f"config.toml not found at: {target_path.absolute()}")
    except Exception as error:
        raise RuntimeError(f"Error loading config.toml: {error}")
