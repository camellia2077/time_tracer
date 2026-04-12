#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import secrets
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[4]
ANDROID_ROOT = REPO_ROOT / "apps" / "android"
ANDROID_APP_ROOT = ANDROID_ROOT / "app"
DEFAULT_KEYSTORE_NAME = "tracer-release.jks"
DEFAULT_KEY_ALIAS = "tracer-release"
DEFAULT_VALIDITY_DAYS = 365 * 25
DEFAULT_DNAME = "CN=Time Tracer Release, OU=Engineering, O=Time Tracer, L=Shanghai, ST=Shanghai, C=CN"


@dataclass(frozen=True)
class SigningConfig:
    store_file: Path
    store_password: str
    key_alias: str
    key_password: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate an Android release keystore and write local signing config."
    )
    parser.add_argument(
        "--keystore-path",
        default=str(ANDROID_APP_ROOT / DEFAULT_KEYSTORE_NAME),
        help="Absolute or repo-relative path to the output JKS file.",
    )
    parser.add_argument(
        "--store-password",
        help="Keystore password. If omitted, uses TT_ANDROID_RELEASE_STORE_PASSWORD or generates one with --generate-passwords.",
    )
    parser.add_argument(
        "--key-password",
        help="Key password. If omitted, uses TT_ANDROID_RELEASE_KEY_PASSWORD or matches store password.",
    )
    parser.add_argument(
        "--key-alias",
        default=DEFAULT_KEY_ALIAS,
        help=f"Key alias to create (default: {DEFAULT_KEY_ALIAS}).",
    )
    parser.add_argument(
        "--validity-days",
        type=int,
        default=DEFAULT_VALIDITY_DAYS,
        help=f"Certificate validity in days (default: {DEFAULT_VALIDITY_DAYS}).",
    )
    parser.add_argument(
        "--dname",
        default=DEFAULT_DNAME,
        help="Distinguished name passed to keytool -dname.",
    )
    parser.add_argument(
        "--keystore-properties-path",
        default=str(ANDROID_ROOT / "keystore.properties"),
        help="Path to write keystore.properties.",
    )
    parser.add_argument(
        "--write-keystore-properties",
        action="store_true",
        help="Write apps/android/keystore.properties after generating or validating the keystore.",
    )
    parser.add_argument(
        "--overwrite-keystore-properties",
        action="store_true",
        help="Allow overwriting an existing keystore.properties file.",
    )
    parser.add_argument(
        "--generate-passwords",
        action="store_true",
        help="Generate strong passwords when they are not provided.",
    )
    parser.add_argument(
        "--password-length",
        type=int,
        default=32,
        help="Generated password length (default: 32).",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Allow overwriting an existing keystore file.",
    )
    parser.add_argument(
        "--keytool",
        default="keytool",
        help="Path to keytool executable (default: keytool).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print planned actions without executing keytool or writing files.",
    )
    return parser.parse_args()


def resolve_path(raw_path: str) -> Path:
    path = Path(raw_path)
    if path.is_absolute():
        return path
    return (REPO_ROOT / path).resolve()


def generate_password(length: int) -> str:
    alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*()-_=+"
    return "".join(secrets.choice(alphabet) for _ in range(length))


def resolve_passwords(args: argparse.Namespace) -> SigningConfig:
    store_password = args.store_password or os.environ.get("TT_ANDROID_RELEASE_STORE_PASSWORD", "")
    key_password = args.key_password or os.environ.get("TT_ANDROID_RELEASE_KEY_PASSWORD", "")

    if args.generate_passwords:
        if not store_password:
            store_password = generate_password(args.password_length)
        if not key_password:
            key_password = generate_password(args.password_length)

    if not store_password:
        raise SystemExit(
            "Missing store password. Pass --store-password, set TT_ANDROID_RELEASE_STORE_PASSWORD, or use --generate-passwords."
        )

    if not key_password:
        key_password = store_password

    return SigningConfig(
        store_file=resolve_path(args.keystore_path),
        store_password=store_password,
        key_alias=args.key_alias,
        key_password=key_password,
    )


def ensure_parent_dir(path: Path, *, dry_run: bool) -> None:
    if dry_run:
        return
    path.parent.mkdir(parents=True, exist_ok=True)


