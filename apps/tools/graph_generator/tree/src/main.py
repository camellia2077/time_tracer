import os

from core.config import TreeConfig
from services.tree_service import TreeService


def run_tree_generator(
    base_dir: str,
    root_path: str | None = None,
    max_depth: int | None = None,
    path_overrides: dict | None = None,
    setting_overrides: dict | None = None,
    config_dir: str | None = None,
) -> None:
    print("--- Initializing tree graph generator ---")
    if config_dir:
        config_path = os.path.join(config_dir, "tree_config.toml")
    else:
        config_path = os.path.join(base_dir, "configs", "tree_config.toml")
    config = TreeConfig(config_path)
    config.apply_environment()
    config.apply_overrides(paths=path_overrides, settings=setting_overrides)

    default_output_dir = os.path.join(base_dir, "output")
    service = TreeService(config, default_output_dir)
    service.generate_tree(root_path_override=root_path, max_depth_override=max_depth)


if __name__ == "__main__":
    run_tree_generator(os.path.dirname(os.path.abspath(__file__)))
