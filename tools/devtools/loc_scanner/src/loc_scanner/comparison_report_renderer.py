def build_comparison_markdown(comparison: dict[str, object]) -> str:
    summary = comparison["summary"]
    lines = [
        "# LOC Scanner Context Delta", "",
        f"- Baseline: `{comparison['baseline']}`",
        f"- Current: `{comparison['current']}`", "",
        "## Summary", "",
        f"- Baseline matched files: `{summary['baseline_matched_files']}`",
        f"- Current matched files: `{summary['current_matched_files']}`",
        f"- Added: `{summary['added_files']}`",
        f"- Removed: `{summary['removed_files']}`",
        f"- Line-count changed: `{summary['changed_files']}`",
        f"- Net matched files: `{summary['net_matched_files']}`", "",
    ]
    append_delta_section(lines, "Added hotspots", comparison["added"])
    append_delta_section(lines, "Removed hotspots", comparison["removed"])
    lines.extend(["## Changed hotspots", ""])
    changed = comparison["changed"]
    if not changed:
        lines.extend(["No matched hotspot changed line count.", ""])
    else:
        lines.extend([
            "| File | Before | After | Delta |",
            "| --- | ---: | ---: | ---: |",
        ])
        for item in changed:
            lines.append(
                f"| `{item['path']}` | {item['before_lines']} | "
                f"{item['after_lines']} | {item['delta_lines']:+d} |"
            )
        lines.append("")
    return "\n".join(lines)


def append_delta_section(
    lines: list[str],
    title: str,
    findings: list[dict[str, object]],
) -> None:
    lines.extend([f"## {title}", ""])
    if not findings:
        lines.extend(["None.", ""])
        return
    lines.extend([
        "| File | Component | Language | Lines |",
        "| --- | --- | --- | ---: |",
    ])
    for item in findings:
        lines.append(
            f"| `{item['path']}` | `{item['component']}` | "
            f"`{item['language']}` | {item['lines']} |"
        )
    lines.append("")
