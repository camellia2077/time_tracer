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
)


class Context:
    def __init__(self, repo_root: Path):
        self.repo_root = repo_root
        self.apps_root = repo_root / "apps"
        self._app_path_overrides = {}
        self.config = self._load_config()

    def _load_config(self) -> AgentConfig:
        config_path = self.repo_root / "scripts" / "toolchain" / "config.toml"
        if not config_path.exists():
            return AgentConfig()

        try:
            with open(config_path, "rb") as f:
                data = tomllib.load(f)

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
            rename_data = data.get("rename", {})
            post_change_data = data.get("post_change", {})
            return AgentConfig(
                apps=apps_data,
                build=build_cfg,
                tidy=TidyConfig(**tidy_data),
                rename=RenameConfig(**rename_data),
                post_change=PostChangeConfig(**post_change_data),
            )
        except Exception as e:
            print(f"Warning: Failed to load config from {config_path}: {e}. Using defaults.")
            return AgentConfig()

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
