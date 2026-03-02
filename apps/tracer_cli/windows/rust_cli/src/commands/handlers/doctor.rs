use std::env;
use std::path::PathBuf;

use crate::cli::DoctorArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

pub struct DoctorHandler;

impl CommandHandler<DoctorArgs> for DoctorHandler {
    fn handle(&self, _args: DoctorArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let exe_path = env::current_exe()
            .map_err(|e| AppError::Io(format!("Resolve current exe failed: {e}")))?;
        let exe_dir = exe_path
            .parent()
            .map(PathBuf::from)
            .ok_or_else(|| AppError::Io("Cannot resolve executable directory".to_string()))?;
        let output_root = ctx
            .output_path
            .as_ref()
            .map(PathBuf::from)
            .unwrap_or_else(|| exe_dir.join("output"));
        let db_path = ctx
            .db_path
            .as_ref()
            .map(PathBuf::from)
            .unwrap_or_else(|| output_root.join("db").join("time_data.sqlite3"));

        println!("Runtime Doctor");
        println!("  exe_path   : {}", exe_path.display());
        println!("  exe_dir    : {}", exe_dir.display());
        println!("  output_root: {}", output_root.display());
        println!("  db_path    : {}", db_path.display());
        println!();

        let checks = vec![
            required_check("core_dll", exe_dir.join("tracer_core.dll")),
            required_check("reports_shared_dll", exe_dir.join("reports_shared.dll")),
            required_check("config_toml", exe_dir.join("config").join("config.toml")),
            required_check(
                "converter_alias_mapping",
                exe_dir
                    .join("config")
                    .join("converter")
                    .join("alias_mapping.toml"),
            ),
            optional_dir_check(
                "db_parent_dir",
                db_path.parent().unwrap_or(&output_root).to_path_buf(),
            ),
            optional_check("db_file", db_path.clone()),
        ];

        let mut required_total = 0;
        let mut required_passed = 0;
        for check in &checks {
            if check.required {
                required_total += 1;
                if check.ok {
                    required_passed += 1;
                }
            }
            let status = if check.ok { "[OK]" } else { "[MISS]" };
            let optional = if check.required { "" } else { " [optional]" };
            println!(
                "  {status:<6} {} -> {} ({}){}",
                check.id,
                check.path.display(),
                check.detail,
                optional
            );
        }
        println!();
        println!("  Summary: required {required_passed}/{required_total} passed.");

        let all_required_ok = required_passed == required_total;
        if all_required_ok {
            return Ok(());
        }

        if has_failure(&checks, "config_toml") || has_failure(&checks, "converter_alias_mapping") {
            return Err(AppError::Config(
                "Doctor check failed: missing required config files.".to_string(),
            ));
        }
        if has_failure(&checks, "core_dll") || has_failure(&checks, "reports_shared_dll") {
            return Err(AppError::DllCompatibility(
                "Doctor check failed: missing required runtime dependencies.".to_string(),
            ));
        }

        Err(AppError::Io(
            "Doctor check failed: runtime path checks are not healthy.".to_string(),
        ))
    }
}

struct Check {
    id: &'static str,
    required: bool,
    ok: bool,
    path: PathBuf,
    detail: String,
}

fn required_check(id: &'static str, path: PathBuf) -> Check {
    let ok = path.exists();
    Check {
        id,
        required: true,
        ok,
        path,
        detail: if ok { "present" } else { "missing" }.to_string(),
    }
}

fn optional_check(id: &'static str, path: PathBuf) -> Check {
    let ok = path.exists();
    Check {
        id,
        required: false,
        ok,
        path,
        detail: if ok { "present" } else { "missing" }.to_string(),
    }
}

fn optional_dir_check(id: &'static str, path: PathBuf) -> Check {
    let ok = path.is_dir();
    Check {
        id,
        required: false,
        ok,
        path,
        detail: if ok { "present" } else { "missing" }.to_string(),
    }
}

fn has_failure(checks: &[Check], id: &str) -> bool {
    checks.iter().any(|c| c.required && !c.ok && c.id == id)
}
