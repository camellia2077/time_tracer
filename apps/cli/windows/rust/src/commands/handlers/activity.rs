use std::fs;
use std::path::{Path, PathBuf};

use serde_json::json;

use crate::cli::{ActivityArgs, ActivityCommand, ActivityMergeArgs};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::commands::handlers::alias::{migrate_alias_sources, plan_hierarchy_operation};
use crate::error::AppError;

pub struct ActivityHandler;

impl CommandHandler<ActivityArgs> for ActivityHandler {
    fn handle(&self, args: ActivityArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            ActivityCommand::Merge(args) => merge_activity(args, ctx),
        }
    }
}

fn merge_activity(args: ActivityMergeArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let original_toml = fs::read_to_string(&path)
        .map_err(|e| AppError::Io(format!("Read canonical TOML failed: {e}")))?;
    let planned = plan_hierarchy_operation(
        &original_toml,
        ctx,
        json!({
            "kind": "merge_leaf_canonical",
            "target_path": args.from,
            "destination_path": args.into,
        }),
    )?;
    let updated_files = migrate_alias_sources(
        &path,
        &original_toml,
        &planned.updated_toml_content,
        Path::new(&args.input),
        &planned.replacements,
        &planned.alias_replacements,
        ctx,
    )?;
    let replacement = planned.replacements.first().ok_or_else(|| {
        AppError::Logic("Core merge_leaf_canonical did not return a canonical replacement.".into())
    })?;
    println!(
        "Merged activity from `{}` into `{}`; rebuilt database and updated {} TXT file(s).",
        replacement.old_canonical, replacement.new_canonical, updated_files
    );
    Ok(())
}
