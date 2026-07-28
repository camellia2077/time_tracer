use std::fs;
use std::path::{Path, PathBuf};

use serde_json::json;

use crate::cli::{
    AliasAddArgs, AliasArgs, AliasCommand, AliasFileArgs, AliasGroupArgs, AliasMoveArgs,
    AliasMoveConfigArgs, AliasRenameGroupArgs, AliasTreeArgs,
};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct AliasHandler;

impl CommandHandler<AliasArgs> for AliasHandler {
    fn handle(&self, args: AliasArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            AliasCommand::Add(args) => add_alias(args, ctx),
            AliasCommand::Promote(args) => promote(args, ctx),
            AliasCommand::Move(args) => move_alias(args, ctx),
            AliasCommand::RenameGroup(args) => rename_group(args, ctx),
            AliasCommand::MoveConfig(args) => move_config(args, ctx),
            AliasCommand::Tree(args) => render_tree(args, ctx),
            AliasCommand::RenameGroupAlias(args) => group_alias(args, true, ctx),
            AliasCommand::AddGroupAlias(args) => group_alias(args, false, ctx),
        }
    }
}

fn render_tree(args: AliasTreeArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let toml_content = fs::read_to_string(&path)
        .map_err(|error| AppError::Io(format!("Read alias TOML {} failed: {error}", path.display())))?;
    let core = CoreApi::load()?;
    let runtime = core.bootstrap("alias-tree", &ctx.without_output())?;
    print!("{}", runtime.alias_hierarchy().render_text(&json!({
        "action": "render_alias_hierarchy_text",
        "toml_content": toml_content,
        "show_aliases": args.show_aliases,
    }))?);
    Ok(())
}

fn add_alias(args: AliasAddArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    apply_hierarchy_operation(
        &path,
        ctx,
        json!({
            "kind": "append_leaf_alias",
            "target_path": args.group,
            "canonical_key": args.canonical,
            "aliases": [args.alias],
        }),
    )?;
    println!(
        "Added alias `{}` to canonical `{}` in `{}`.",
        args.alias, args.canonical, args.group
    );
    Ok(())
}

fn plan_hierarchy_operation(
    toml_content: &str,
    ctx: &CommandContext,
    operation: serde_json::Value,
) -> Result<crate::core::runtime::AliasHierarchyOperationOutput, AppError> {
    let api = CoreApi::load()?;
    let runtime = api.bootstrap("alias-hierarchy-edit", &ctx.without_output())?;
    runtime.alias_hierarchy().apply_operation(&json!({
        "action": "apply_alias_hierarchy_operation",
        "toml_content": toml_content,
        "operation": operation,
    }))
}

fn apply_hierarchy_operation(
    path: &Path,
    ctx: &CommandContext,
    operation: serde_json::Value,
) -> Result<crate::core::runtime::AliasHierarchyOperationOutput, AppError> {
    let toml_content = fs::read_to_string(path)
        .map_err(|error| AppError::Io(format!("Read alias TOML {} failed: {error}", path.display())))?;
    let result = plan_hierarchy_operation(&toml_content, ctx, operation)?;
    fs::write(path, &result.updated_toml_content)
        .map_err(|error| AppError::Io(format!("Write alias TOML {} failed: {error}", path.display())))?;
    Ok(result)
}




fn promote(args: AliasFileArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    apply_hierarchy_operation(
        &path,
        ctx,
        json!({"kind": "promote_leaf", "target_alias": args.alias}),
    )?;
    println!(
        "Promoted alias `{}` to a group in {}.",
        args.alias,
        path.display()
    );
    Ok(())
}

fn group_alias(args: AliasGroupArgs, rename: bool, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let operation = if rename {
        let old_alias = args.old_alias.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("--old-alias is required for rename-group-alias.".into())
        })?;
        json!({
            "kind": "rename_group_alias",
            "target_path": args.group,
            "old_alias": old_alias,
            "new_name": args.alias,
        })
    } else {
        json!({
            "kind": "append_group_alias",
            "target_path": args.group,
            "aliases": [args.alias],
        })
    };
    apply_hierarchy_operation(&path, ctx, operation)?;
    println!("Updated group aliases in {}.", path.display());
    Ok(())
}

