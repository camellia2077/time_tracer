import json
import struct
from pathlib import Path


def to_m(size_bytes: int) -> float:
    # Use decimal mega unit for concise "m" output.
    return round(size_bytes / 1_000_000.0, 6)


def _read_pe_sections(artifact: Path, tracked_section_names: set[str]) -> list[dict[str, object]]:
    try:
        binary = artifact.read_bytes()
    except OSError:
        return []

    if len(binary) < 0x40 or binary[0:2] != b"MZ":
        return []

    pe_offset = struct.unpack_from("<I", binary, 0x3C)[0]
    if pe_offset + 24 > len(binary):
        return []
    if binary[pe_offset : pe_offset + 4] != b"PE\x00\x00":
        return []

    number_of_sections = struct.unpack_from("<H", binary, pe_offset + 6)[0]
    optional_header_size = struct.unpack_from("<H", binary, pe_offset + 20)[0]
    section_table_offset = pe_offset + 24 + optional_header_size

    sections: list[dict[str, object]] = []
    for index in range(number_of_sections):
        header_offset = section_table_offset + index * 40
        if header_offset + 40 > len(binary):
            break

        raw_name = binary[header_offset : header_offset + 8]
        section_name = raw_name.split(b"\x00", 1)[0].decode("ascii", errors="ignore")
        if section_name not in tracked_section_names:
            continue

        virtual_size = struct.unpack_from("<I", binary, header_offset + 8)[0]
        raw_size = struct.unpack_from("<I", binary, header_offset + 16)[0]
        sections.append(
            {
                "name": section_name,
                "raw_size_bytes": raw_size,
                "raw_size_m": to_m(raw_size),
                "virtual_size_bytes": virtual_size,
                "virtual_size_m": to_m(virtual_size),
            }
        )
    return sections


def _aggregate_pe_sections(artifacts: list[dict[str, object]]) -> list[dict[str, object]]:
    totals: dict[str, dict[str, int]] = {}
    for artifact in artifacts:
        for section in artifact.get("pe_sections", []):
            section_name = str(section.get("name", ""))
            if not section_name:
                continue
            if section_name not in totals:
                totals[section_name] = {"raw_size_bytes": 0, "virtual_size_bytes": 0}
            totals[section_name]["raw_size_bytes"] += int(section.get("raw_size_bytes", 0))
            totals[section_name]["virtual_size_bytes"] += int(section.get("virtual_size_bytes", 0))

    summary: list[dict[str, object]] = []
    for section_name in sorted(totals.keys()):
        raw_size = totals[section_name]["raw_size_bytes"]
        virtual_size = totals[section_name]["virtual_size_bytes"]
        summary.append(
            {
                "name": section_name,
                "raw_size_bytes": raw_size,
                "raw_size_m": to_m(raw_size),
                "virtual_size_bytes": virtual_size,
                "virtual_size_m": to_m(virtual_size),
            }
        )
    return summary


def collect_artifact_sizes(
    bin_dir: Path,
    artifact_glob: str,
    exclude_substrings: list[str] | None = None,
    tracked_pe_sections: list[str] | None = None,
) -> tuple[list[dict[str, object]], int]:
    if not bin_dir.exists():
        raise FileNotFoundError(f"build output dir not found: {bin_dir}")

    excluded = [item.lower() for item in (exclude_substrings or []) if item]
    tracked_sections = {
        item.strip() for item in (tracked_pe_sections or []) if item and item.strip()
    }
    artifacts: list[dict[str, object]] = []
    total_bytes = 0

    for artifact in sorted(path for path in bin_dir.glob(artifact_glob) if path.is_file()):
        name_lower = artifact.name.lower()
        if any(token in name_lower for token in excluded):
            continue

        size_bytes = artifact.stat().st_size
        total_bytes += size_bytes
        item = {"name": artifact.name, "bytes": size_bytes, "m": to_m(size_bytes)}
        if tracked_sections:
            item["pe_sections"] = _read_pe_sections(artifact, tracked_sections)
        artifacts.append(item)

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
        "pe_sections_total": _aggregate_pe_sections(artifacts),
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
