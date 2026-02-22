import json
from pathlib import Path


def to_m(size_bytes: int) -> float:
    # Use decimal mega unit for concise "m" output.
    return round(size_bytes / 1_000_000.0, 6)


def collect_artifact_sizes(
    bin_dir: Path,
    artifact_glob: str,
    exclude_substrings: list[str] | None = None,
) -> tuple[list[dict[str, object]], int]:
    if not bin_dir.exists():
        raise FileNotFoundError(f"build output dir not found: {bin_dir}")

    excluded = [item.lower() for item in (exclude_substrings or []) if item]
    artifacts: list[dict[str, object]] = []
    total_bytes = 0

    for artifact in sorted(path for path in bin_dir.glob(artifact_glob) if path.is_file()):
        name_lower = artifact.name.lower()
        if any(token in name_lower for token in excluded):
            continue

        size_bytes = artifact.stat().st_size
        total_bytes += size_bytes
        artifacts.append(
            {
                "name": artifact.name,
                "bytes": size_bytes,
                "m": to_m(size_bytes),
            }
        )

    return artifacts, total_bytes


def write_artifact_size_json(
    output_path: Path,
    scope: str,
    bin_dir: Path,
    artifacts: list[dict[str, object]],
    total_bytes: int,
) -> None:
    payload = {
        "scope": scope,
        "bin_dir": str(bin_dir.resolve()),
        "units": {"bytes": "bytes", "m": "m"},
        "artifacts": artifacts,
        "total": {
            "bytes": total_bytes,
            "m": to_m(total_bytes),
        },
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