fn move_config(args: AliasMoveConfigArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let result = apply_hierarchy_operation(
        &path,
        ctx,
        json!({
            "kind": "move_leaf",
            "target_alias": args.alias,
            "destination_path": args.to,
        }),
    )?;
    let replacement = result.replacements.first().ok_or_else(|| {
        AppError::Logic("Core move_leaf did not return a canonical replacement.".into())
    })?;
    println!(
        "Moved canonical leaf selected by alias `{}` in TOML only to `{}`.",
        args.alias,
        args.to
    );
    println!("Old canonical: {}", replacement.old_canonical);
    println!("New canonical: {}", replacement.new_canonical);
    println!("TXT files and database were not modified.");
    println!("Existing TXT using the old canonical may require migration.");
    Ok(())
}

fn move_alias(args: AliasMoveArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file_args.file);
    let original_toml = fs::read_to_string(&path)
        .map_err(|e| AppError::Io(format!("Read alias TOML failed: {e}")))?;
    let input_root = PathBuf::from(&args.input);
    let planned = plan_hierarchy_operation(
        &original_toml,
        ctx,
        json!({
            "kind": "move_leaf",
            "target_alias": args.file_args.alias,
            "destination_path": args.to,
        }),
    )?;
    let updated_files = migrate_alias_sources(
        &path,
        &original_toml,
        &planned.updated_toml_content,
        &input_root,
        &planned.replacements,
        ctx,
    )?;
    let replacement = planned.replacements.first().ok_or_else(|| {
        AppError::Logic("Core move_leaf did not return a canonical replacement.".into())
    })?;
    println!(
        "Moved canonical leaf from `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
        replacement.old_canonical, replacement.new_canonical, updated_files
    );
    Ok(())
}

fn rename_group(args: AliasRenameGroupArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let original_toml = fs::read_to_string(&path)
        .map_err(|e| AppError::Io(format!("Read alias TOML failed: {e}")))?;
    let planned = plan_hierarchy_operation(
        &original_toml,
        ctx,
        json!({
            "kind": "rename_group_canonical",
            "target_path": args.group,
            "new_name": args.name,
        }),
    )?;

    let input_root = PathBuf::from(&args.input);
    let updated_files = migrate_alias_sources(
        &path,
        &original_toml,
        &planned.updated_toml_content,
        &input_root,
        &planned.replacements,
        ctx,
    )?;
    println!(
        "Renamed group `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
        args.group, args.name, updated_files
    );
    Ok(())
}

