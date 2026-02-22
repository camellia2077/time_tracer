from pathlib import Path

from ...core.context import Context
from . import config_migrate_paths, config_migrate_planner, config_migrate_report


class ConfigMigrateCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        config_root_arg: str | None = None,
        profile: str | None = None,
        apply_changes: bool = False,
        dry_run: bool = False,
        rollback: bool = False,
        show_diff: bool = False,
    ) -> int:
        if apply_changes and rollback:
            print("Error: --apply and --rollback cannot be used together.")
            return 2
        if apply_changes and dry_run:
            print("Error: --apply and --dry-run cannot be used together.")
            return 2

        config_root = config_migrate_paths.resolve_config_root(self.ctx, app_name, config_root_arg)
        source_config_path = config_root / "config.toml"
        bundle_path = config_root / "meta" / "bundle.toml"
        backup_path = bundle_path.with_suffix(bundle_path.suffix + ".bak")

        if rollback:
            return self._execute_rollback(
                bundle_path=bundle_path,
                backup_path=backup_path,
                dry_run=dry_run,
            )

        if not source_config_path.exists():
            print(f"Error: source config not found: {source_config_path}")
            return 1

        effective_profile = config_migrate_paths.resolve_profile(profile, app_name)
        model = config_migrate_planner.build_bundle_model(
            config_root,
            source_config_path,
            effective_profile,
        )
        generated_text = config_migrate_report.render_bundle_toml(model)

        exists_before = bundle_path.exists()
        old_text = bundle_path.read_text(encoding="utf-8") if exists_before else ""

        up_to_date = exists_before and old_text == generated_text
        mode = "apply" if apply_changes else "dry-run"
        report_formats = ", ".join(model["paths"]["reports"].keys())
        print(f"--- config-migrate mode: {mode}")
        print(f"--- source: {source_config_path}")
        print(f"--- target: {bundle_path}")
        print(f"--- profile: {effective_profile}")
        print(f"--- report formats: {report_formats}")
        print(f"--- required files: {len(model['file_list']['required'])}")
        print(f"--- optional files: {len(model['file_list']['optional'])}")

        if up_to_date:
            print("--- bundle.toml is already up-to-date (idempotent).")
            return 0

        action = "create" if not exists_before else "update"
        print(f"--- bundle.toml would {action}.")
        if show_diff:
            config_migrate_report.print_diff(
                old_text if exists_before else "",
                generated_text,
                f"{bundle_path} (current)",
                f"{bundle_path} (generated)",
            )

        if not apply_changes:
            return 0

        return self._write_bundle(
            app_name=app_name,
            bundle_path=bundle_path,
            backup_path=backup_path,
            exists_before=exists_before,
            old_text=old_text,
            generated_text=generated_text,
        )

    def _execute_rollback(self, bundle_path: Path, backup_path: Path, dry_run: bool) -> int:
        if not backup_path.exists():
            print(f"Error: rollback backup not found: {backup_path}")
            return 1
        if dry_run:
            print(f"--- config-migrate rollback dry-run: {backup_path} -> {bundle_path}")
            return 0
        bundle_path.parent.mkdir(parents=True, exist_ok=True)
        bundle_path.write_text(backup_path.read_text(encoding="utf-8"), encoding="utf-8")
        print(f"--- config-migrate rollback applied: restored {bundle_path}")
        return 0

    def _write_bundle(
        self,
        app_name: str,
        bundle_path: Path,
        backup_path: Path,
        exists_before: bool,
        old_text: str,
        generated_text: str,
    ) -> int:
        bundle_path.parent.mkdir(parents=True, exist_ok=True)
        if exists_before:
            backup_path.write_text(old_text, encoding="utf-8")
            print(f"--- backup written: {backup_path}")
        bundle_path.write_text(generated_text, encoding="utf-8")
        print(f"--- bundle written: {bundle_path}")
        print(
            "--- rollback option: `python scripts/run.py config-migrate --app "
            f"{app_name} --rollback`"
        )
        return 0
