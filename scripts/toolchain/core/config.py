from dataclasses import dataclass, field


@dataclass
class AppConfig:
    path: str
    backend: str = "cmake"
    default_tidy: bool = True
    cmake_flags: list[str] = field(default_factory=list)
    gradle_wrapper: str = ""
    gradle_tasks: list[str] = field(default_factory=list)
    config_sync_target: str = ""


@dataclass
class BuildProfileConfig:
    extends: str = ""
    description: str = ""
    build_dir: str = ""
    cmake_args: list[str] = field(default_factory=list)
    build_targets: list[str] = field(default_factory=list)
    gradle_tasks: list[str] = field(default_factory=list)
    gradle_args: list[str] = field(default_factory=list)
    cargo_args: list[str] = field(default_factory=list)


@dataclass
class BuildConfig:
    compiler: str = "default"
    c_compiler: str | None = None
    cxx_compiler: str | None = None
    default_profile: str = ""
    profiles: dict[str, BuildProfileConfig] = field(default_factory=dict)


@dataclass
class TidyFixStrategyConfig:
    auto_fix: list[str] = field(
        default_factory=lambda: [
            "modernize-use-nullptr",
            "modernize-use-override",
            "modernize-use-using",
            "modernize-loop-convert",
            "modernize-raw-string-literal",
            "readability-braces-around-statements",
            "readability-else-after-return",
            "readability-redundant-control-flow",
        ]
    )
    safe_refactor: list[str] = field(
        default_factory=lambda: [
            "bugprone-easily-swappable-parameters",
            "readability-function-cognitive-complexity",
            "bugprone-branch-clone",
            "performance-for-range-copy",
            "modernize-pass-by-value",
            "readability-identifier-naming",
        ]
    )
    nolint_allowed: list[str] = field(
        default_factory=lambda: [
            "readability-function-cognitive-complexity",
            "bugprone-easily-swappable-parameters",
            "cppcoreguidelines-avoid-magic-numbers",
        ]
    )
    manual_only: list[str] = field(
        default_factory=lambda: [
            "clang-diagnostic-error",
            "clang-diagnostic-*",
            "clang-analyzer-*",
            "cppcoreguidelines-pro-type-reinterpret-cast",
            "concurrency-*",
        ]
    )


@dataclass
class TidyConfig:
    max_lines: int = 100
    max_diags: int = 10
    batch_size: int = 50
    jobs: int = 0
    parse_workers: int = 0
    keep_going: bool = True
    header_filter_regex: str = r"^(?!.*[\\/]_deps[\\/]).*"
    run_fix_before_tidy: bool = True
    fix_limit: int = 0
    auto_full_on_no_such_file: bool = True
    auto_full_on_glob_mismatch: bool = True
    auto_full_on_high_already_renamed: bool = True
    auto_full_already_renamed_ratio: float = 0.6
    auto_full_already_renamed_min: int = 20
    fix_strategy: TidyFixStrategyConfig = field(default_factory=TidyFixStrategyConfig)


@dataclass
class QualityGateAuditConfig:
    # File extensions that should be text-normalized before gate compare.
    normalize_ext: list[str] = field(default_factory=lambda: [".md"])


@dataclass
class QualityConfig:
    gate_audit: QualityGateAuditConfig = field(default_factory=QualityGateAuditConfig)


@dataclass
class RenameConfig:
    engine: str = "clangd"
    check_name: str = "readability-identifier-naming"
    clangd_path: str = "clangd"
    clangd_background_index: bool = True
    clangd_warmup_seconds: float = 1.0
    skip_header_single_edit: bool = True
    max_candidates_per_run: int = 300
    dry_run_default: bool = False
    allowed_kinds: list[str] = field(
        default_factory=lambda: [
            "constant",
            "variable",
            "function",
            "method",
            "parameter",
            "member",
            "class member",
            "private member",
            "protected member",
        ]
    )


@dataclass
class PostChangeConfig:
    default_app: str = "tracer_core"
    default_build_dir: str = "build_fast"
    run_tests: str = "auto"
    script_changes: str = "build"


@dataclass
class AgentConfig:
    apps: dict[str, AppConfig] = field(default_factory=dict)
    build: BuildConfig = field(default_factory=BuildConfig)
    quality: QualityConfig = field(default_factory=QualityConfig)
    tidy: TidyConfig = field(default_factory=TidyConfig)
    rename: RenameConfig = field(default_factory=RenameConfig)
    post_change: PostChangeConfig = field(default_factory=PostChangeConfig)