def resolve_keytool_command(raw_keytool: str) -> str:
    resolved = shutil.which(raw_keytool)
    if resolved:
        return resolved
    raw_path = Path(raw_keytool)
    if raw_path.exists():
        return str(raw_path.resolve())
    raise SystemExit(
        f"keytool not found: {raw_keytool}. Install a JDK or pass --keytool with an explicit path."
    )


def build_keytool_command(
    *,
    keytool: str,
    config: SigningConfig,
    validity_days: int,
    dname: str,
) -> list[str]:
    return [
        keytool,
        "-genkeypair",
        "-v",
        "-keystore",
        str(config.store_file),
        "-alias",
        config.key_alias,
        "-keyalg",
        "RSA",
        "-keysize",
        "4096",
        "-validity",
        str(validity_days),
        "-storepass",
        config.store_password,
        "-keypass",
        config.key_password,
        "-dname",
        dname,
    ]


def create_keystore(
    *,
    config: SigningConfig,
    keytool: str,
    validity_days: int,
    dname: str,
    force: bool,
    dry_run: bool,
) -> bool:
    if config.store_file.exists():
        if not force:
            print(f"--- keystore exists, keeping current file: {config.store_file}")
            return False
        if not dry_run:
            config.store_file.unlink()

    ensure_parent_dir(config.store_file, dry_run=dry_run)
    command = build_keytool_command(
        keytool=keytool,
        config=config,
        validity_days=validity_days,
        dname=dname,
    )
    if dry_run:
        print("--- dry-run keytool command")
        print(" ".join(command))
        return True

    completed = subprocess.run(
        command,
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        raise SystemExit(
            "keytool failed with exit code "
            f"{completed.returncode}\nSTDOUT:\n{completed.stdout}\nSTDERR:\n{completed.stderr}"
        )
    print(f"--- created keystore: {config.store_file}")
    return True


def escape_properties_value(value: str) -> str:
    return value.replace("\\", "\\\\")


def build_keystore_properties_content(config: SigningConfig) -> str:
    return "\n".join(
        [
            f"STORE_FILE={escape_properties_value(str(config.store_file))}",
            f"STORE_PASSWORD={config.store_password}",
            f"KEY_ALIAS={config.key_alias}",
            f"KEY_PASSWORD={config.key_password}",
            "",
        ]
    )


def write_keystore_properties(
    *,
    config: SigningConfig,
    properties_path: Path,
    overwrite: bool,
    dry_run: bool,
) -> None:
    if properties_path.exists() and not overwrite:
        raise SystemExit(
            f"Refusing to overwrite existing keystore properties: {properties_path}. Use --overwrite-keystore-properties."
        )
    content = build_keystore_properties_content(config)
    if dry_run:
        print(f"--- dry-run write keystore properties: {properties_path}")
        print(content)
        return

    ensure_parent_dir(properties_path, dry_run=False)
    properties_path.write_text(content, encoding="utf-8")
    print(f"--- wrote keystore properties: {properties_path}")


def print_summary(*, config: SigningConfig, created_keystore: bool, wrote_properties: bool) -> None:
    print("--- android release signing summary")
    print(f"keystore: {config.store_file}")
    print(f"created: {'yes' if created_keystore else 'no'}")
    print(f"key alias: {config.key_alias}")
    print(f"wrote keystore.properties: {'yes' if wrote_properties else 'no'}")
    print("")
    print("Save these values securely before using this keystore for published builds:")
    print(f"STORE_PASSWORD={config.store_password}")
    print(f"KEY_ALIAS={config.key_alias}")
    print(f"KEY_PASSWORD={config.key_password}")


def main() -> int:
    args = parse_args()
    config = resolve_passwords(args)
    keytool = resolve_keytool_command(args.keytool)
    created_keystore = create_keystore(
        config=config,
        keytool=keytool,
        validity_days=args.validity_days,
        dname=args.dname,
        force=args.force,
        dry_run=args.dry_run,
    )

    wrote_properties = False
    if args.write_keystore_properties:
        write_keystore_properties(
            config=config,
            properties_path=resolve_path(args.keystore_properties_path),
            overwrite=args.overwrite_keystore_properties,
            dry_run=args.dry_run,
        )
        wrote_properties = True

    print_summary(
        config=config,
        created_keystore=created_keystore,
        wrote_properties=wrote_properties,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
