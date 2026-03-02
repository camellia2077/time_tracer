import os
import tomllib
from pathlib import Path

from .config import (
    AgentConfig,
    AppConfig,
    BuildConfig,
    BuildProfileConfig,
    PostChangeConfig,
    RenameConfig,
    TidyConfig,
    TidyFixStrategyConfig,
)


class Context:
    def __init__(self, repo_root: Path):
        self.repo_root = repo_root
        self.apps_root = repo_root / "apps"
        self._app_path_overrides = {}
        self.config = self._load_config()

    def _load_config(self) -> AgentConfig:
        try:
            data = self._load_raw_config_data()
            if not data:
                return AgentConfig()

            apps_data = {}
            for name, meta in data.get("apps", {}).items():
                apps_data[name] = AppConfig(**meta)

            build_data = data.get("build", {})
            build_profiles_data = build_data.get("profiles", {})
            build_data_without_profiles = dict(build_data)
            build_data_without_profiles.pop("profiles", None)
            build_cfg = BuildConfig(**build_data_without_profiles)
            if isinstance(build_profiles_data, dict):
                for profile_name, profile_meta in build_profiles_data.items():
                    if not isinstance(profile_meta, dict):
                        continue
                    build_cfg.profiles[profile_name] = BuildProfileConfig(**profile_meta)
            tidy_data = data.get("tidy", {})
            tidy_data_without_fix_strategy = dict(tidy_data)
            fix_strategy_data = tidy_data_without_fix_strategy.pop("fix_strategy", {})
            tidy_cfg = TidyConfig(**tidy_data_without_fix_strategy)
            if isinstance(fix_strategy_data, dict):
                tidy_cfg.fix_strategy = TidyFixStrategyConfig(**fix_strategy_data)
            rename_data = data.get("rename", {})
            post_change_data = data.get("post_change", {})
            return AgentConfig(
                apps=apps_data,
                build=build_cfg,
                tidy=tidy_cfg,
                rename=RenameConfig(**rename_data),
                post_change=PostChangeConfig(**post_change_data),
            )
        except Exception as e:
            print(
                "Warning: Failed to load config from scripts/toolchain/config.toml "
                f"and/or scripts/toolchain/config/*.toml: {e}. Using defaults."
            )
            return AgentConfig()

    def _load_raw_config_data(self) -> dict:
        base_path = self.repo_root / "scripts" / "toolchain" / "config.toml"
        split_dir = self.repo_root / "scripts" / "toolchain" / "config"

        merged: dict = {}
        base_data: dict = {}
        fragment_data_list: list[dict] = []

        if base_path.exists():
            base_data = self._load_toml_file(base_path)
            merged = self._deep_merge_dicts(merged, base_data)

        if split_dir.is_dir():
            for fragment_path in sorted(split_dir.glob("*.toml")):
                fragment_data = self._load_toml_file(fragment_path)
                fragment_data_list.append(fragment_data)
                merged = self._deep_merge_dicts(merged, fragment_data)

        self._warn_deprecated_base_overlaps(base_data, fragment_data_list)

        return merged

    @staticmethod
    def _load_toml_file(path: Path) -> dict:
        with open(path, "rb") as handle:
            data = tomllib.load(handle)
        if isinstance(data, dict):
            return data
        return {}

    @classmethod
    def _deep_merge_dicts(cls, left: dict, right: dict) -> dict:
        merged = dict(left)
        for key, value in right.items():
            current = merged.get(key)
            if isinstance(current, dict) and isinstance(value, dict):
                merged[key] = cls._deep_merge_dicts(current, value)
            else:
                merged[key] = value
        return merged

    @classmethod
    def _collect_leaf_key_paths(
        cls,
        data: dict,
        prefix: tuple[str, ...] = (),
    ) -> set[tuple[str, ...]]:
        paths: set[tuple[str, ...]] = set()
        for key, value in data.items():
            next_prefix = (*prefix, str(key))
            if isinstance(value, dict):
                paths |= cls._collect_leaf_key_paths(value, next_prefix)
            else:
                paths.add(next_prefix)
        return paths

    @classmethod
    def _warn_deprecated_base_overlaps(
        cls,
        base_data: dict,
        fragment_data_list: list[dict],
    ) -> None:
        if not base_data or not fragment_data_list:
            return

        base_paths = cls._collect_leaf_key_paths(base_data)
        if not base_paths:
            return

        fragment_paths: set[tuple[str, ...]] = set()
        for fragment_data in fragment_data_list:
            fragment_paths |= cls._collect_leaf_key_paths(fragment_data)

        overlaps = sorted(base_paths & fragment_paths)
        if not overlaps:
            return

        sample_limit = 8
        sample = ", ".join(".".join(path) for path in overlaps[:sample_limit])
        more_count = max(0, len(overlaps) - sample_limit)
        more_suffix = f" (+{more_count} more)" if more_count else ""
        print(
            "Warning: deprecated duplicate config keys detected between "
            "scripts/toolchain/config.toml and scripts/toolchain/config/*.toml. "
            f"Prefer keeping these keys only in split files. "
            f"keys: {sample}{more_suffix}"
        )

    def set_app_path_override(self, app_name: str, path: str):
        """Manually override an application path (e.g. from CLI)."""
        if app_name not in self.config.apps:
            # Dynamically create app config if missing but overridden via CLI
            self.config.apps[app_name] = AppConfig(path=path)
        self._app_path_overrides[app_name] = Path(path)

    def get_app_dir(self, app_name: str) -> Path:
        # 1. Check overrides
        if app_name in self._app_path_overrides:
            path = self._app_path_overrides[app_name]
            return path if path.is_absolute() else self.repo_root / path

        # 2. Check config
        if app_name in self.config.apps:
            path = Path(self.config.apps[app_name].path)
            return path if path.is_absolute() else self.repo_root / path

        # 3. Fallback
        return self.apps_root / app_name

    def get_app_metadata(self, app_name: str) -> AppConfig:
        if app_name not in self.config.apps:
            # Return a default if not found
            return AppConfig(path=str(self.get_app_dir(app_name)))
        return self.config.apps[app_name]

    def setup_env(self):
        """Setup specialized environment for toolchain."""
        env = os.environ.copy()
        extra_paths = [r"C:\msys64\ucrt64\bin", r"C:\msys64\usr\bin"]
        env["PATH"] = os.pathsep.join(extra_paths + [env.get("PATH", "")])
        return env
