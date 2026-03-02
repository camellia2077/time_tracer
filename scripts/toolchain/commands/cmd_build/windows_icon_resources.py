import os
import shutil
import subprocess
from pathlib import Path

from ...core.context import Context


def _env_truthy(name: str) -> bool:
    value = (os.getenv(name) or "").strip().lower()
    return value in {"1", "true", "yes", "on"}


def should_enable_windows_cli_icon(app_name: str, profile_name: str | None) -> bool:
    if os.name != "nt":
        return False
    if app_name != "tracer_windows_rust_cli":
        return False
    # Explicit manual off-switch for any environment.
    if _env_truthy("TT_WINDOWS_CLI_ICON_DISABLE"):
        return False
    # Hard-disable in CI: icon generation depends on host tools (e.g. rsvg-convert)
    # that may not exist on runners, and should not block release verification.
    if _env_truthy("GITHUB_ACTIONS") or _env_truthy("CI"):
        return False
    profile_key = (profile_name or "").strip().lower()
    # CI runners may not provide `rsvg-convert`; skip icon embedding for *_ci* profiles
    # to keep release verification focused on runtime correctness instead of host tooling.
    if "ci" in profile_key:
        return False
    return "release" in profile_key


def _build_png_ico_payload(png_bytes: bytes) -> bytes:
    # ICO can embed PNG directly. A 256x256 entry is encoded as width/height=0.
    icon_dir = (
        (0).to_bytes(2, byteorder="little")
        + (1).to_bytes(2, byteorder="little")
        + (1).to_bytes(2, byteorder="little")
    )
    image_offset = 6 + 16
    icon_entry = (
        bytes([0, 0, 0, 0])
        + (1).to_bytes(2, byteorder="little")
        + (32).to_bytes(2, byteorder="little")
        + len(png_bytes).to_bytes(4, byteorder="little")
        + image_offset.to_bytes(4, byteorder="little")
    )
    return icon_dir + icon_entry + png_bytes


def _resolve_icon_ico_override_from_env() -> Path | None:
    raw_path = (os.getenv("TT_WINDOWS_CLI_ICON_ICO") or "").strip()
    if not raw_path:
        return None
    resolved = Path(raw_path).expanduser().resolve()
    if not resolved.exists():
        print(f"Error: TT_WINDOWS_CLI_ICON_ICO does not exist: {resolved}")
        return None
    return resolved


def _resolve_source_svg_path(ctx: Context, svg_override: str | None) -> Path:
    if svg_override and svg_override.strip():
        return Path(svg_override.strip()).expanduser().resolve()
    raw_env_svg = (os.getenv("TT_WINDOWS_CLI_ICON_SVG") or "").strip()
    if raw_env_svg:
        return Path(raw_env_svg).expanduser().resolve()
    return (
        ctx.repo_root
        / "apps"
        / "shared"
        / "branding"
        / "sharp_rounded_white.svg"
    ).resolve()


def _generate_windows_cli_icon_from_svg(
    ctx: Context,
    app_dir: Path,
    build_dir_name: str,
    svg_override: str | None,
) -> Path | None:
    source_svg = _resolve_source_svg_path(ctx=ctx, svg_override=svg_override)
    if not source_svg.exists():
        print(f"Error: source SVG icon not found: {source_svg}")
        return None

    output_dir = app_dir / build_dir_name / "resources"
    output_dir.mkdir(parents=True, exist_ok=True)
    output_png = output_dir / "time_tracer_cli.icon.256.png"
    output_ico = output_dir / "time_tracer_cli.ico"

    cached_icon = output_ico.exists()
    if shutil.which("rsvg-convert") is None:
        if cached_icon:
            print(
                "Warning: `rsvg-convert` not found; fallback to cached icon: "
                f"{output_ico}"
            )
            return output_ico
        print(
            "Error: `rsvg-convert` not found. Please install librsvg "
            "(or set TT_WINDOWS_CLI_ICON_ICO to a valid .ico path)."
        )
        return None

    completed = subprocess.run(
        [
            "rsvg-convert",
            "-w",
            "256",
            "-h",
            "256",
            "-o",
            str(output_png),
            str(source_svg),
        ],
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        if completed.stdout.strip():
            print(completed.stdout.strip())
        if completed.stderr.strip():
            print(completed.stderr.strip())
        print(f"Error: failed to rasterize SVG icon: {source_svg}")
        if cached_icon:
            print(f"Warning: fallback to cached icon: {output_ico}")
            return output_ico
        return None

    try:
        png_bytes = output_png.read_bytes()
    except OSError as error:
        print(f"Error: failed to read rasterized PNG icon: {error}")
        if cached_icon:
            print(f"Warning: fallback to cached icon: {output_ico}")
            return output_ico
        return None
    finally:
        try:
            output_png.unlink()
        except OSError:
            pass

    if not png_bytes.startswith(b"\x89PNG\r\n\x1a\n"):
        print(f"Error: rasterized icon is not a valid PNG: {output_png}")
        if cached_icon:
            print(f"Warning: fallback to cached icon: {output_ico}")
            return output_ico
        return None

    try:
        output_ico.write_bytes(_build_png_ico_payload(png_bytes))
    except OSError as error:
        print(f"Error: failed to write ICO icon: {error}")
        return None

    print(f"--- build: generated Windows icon: {output_ico}")
    return output_ico


def prepare_windows_cli_icon_ico(
    ctx: Context,
    app_name: str,
    app_dir: Path,
    build_dir_name: str,
    profile_name: str | None,
    svg_override: str | None = None,
) -> Path | None:
    if not should_enable_windows_cli_icon(app_name=app_name, profile_name=profile_name):
        return None
    icon_override = _resolve_icon_ico_override_from_env()
    if icon_override is not None:
        print(f"--- build: using TT_WINDOWS_CLI_ICON_ICO override: {icon_override}")
        return icon_override
    return _generate_windows_cli_icon_from_svg(
        ctx=ctx,
        app_dir=app_dir,
        build_dir_name=build_dir_name,
        svg_override=svg_override,
    )
