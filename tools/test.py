import sys
from pathlib import Path


def _resolve_tools_root() -> Path:
    return Path(__file__).resolve().parent


def _resolve_repo_root(tools_root: Path) -> Path:
    return tools_root.parent


def _resolve_suite_assets_root(tools_root: Path) -> Path:
    return tools_root / "suites"


def _resolve_test_root(repo_root: Path) -> Path:
    return repo_root / "test"


def _ensure_framework_path(tools_root: Path) -> None:
    framework_root = tools_root / "test_framework"
    framework_root_str = str(framework_root)
    if framework_root_str not in sys.path:
        sys.path.insert(0, framework_root_str)


def _ensure_repo_root_path(repo_root: Path) -> None:
    repo_root_str = str(repo_root)
    if repo_root_str not in sys.path:
        sys.path.insert(0, repo_root_str)


def _load_runner_modules():
    tools_root = _resolve_tools_root()
    repo_root = _resolve_repo_root(tools_root)
    suite_assets_root = _resolve_suite_assets_root(tools_root)
    test_root = _resolve_test_root(repo_root)
    _ensure_repo_root_path(repo_root)
    _ensure_framework_path(tools_root)
    from runner.commands import runtime_guard as runtime_guard_module
    from runner.commands import smoke_windows_cli as smoke_windows_cli_module
    from runner.commands import suite as suite_module

    return (
        repo_root,
        suite_assets_root,
        test_root,
        suite_module,
        runtime_guard_module,
        smoke_windows_cli_module,
    )


def _print_top_level_help() -> None:
    print("Unified test entry")
    print("Usage:")
    print("  python tools/test.py suite [suite args...]")
    print("  python tools/test.py runtime-guard [runtime-guard args...]")
    print("  python tools/test.py smoke-windows-cli [smoke args...]")
    print("")
    print("Subcommands:")
    print("  suite          Run executable suites.")
    print("  runtime-guard  Run reusable runtime guard scenarios.")
    print("  smoke-windows-cli  Run windows CLI smoke flow (suite + runtime guard).")
    print("")
    print("Examples:")
    print(
        "  python tools/test.py suite --suite artifact_windows_cli --build-dir build_fast --concise"
    )
    print("  python tools/test.py suite --suite artifact_log_generator --build-dir build_fast")
    print("  python tools/test.py runtime-guard --build-dir build_fast")
    print("  python tools/test.py smoke-windows-cli --build-dir build_fast")


def main(argv=None):
    (
        repo_root,
        suite_assets_root,
        test_root,
        suite_module,
        runtime_guard_module,
        smoke_windows_cli_module,
    ) = (
        _load_runner_modules()
    )
    raw_argv = list(sys.argv[1:] if argv is None else argv)

    if raw_argv and raw_argv[0] in {"-h", "--help", "help"}:
        _print_top_level_help()
        return 0

    if not raw_argv:
        print("Error: missing subcommand. Use `suite` / `runtime-guard` / `smoke-windows-cli`.")
        _print_top_level_help()
        return 2

    if raw_argv and raw_argv[0] == "suite":
        return suite_module.main(
            raw_argv[1:],
            suite_assets_root=suite_assets_root,
            test_root=test_root,
            repo_root=repo_root,
        )

    if raw_argv and raw_argv[0] == "runtime-guard":
        return runtime_guard_module.main(raw_argv[1:], repo_root=repo_root)

    if raw_argv and raw_argv[0] == "smoke-windows-cli":
        return smoke_windows_cli_module.main(raw_argv[1:], repo_root=repo_root)

    print(
        f"Error: unknown subcommand `{raw_argv[0]}`. "
        "Use `suite` / `runtime-guard` / `smoke-windows-cli`."
    )
    _print_top_level_help()
    return 2


if __name__ == "__main__":
    sys.exit(main())
