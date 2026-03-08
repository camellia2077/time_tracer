#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import tomllib
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate GitHub Actions windows matrix from toolchain build config."
    )
    parser.add_argument(
        "--config",
        default="tools/toolchain/config/build.toml",
        help="Path to build.toml (default: tools/toolchain/config/build.toml).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    config_path = Path(args.config)
    if not config_path.is_file():
        print(f"Error: config file not found: {config_path}")
        return 2

    with config_path.open("rb") as handle:
        payload = tomllib.load(handle)

    build = payload.get("build")
    if not isinstance(build, dict):
        print("Error: missing [build] section.")
        return 2
    profiles = build.get("profiles")
    if not isinstance(profiles, dict):
        print("Error: missing [build.profiles] section.")
        return 2

    ci = build.get("ci")
    if not isinstance(ci, dict):
        print("Error: missing [build.ci] section.")
        return 2
    rows = ci.get("windows_matrix")
    if not isinstance(rows, list):
        print("Error: missing [[build.ci.windows_matrix]] rows.")
        return 2

    include: list[dict] = []
    for index, row in enumerate(rows, start=1):
        if not isinstance(row, dict):
            print(f"Error: windows_matrix row #{index} must be a table.")
            return 2
        profile = str(row.get("profile", "")).strip()
        if not profile:
            print(f"Error: windows_matrix row #{index} missing `profile`.")
            return 2
        profile_cfg = profiles.get(profile)
        if not isinstance(profile_cfg, dict):
            print(
                f"Error: windows_matrix row #{index} references unknown profile `{profile}`."
            )
            return 2

        build_dir = str(row.get("build_dir", "")).strip() or str(
            profile_cfg.get("build_dir", "")
        ).strip()
        if not build_dir:
            print(
                f"Error: windows_matrix row #{index} missing build_dir and profile `{profile}` has no build_dir."
            )
            return 2

        include.append(
            {
                "job_name": str(row.get("job_name", profile)).strip() or profile,
                "profile": profile,
                "build_dir": build_dir,
                "cmake_args": str(row.get("cmake_args", "")).strip(),
                "allow_failure": bool(row.get("allow_failure", False)),
            }
        )

    print(json.dumps({"include": include}, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