fn migrate_alias_sources(
    path: &Path,
    original_toml: &str,
    updated_toml: &str,
    input_root: &Path,
    replacements: &[crate::core::runtime::AliasHierarchyCanonicalReplacement],
    ctx: &CommandContext,
) -> Result<usize, AppError> {
    if replacements.is_empty() {
        return Err(AppError::Config("No canonical replacements were generated.".into()));
    }
    let api = CoreApi::load()?;
    let active = api.bootstrap("alias-canonical-migration", ctx)?;
    let active_db = PathBuf::from(&active.paths().db_path);
    let txt_files = collect_txt_files(input_root)?;
    let replacement_values: Vec<serde_json::Value> = replacements
        .iter()
        .map(|replacement| {
            json!({
                "old_canonical": replacement.old_canonical,
                "new_canonical": replacement.new_canonical,
            })
        })
        .collect();
    let mut txt_candidates = Vec::new();
    for file in &txt_files {
        let content = fs::read_to_string(file)
            .map_err(|e| AppError::Io(format!("Read TXT {} failed: {e}", file.display())))?;
        let replaced = active.txt().replace_canonical_activity_names(&json!({
            "action": "replace_canonical_activity_names",
            "content": content,
            "replacements": replacement_values.clone(),
        }))?;
        if replaced.updated_content != content {
            txt_candidates.push((file.clone(), content, replaced.updated_content));
        }
    }
    drop(active);

    for (file, _, updated) in &txt_candidates {
        if let Err(error) = fs::write(file, updated) {
            rollback_sources(path, original_toml, &txt_candidates);
            return Err(AppError::Io(format!("Write TXT {} failed: {error}", file.display())));
        }
    }
    if let Err(error) = fs::write(path, updated_toml) {
        rollback_sources(path, original_toml, &txt_candidates);
        return Err(AppError::Io(format!("Write alias TOML failed: {error}")));
    }

    let tx = active_db
        .parent()
        .unwrap_or(Path::new("."))
        .join(".alias-canonical-migration-tmp");
    let candidate_db = tx.join("candidate.sqlite3");
    let candidate_output = tx.join("output");
    let _ = fs::remove_dir_all(&tx);
    if let Err(error) = fs::create_dir_all(&tx) {
        rollback_sources(path, original_toml, &txt_candidates);
        return Err(AppError::Io(format!("Create migration directory failed: {error}")));
    }
    if let Err(error) = fs::create_dir_all(&candidate_output) {
        rollback_sources(path, original_toml, &txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(AppError::Io(format!(
            "Create candidate output directory failed: {error}"
        )));
    }
    let candidate_ctx = CommandContext {
        db_path: Some(candidate_db.to_string_lossy().into_owned()),
        output_path: Some(candidate_output.to_string_lossy().into_owned()),
    };
    let candidate_result = (|| -> Result<(), AppError> {
        let candidate_api = CoreApi::load()?;
        let candidate = candidate_api.bootstrap("alias-canonical-migration", &candidate_ctx)?;
        candidate.pipeline().ingest(&json!({
            "input_path": input_root,
            "date_check_mode": "continuity",
            "save_processed_output": false,
        }))?;
        drop(candidate);
        Ok(())
    })();
    if let Err(error) = candidate_result {
        rollback_sources(path, original_toml, &txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }

    let backup = tx.join("backup.sqlite3");
    if let Err(error) = move_sqlite_files(&active_db, &backup) {
        rollback_sources(path, original_toml, &txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }
    if let Err(error) = move_sqlite_files(&candidate_db, &active_db) {
        let _ = move_sqlite_files(&backup, &active_db);
        rollback_sources(path, original_toml, &txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }
    let _ = fs::remove_dir_all(&tx);
    Ok(txt_candidates.len())
}

fn collect_txt_files(root: &Path) -> Result<Vec<PathBuf>, AppError> {
    if !root.is_dir() {
        return Err(AppError::InvalidArguments(format!(
            "TXT input path is not a directory: {}",
            root.display()
        )));
    }
    let mut out = Vec::new();
    for entry in
        fs::read_dir(root).map_err(|e| AppError::Io(format!("List TXT input failed: {e}")))?
    {
        let path = entry
            .map_err(|e| AppError::Io(format!("Read TXT input entry failed: {e}")))?
            .path();
        if path.is_dir() {
            out.extend(collect_txt_files(&path)?);
        } else if path.extension().and_then(|e| e.to_str()) == Some("txt") {
            out.push(path);
        }
    }
    out.sort();
    Ok(out)
}

fn rollback_sources(path: &Path, toml: &str, candidates: &[(PathBuf, String, String)]) {
    let _ = fs::write(path, toml);
    for (file, original, _) in candidates {
        let _ = fs::write(file, original);
    }
}

fn move_sqlite_files(source: &Path, target: &Path) -> Result<(), AppError> {
    if let Some(parent) = target.parent() {
        fs::create_dir_all(parent)
            .map_err(|e| AppError::Io(format!("Create database directory failed: {e}")))?;
    }
    for suffix in ["", "-wal", "-shm"] {
        let from = PathBuf::from(format!("{}{}", source.display(), suffix));
        if from.exists() {
            let to = PathBuf::from(format!("{}{}", target.display(), suffix));
            if to.exists() {
                fs::remove_file(&to)
                    .map_err(|e| AppError::Io(format!("Remove database backup failed: {e}")))?;
            }
            fs::rename(&from, &to)
                .map_err(|e| AppError::Io(format!("Move database file failed: {e}")))?;
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::{SystemTime, UNIX_EPOCH};

    fn fixture_path(name: &str) -> PathBuf {
        let stamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        std::env::temp_dir().join(format!("time-tracer-alias-{name}-{stamp}.toml"))
    }

    #[test]
    fn promote_preserves_record_name_and_canonical_leaf() {
        let path = fixture_path("promote");
        fs::write(&path, "parent = \"exercise\"\n\n[aliases]\n\"cardio\" = [\"有氧运动\"]\n\"running\" = [\"跑步\"]\n").unwrap();
        promote(AliasFileArgs {
            file: path.to_string_lossy().into_owned(),
            alias: "有氧运动".into(),
        }, &CommandContext::default())
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[aliases]\n"));
        assert!(text.contains("[aliases.cardio]"));
        assert!(text.contains("group_aliases = [\"有氧运动\"]"));
        assert!(text.contains("running = [\"跑步\"]"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn add_supports_root_aliases() {
        let path = fixture_path("root-alias");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[aliases.cardio]\ngroup_aliases = [\"有氧运动\"]\n",
        )
        .unwrap();
        add_alias(AliasAddArgs {
            file: path.to_string_lossy().into_owned(),
            group: "root".into(),
            alias: "瑜伽".into(),
            canonical: "yoga".into(),
        }, &CommandContext::default())
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("yoga = [\"瑜伽\"]"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn group_alias_add_and_rename_round_trip() {
        let path = fixture_path("group-alias");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[aliases.cardio]\ngroup_aliases = [\"有氧运动\"]\n",
        )
        .unwrap();
        group_alias(
            AliasGroupArgs {
                file: path.to_string_lossy().into_owned(),
                group: "cardio".into(),
                alias: "有氧".into(),
                old_alias: None,
            },
            false,
            &CommandContext::default(),
        )
        .unwrap();
        group_alias(
            AliasGroupArgs {
                file: path.to_string_lossy().into_owned(),
                group: "cardio".into(),
                alias: "锻炼".into(),
                old_alias: None,
            },
            false,
            &CommandContext::default(),
        )
        .unwrap();
        group_alias(
            AliasGroupArgs {
                file: path.to_string_lossy().into_owned(),
                group: "cardio".into(),
                alias: "运动".into(),
                old_alias: Some("锻炼".into()),
            },
            true,
            &CommandContext::default(),
        )
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("有氧运动"));
        assert!(text.contains("运动"));
        assert!(!text.contains("有氧\"]"));
        assert!(
            group_alias(
                AliasGroupArgs {
                    file: path.to_string_lossy().into_owned(),
                    group: "cardio".into(),
                    alias: "有氧运动".into(),
                    old_alias: None,
                },
                false,
                &CommandContext::default(),
            )
            .is_err()
        );
        let _ = fs::remove_file(path);
    }

    #[test]
    fn promote_preserves_nested_group_location() {
        let path = fixture_path("nested-promote");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[aliases.cardio]\n\"running\" = [\"跑步\"]\n",
        )
        .unwrap();
        promote(AliasFileArgs {
            file: path.to_string_lossy().into_owned(),
            alias: "跑步".into(),
        }, &CommandContext::default())
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[aliases.cardio.running]"));
        assert!(text.contains("group_aliases = [\"跑步\"]"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn move_config_moves_entire_canonical_leaf_without_txt_or_database() {
        let path = fixture_path("move-config");
        fs::write(
            &path,
            "parent = \"study\"\n\n[aliases.math.calculus]\n\"double-integral\" = [\"二重积分\", \"高等数学二重积分\"]\n\n[aliases.math.calculus.multiple-integral]\ngroup_aliases = [\"重积分\"]\n",
        )
        .unwrap();

        move_config(AliasMoveConfigArgs {
            file: path.to_string_lossy().into_owned(),
            alias: "二重积分".into(),
            to: "math.calculus.multiple-integral".into(),
        }, &CommandContext::default())
        .unwrap();

        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[aliases.math.calculus.multiple-integral]"));
        assert!(text.contains("double-integral = [ '二重积分', '高等数学二重积分' ]"));
        let _ = fs::remove_file(path);
    }

}
