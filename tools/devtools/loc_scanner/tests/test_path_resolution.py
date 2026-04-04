import argparse
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from loc_scanner.cli_app import LocCliApplication
from loc_scanner.service import ScanArgumentResolver


def test_resolve_paths_with_workspace_root(tmp_path: Path) -> None:
    resolver = ScanArgumentResolver()
    workspace_root = tmp_path / "workspace"
    workspace_root.mkdir(parents=True, exist_ok=True)

    paths = resolver.resolve_paths(
        raw_paths=["src", str(tmp_path)],
        default_paths=["."],
        workspace_root=workspace_root,
    )

    assert paths[0] == (workspace_root / "src").resolve()
    assert paths[1] == tmp_path.resolve()


def test_default_log_path_uses_workspace_root(tmp_path: Path) -> None:
    log_path = LocCliApplication._resolve_log_path(
        None,
        "py",
        workspace_root=tmp_path,
    )
    expected = (tmp_path / ".loc_scanner_logs" / "scan_py.json").resolve()
    assert log_path == expected


def test_build_base_payload_contains_workspace_root(tmp_path: Path) -> None:
    args = argparse.Namespace(lang="py")
    payload = LocCliApplication._build_base_payload(args=args, workspace_root=tmp_path)
    assert payload["lang"] == "py"
    assert payload["workspace_root"] == str(tmp_path)
