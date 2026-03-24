import os
import tomllib
from dataclasses import fields
from pathlib import Path

from .config import (
    AgentConfig,
    AppConfig,
    BuildConfig,
    BuildProfileConfig,
    QualityConfig,
    QualityGateAuditConfig,
    RenameConfig,
    TidyConfig,
    TidyFixStrategyConfig,
    TidySourceScopeConfig,
)
from .generated_paths import (
    resolve_analyze_layout,
    resolve_build_layout,
    resolve_out_root,
    resolve_test_result_layout,
    resolve_test_result_layout_for_app,
    resolve_tidy_layout,
    resolve_validation_layout,
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
                apps_data[name] = AppConfig(
                    **self._filter_dataclass_kwargs(AppConfig, meta)
                )

            build_data = data.get("build", {})
            build_profiles_data = build_data.get("profiles", {})
            build_data_without_profiles = dict(build_data)
            build_data_without_profiles.pop("profiles", None)
            build_cfg = BuildConfig(
                **self._filter_dataclass_kwargs(BuildConfig, build_data_without_profiles)
            )
            if isinstance(build_profiles_data, dict):
                build_cfg.profiles = self._resolve_build_profiles(build_profiles_data)

            quality_data = data.get("quality", {})
            quality_data_without_gate = dict(quality_data)
            gate_audit_data = quality_data_without_gate.pop("gate_audit", {})
            quality_cfg = QualityConfig(
                **self._filter_dataclass_kwargs(QualityConfig, quality_data_without_gate)
            )
            if isinstance(gate_audit_data, dict):
                quality_cfg.gate_audit = QualityGateAuditConfig(
                    **self._filter_dataclass_kwargs(QualityGateAuditConfig, gate_audit_data)
                )

            tidy_data = data.get("tidy", {})
            tidy_data_without_fix_strategy = dict(tidy_data)
            fix_strategy_data = tidy_data_without_fix_strategy.pop("fix_strategy", {})
            source_scopes_data = tidy_data_without_fix_strategy.pop("source_scopes", {})
            tidy_cfg = TidyConfig(
                **self._filter_dataclass_kwargs(TidyConfig, tidy_data_without_fix_strategy)
            )
            if isinstance(fix_strategy_data, dict):
                tidy_cfg.fix_strategy = TidyFixStrategyConfig(
                    **self._filter_dataclass_kwargs(TidyFixStrategyConfig, fix_strategy_data)
                )
            if isinstance(source_scopes_data, dict):
                parsed_source_scopes: dict[str, TidySourceScopeConfig] = {}
                for scope_name, scope_meta in source_scopes_data.items():
                    if not isinstance(scope_name, str) or not isinstance(scope_meta, dict):
                        continue
                    parsed_source_scopes[scope_name] = TidySourceScopeConfig(
                        **self._filter_dataclass_kwargs(TidySourceScopeConfig, scope_meta)
                    )
                tidy_cfg.source_scopes = parsed_source_scopes
            rename_data = data.get("rename", {})
            return AgentConfig(
                apps=apps_data,
                build=build_cfg,
                quality=quality_cfg,
                tidy=tidy_cfg,
                rename=RenameConfig(
                    **self._filter_dataclass_kwargs(RenameConfig, rename_data)
                ),
            )
        except Exception as e:
            print(
                "Warning: Failed to load config from tools/toolchain/config/*.toml "
                f": {e}. Using defaults."
            )
            return AgentConfig()

    @staticmethod
    def _filter_dataclass_kwargs(cls_type, raw_data: dict) -> dict:
        if not isinstance(raw_data, dict):
            return {}
        allowed = {item.name for item in fields(cls_type)}
        return {key: value for key, value in raw_data.items() if key in allowed}

    @classmethod
    def _merge_profile_meta(cls, base: dict, override: dict) -> dict:
        merged = dict(base)
        for key, value in override.items():
            if key in {"extends", "inherits"}:
                continue
            if isinstance(value, list):
                inherited = merged.get(key, [])
                if isinstance(inherited, list):
                    merged[key] = [*inherited, *value]
                else:
                    merged[key] = list(value)
                continue
            if value is None:
                continue
            if isinstance(value, str) and value == "" and key in merged:
                continue
            merged[key] = value
        return merged

    @classmethod
    def _resolve_build_profiles(cls, profiles_payload: dict) -> dict[str, BuildProfileConfig]:
        raw_profiles: dict[str, dict] = {}
        for profile_name, profile_meta in profiles_payload.items():
            if not isinstance(profile_name, str):
                continue
            if not isinstance(profile_meta, dict):
                continue
            raw_profiles[profile_name] = dict(profile_meta)

        resolved_meta: dict[str, dict] = {}
        resolving: set[str] = set()

        def resolve_one(profile_name: str) -> dict:
            if profile_name in resolved_meta:
                return resolved_meta[profile_name]
            if profile_name in resolving:
                raise ValueError(
                    f"build profile inheritance cycle detected at `{profile_name}`"
                )
            profile_meta = raw_profiles.get(profile_name)
            if profile_meta is None:
                raise ValueError(f"build profile `{profile_name}` not found")

            resolving.add(profile_name)
            inherited_name = str(
                profile_meta.get("extends") or profile_meta.get("inherits") or ""
            ).strip()
            merged: dict = {}
            if inherited_name:
                if inherited_name not in raw_profiles:
                    raise ValueError(
                        f"build profile `{profile_name}` extends unknown profile `{inherited_name}`"
                    )
                merged = cls._merge_profile_meta(merged, resolve_one(inherited_name))
            merged = cls._merge_profile_meta(merged, profile_meta)
            merged["extends"] = inherited_name
            resolving.remove(profile_name)
            resolved_meta[profile_name] = merged
            return merged

        for profile_name in raw_profiles.keys():
            resolve_one(profile_name)

        parsed: dict[str, BuildProfileConfig] = {}
        for profile_name, profile_meta in resolved_meta.items():
            parsed[profile_name] = BuildProfileConfig(
                **cls._filter_dataclass_kwargs(BuildProfileConfig, profile_meta)
            )
        return parsed

    def _load_raw_config_data(self) -> dict:
        split_dir = self.repo_root / "tools" / "toolchain" / "config"

        merged: dict = {}

        if split_dir.is_dir():
            for fragment_path in sorted(split_dir.glob("*.toml")):
                fragment_data = self._load_toml_file(fragment_path)
                merged = self._deep_merge_dicts(merged, fragment_data)

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

    def get_app_source_dir(self, app_name: str) -> Path:
        app = self.get_app_metadata(app_name)
        source_path_raw = (getattr(app, "cmake_source_path", "") or "").strip()
        if source_path_raw:
            source_path = Path(source_path_raw)
            return source_path if source_path.is_absolute() else self.repo_root / source_path
        return self.get_app_dir(app_name)

    def get_app_metadata(self, app_name: str) -> AppConfig:
        if app_name not in self.config.apps:
            # Return a default if not found
            return AppConfig(path=str(self.get_app_dir(app_name)))
        return self.config.apps[app_name]

    def get_out_root(self) -> Path:
        return resolve_out_root(self.repo_root)

    def get_build_layout(self, app_name: str, build_dir_name: str):
        return resolve_build_layout(self.repo_root, app_name, build_dir_name)

    def get_build_dir(self, app_name: str, build_dir_name: str) -> Path:
        return self.get_build_layout(app_name, build_dir_name).root

    def get_tidy_layout(self, app_name: str, tidy_build_dir_name: str):
        return resolve_tidy_layout(self.repo_root, app_name, tidy_build_dir_name)

    def get_tidy_dir(self, app_name: str, tidy_build_dir_name: str) -> Path:
        return self.get_tidy_layout(app_name, tidy_build_dir_name).root

    def get_analyze_layout(self, app_name: str, analyze_workspace_name: str):
        return resolve_analyze_layout(self.repo_root, app_name, analyze_workspace_name)

    def get_analyze_dir(self, app_name: str, analyze_workspace_name: str) -> Path:
        return self.get_analyze_layout(app_name, analyze_workspace_name).root

    def get_test_result_layout(self, result_target: str):
        return resolve_test_result_layout(self.repo_root, result_target)

    def get_test_result_layout_for_app(self, app_name: str):
        return resolve_test_result_layout_for_app(self.repo_root, app_name)

    def get_validation_layout(self, run_name: str):
        return resolve_validation_layout(self.repo_root, run_name)

    def setup_env(self):
        """Setup specialized environment for toolchain."""
        env = os.environ.copy()
        extra_paths = [r"C:\msys64\ucrt64\bin", r"C:\msys64\usr\bin"]
        env["PATH"] = os.pathsep.join(extra_paths + [env.get("PATH", "")])
        return env
