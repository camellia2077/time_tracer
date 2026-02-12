from dataclasses import dataclass, field

@dataclass
class AppConfig:
    path: str
    default_tidy: bool = True
    cmake_flags: list[str] = field(default_factory=list)

@dataclass
class BuildConfig:
    compiler: str = "default"
    c_compiler: str | None = None
    cxx_compiler: str | None = None

@dataclass
class TidyConfig:
    max_lines: int = 100
    max_diags: int = 10
    batch_size: int = 50
    jobs: int = 0
    parse_workers: int = 0
    keep_going: bool = True
    run_fix_before_tidy: bool = True
    fix_limit: int = 0

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
    allowed_kinds: list[str] = field(default_factory=lambda: [
        "constant",
        "variable",
        "function",
        "method",
        "parameter",
        "member",
        "class member",
        "private member",
        "protected member",
    ])

@dataclass
class AgentConfig:
    apps: dict[str, AppConfig] = field(default_factory=dict)
    build: BuildConfig = field(default_factory=BuildConfig)
    tidy: TidyConfig = field(default_factory=TidyConfig)
    rename: RenameConfig = field(default_factory=RenameConfig)
