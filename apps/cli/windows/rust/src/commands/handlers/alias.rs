use std::fs::{self, OpenOptions};
use std::io::Write;
use std::path::{Path, PathBuf};

use serde_json::json;

use crate::cli::{
    AliasAddArgs, AliasArgs, AliasCommand, AliasCreateArgs, AliasFileArgs, AliasGroupArgs,
    AliasMoveArgs, AliasMoveConfigArgs, AliasRenameGroupArgs, AliasRenameParentArgs,
};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

use super::alias_migration::{
    collect_alias_documents, migrate_alias_document_sources, rollback_alias_tomls,
    updated_cross_document_files, write_alias_toml_candidates,
};
use super::alias_tree::render_tree;

pub(crate) use super::alias_migration::migrate_alias_sources;

pub struct AliasHandler;

impl CommandHandler<AliasArgs> for AliasHandler {
    fn handle(&self, args: AliasArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            AliasCommand::Create(args) => create_alias(args),
            AliasCommand::Add(args) => add_alias(args, ctx),
            AliasCommand::Promote(args) => promote(args, ctx),
            AliasCommand::Move(args) => move_alias(args, ctx),
            AliasCommand::RenameGroup(args) => rename_group(args, ctx),
            AliasCommand::RenameParent(args) => rename_parent(args, ctx),
            AliasCommand::MoveConfig(args) => move_config(args, ctx),
            AliasCommand::Tree(args) => render_tree(args, ctx),
            AliasCommand::RenameGroupAlias(args) => group_alias(args, true, ctx),
            AliasCommand::AddGroupAlias(args) => group_alias(args, false, ctx),
        }
    }
}

fn create_alias(args: AliasCreateArgs) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let parent_dir = path.parent().ok_or_else(|| {
        AppError::InvalidArguments(
            "Activity hierarchy TOML must be under activity_hierarchy/.".into(),
        )
    })?;
    if parent_dir.file_name().and_then(|name| name.to_str()) != Some("activity_hierarchy") {
        return Err(AppError::InvalidArguments(
            "Activity hierarchy TOML must be under an activity_hierarchy directory.".into(),
        ));
    }
    if path.extension().and_then(|extension| extension.to_str()) != Some("toml") {
        return Err(AppError::InvalidArguments(
            "Activity hierarchy file must use the .toml extension.".into(),
        ));
    }
    let file_name = path
        .file_stem()
        .and_then(|name| name.to_str())
        .ok_or_else(|| {
            AppError::InvalidArguments(
                "Activity hierarchy TOML file name must not be empty.".into(),
            )
        })?;
    if file_name.is_empty() || file_name == "_system" {
        return Err(AppError::InvalidArguments(
            "Activity hierarchy TOML file name must be a non-system name.".into(),
        ));
    }

    fs::create_dir_all(parent_dir).map_err(|error| {
        AppError::Io(format!(
            "Create activity hierarchy directory {} failed: {error}",
            parent_dir.display()
        ))
    })?;
    let content = format!(
        "parent = \"{}\"\n\n[canonical]\n",
        escape_toml_basic_string(file_name)
    );
    let mut output = OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(&path)
        .map_err(|error| {
            if error.kind() == std::io::ErrorKind::AlreadyExists {
                AppError::InvalidArguments(format!(
                    "Activity hierarchy TOML already exists: {}",
                    path.display()
                ))
            } else {
                AppError::Io(format!(
                    "Create activity hierarchy TOML {} failed: {error}",
                    path.display()
                ))
            }
        })?;
    if let Err(error) = output.write_all(content.as_bytes()) {
        let _ = fs::remove_file(&path);
        return Err(AppError::Io(format!(
            "Write activity hierarchy TOML {} failed: {error}",
            path.display()
        )));
    }
    println!("Created activity hierarchy TOML: {}", path.display());
    Ok(())
}

fn escape_toml_basic_string(value: &str) -> String {
    value.replace('\\', "\\\\").replace('"', "\\\"")
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

pub(crate) fn plan_hierarchy_operation(
    toml_content: &str,
    ctx: &CommandContext,
    operation: serde_json::Value,
) -> Result<crate::core::runtime::ActivityHierarchyOperationOutput, AppError> {
    let api = CoreApi::load()?;
    let runtime = api.bootstrap("activity-hierarchy-edit", &ctx.without_output())?;
    runtime.activity_hierarchy().apply_operation(&json!({
        "action": "apply_activity_hierarchy_operation",
        "toml_content": toml_content,
        "operation": operation,
    }))
}

fn plan_cross_document_leaf_move(
    source_path: &Path,
    destination_path: &Path,
    target_alias: &str,
    destination_group: &str,
    ctx: &CommandContext,
) -> Result<crate::core::runtime::ActivityHierarchyCrossDocumentOperationOutput, AppError> {
    plan_cross_document_node_move(
        source_path,
        destination_path,
        json!({
            "kind": "move_leaf",
            "target_alias": target_alias,
            "destination_path": destination_group,
        }),
        ctx,
    )
}

fn plan_cross_document_group_move(
    source_path: &Path,
    destination_path: &Path,
    target_group: &str,
    destination_group: &str,
    ctx: &CommandContext,
) -> Result<crate::core::runtime::ActivityHierarchyCrossDocumentOperationOutput, AppError> {
    plan_cross_document_node_move(
        source_path,
        destination_path,
        json!({
            "kind": "move_group",
            "target_path": target_group,
            "destination_path": destination_group,
        }),
        ctx,
    )
}

fn plan_cross_document_node_move(
    source_path: &Path,
    destination_path: &Path,
    operation: serde_json::Value,
    ctx: &CommandContext,
) -> Result<crate::core::runtime::ActivityHierarchyCrossDocumentOperationOutput, AppError> {
    let source_path = source_path
        .canonicalize()
        .map_err(|e| AppError::Io(format!("Resolve source canonical TOML failed: {e}")))?;
    let destination_path = destination_path
        .canonicalize()
        .map_err(|e| AppError::Io(format!("Resolve destination canonical TOML failed: {e}")))?;
    if source_path == destination_path {
        return Err(AppError::InvalidArguments(
            "Source and destination canonical TOML files must be different.".into(),
        ));
    }
    let documents = collect_alias_documents(&source_path, &destination_path)?;
    let source_name = source_path.to_string_lossy().into_owned();
    let destination_name = destination_path.to_string_lossy().into_owned();
    let document_values: Vec<serde_json::Value> = documents
        .iter()
        .map(|(_, name, content)| {
            json!({
                "source_name": name,
                "toml_content": content,
            })
        })
        .collect();
    let api = CoreApi::load()?;
    let runtime = api.bootstrap(
        "activity-hierarchy-cross-document-edit",
        &ctx.without_output(),
    )?;
    runtime
        .activity_hierarchy()
        .move_node_between_documents(&json!({
            "action": "move_activity_hierarchy_node_between_documents",
            "source_name": source_name,
            "destination_name": destination_name,
            "documents": document_values,
            "operation": operation,
        }))
}

fn apply_hierarchy_operation(
    path: &Path,
    ctx: &CommandContext,
    operation: serde_json::Value,
) -> Result<crate::core::runtime::ActivityHierarchyOperationOutput, AppError> {
    let toml_content = fs::read_to_string(path).map_err(|error| {
        AppError::Io(format!(
            "Read canonical TOML {} failed: {error}",
            path.display()
        ))
    })?;
    let result = plan_hierarchy_operation(&toml_content, ctx, operation)?;
    fs::write(path, &result.updated_toml_content).map_err(|error| {
        AppError::Io(format!(
            "Write canonical TOML {} failed: {error}",
            path.display()
        ))
    })?;
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
    if args.to_file.is_some() {
        return move_config_cross_document(args, ctx);
    }
    if args.group.is_some() {
        return Err(AppError::InvalidArguments(
            "Moving a group requires --to-file.".into(),
        ));
    }
    let alias = args.alias.ok_or_else(|| {
        AppError::InvalidArguments("--alias is required when --to-file is omitted.".into())
    })?;
    let path = PathBuf::from(&args.file);
    let result = apply_hierarchy_operation(
        &path,
        ctx,
        json!({
            "kind": "move_leaf",
            "target_alias": alias,
            "destination_path": args.to,
        }),
    )?;
    let replacement = result.replacements.first().ok_or_else(|| {
        AppError::Logic("Core move_leaf did not return a canonical replacement.".into())
    })?;
    println!(
        "Moved canonical leaf selected by alias `{}` in TOML only to `{}`.",
        alias, args.to
    );
    println!("Old canonical: {}", replacement.old_canonical);
    println!("New canonical: {}", replacement.new_canonical);
    println!("TXT files and database were not modified.");
    println!("Existing TXT using the old canonical may require migration.");
    Ok(())
}

fn move_config_cross_document(
    args: AliasMoveConfigArgs,
    ctx: &CommandContext,
) -> Result<(), AppError> {
    let source_path = PathBuf::from(&args.file);
    let destination_path = PathBuf::from(args.to_file.as_deref().ok_or_else(|| {
        AppError::InvalidArguments("--to-file is required for cross-document moves.".into())
    })?);
    let is_group = args.group.is_some();
    let result = if let Some(group) = args.group.as_deref() {
        plan_cross_document_group_move(&source_path, &destination_path, group, &args.to, ctx)?
    } else {
        let alias = args.alias.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("Either --alias or --group is required.".into())
        })?;
        plan_cross_document_leaf_move(&source_path, &destination_path, alias, &args.to, ctx)?
    };
    let replacement = result.replacements.first().ok_or_else(|| {
        AppError::Logic(
            "Core cross-document node move did not return a canonical replacement.".into(),
        )
    })?;
    let source_name = source_path
        .canonicalize()
        .map_err(|e| AppError::Io(format!("Resolve source canonical TOML failed: {e}")))?
        .to_string_lossy()
        .into_owned();
    let destination_name = destination_path
        .canonicalize()
        .map_err(|e| AppError::Io(format!("Resolve destination canonical TOML failed: {e}")))?
        .to_string_lossy()
        .into_owned();
    let originals = vec![
        (
            PathBuf::from(&source_name),
            source_name.clone(),
            fs::read_to_string(&source_name)
                .map_err(|e| AppError::Io(format!("Read source canonical TOML failed: {e}")))?,
        ),
        (
            PathBuf::from(&destination_name),
            destination_name.clone(),
            fs::read_to_string(&destination_name).map_err(|e| {
                AppError::Io(format!("Read destination canonical TOML failed: {e}"))
            })?,
        ),
    ];
    let updated = updated_cross_document_files(&originals, &result)?;
    write_alias_toml_candidates(&originals, &updated)?;
    if is_group {
        println!(
            "Moved group subtree from `{}` into group `{}` in `{}` (TOML only).",
            source_name, args.to, destination_name
        );
    } else {
        println!(
            "Moved canonical leaf selected by alias from `{}` into group `{}` in `{}` (TOML only).",
            source_name, args.to, destination_name
        );
    }
    println!("Old canonical: {}", replacement.old_canonical);
    println!("New canonical: {}", replacement.new_canonical);
    println!("TXT files and database were not modified.");
    println!("Existing TXT using the old canonical may require migration.");
    Ok(())
}

fn move_alias(args: AliasMoveArgs, ctx: &CommandContext) -> Result<(), AppError> {
    if args.to_file.is_some() {
        return move_alias_cross_document(args, ctx);
    }
    if args.group.is_some() {
        return Err(AppError::InvalidArguments(
            "Moving a group requires --to-file.".into(),
        ));
    }
    let alias = args.alias.ok_or_else(|| {
        AppError::InvalidArguments("--alias is required when --to-file is omitted.".into())
    })?;
    let path = PathBuf::from(&args.file);
    let original_toml = fs::read_to_string(&path)
        .map_err(|e| AppError::Io(format!("Read canonical TOML failed: {e}")))?;
    let input_root = PathBuf::from(&args.input);
    let planned = plan_hierarchy_operation(
        &original_toml,
        ctx,
        json!({
            "kind": "move_leaf",
            "target_alias": alias,
            "destination_path": args.to,
        }),
    )?;
    let updated_files = migrate_alias_sources(
        &path,
        &original_toml,
        &planned.updated_toml_content,
        &input_root,
        &planned.replacements,
        &[],
        ctx,
    )?;
    let replacement = planned.replacements.first().ok_or_else(|| {
        AppError::Logic("Core move_leaf did not return a canonical replacement.".into())
    })?;
    if args.group.is_some() {
        println!(
            "Moved group subtree from `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
            replacement.old_canonical, replacement.new_canonical, updated_files
        );
    } else {
        println!(
            "Moved canonical leaf from `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
            replacement.old_canonical, replacement.new_canonical, updated_files
        );
    }
    Ok(())
}

fn move_alias_cross_document(args: AliasMoveArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let source_path = PathBuf::from(&args.file);
    let destination_path = PathBuf::from(args.to_file.as_deref().ok_or_else(|| {
        AppError::InvalidArguments("--to-file is required for cross-document moves.".into())
    })?);
    let input_root = PathBuf::from(&args.input);
    let planned = if let Some(group) = args.group.as_deref() {
        plan_cross_document_group_move(&source_path, &destination_path, group, &args.to, ctx)?
    } else {
        let alias = args.alias.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("Either --alias or --group is required.".into())
        })?;
        plan_cross_document_leaf_move(&source_path, &destination_path, alias, &args.to, ctx)?
    };
    let source_name = source_path
        .canonicalize()
        .map_err(|e| AppError::Io(format!("Resolve source canonical TOML failed: {e}")))?
        .to_string_lossy()
        .into_owned();
    let destination_name = destination_path
        .canonicalize()
        .map_err(|e| AppError::Io(format!("Resolve destination canonical TOML failed: {e}")))?
        .to_string_lossy()
        .into_owned();
    let originals = vec![
        (
            PathBuf::from(&source_name),
            source_name,
            fs::read_to_string(&source_path)
                .map_err(|e| AppError::Io(format!("Read source canonical TOML failed: {e}")))?,
        ),
        (
            PathBuf::from(&destination_name),
            destination_name,
            fs::read_to_string(&destination_path).map_err(|e| {
                AppError::Io(format!("Read destination canonical TOML failed: {e}"))
            })?,
        ),
    ];
    let updated = updated_cross_document_files(&originals, &planned)?;
    let updated_files = migrate_alias_document_sources(
        &originals,
        &updated,
        &planned.replacements,
        &planned.alias_replacements,
        &input_root,
        ctx,
    )?;
    let replacement = planned.replacements.first().ok_or_else(|| {
        AppError::Logic(
            "Core cross-document node move did not return a canonical replacement.".into(),
        )
    })?;
    if args.group.is_some() {
        println!(
            "Moved group subtree from `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
            replacement.old_canonical, replacement.new_canonical, updated_files
        );
    } else {
        println!(
            "Moved canonical leaf from `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
            replacement.old_canonical, replacement.new_canonical, updated_files
        );
    }
    Ok(())
}

fn rename_group(args: AliasRenameGroupArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let original_toml = fs::read_to_string(&path)
        .map_err(|e| AppError::Io(format!("Read canonical TOML failed: {e}")))?;
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
        &[],
        ctx,
    )?;
    println!(
        "Renamed group `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
        args.group, args.name, updated_files
    );
    Ok(())
}

fn rename_parent(args: AliasRenameParentArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let source_path = PathBuf::from(&args.file);
    let source_parent = source_path.parent().ok_or_else(|| {
        AppError::InvalidArguments(
            "Activity hierarchy TOML must be under activity_hierarchy/.".into(),
        )
    })?;
    if source_parent.file_name().and_then(|name| name.to_str()) != Some("activity_hierarchy") {
        return Err(AppError::InvalidArguments(
            "Activity hierarchy TOML must be under an activity_hierarchy directory.".into(),
        ));
    }
    if source_path
        .extension()
        .and_then(|extension| extension.to_str())
        != Some("toml")
    {
        return Err(AppError::InvalidArguments(
            "Activity hierarchy file must use the .toml extension.".into(),
        ));
    }
    validate_parent_file_name(&args.name)?;
    let old_parent = source_path
        .file_stem()
        .and_then(|name| name.to_str())
        .ok_or_else(|| {
            AppError::InvalidArguments("Activity hierarchy file name is invalid.".into())
        })?
        .to_string();
    if old_parent == "_system" {
        return Err(AppError::InvalidArguments(
            "The system activity hierarchy TOML cannot be renamed.".into(),
        ));
    }
    let destination_path = source_parent.join(format!("{}.toml", args.name));
    if destination_path.exists() {
        return Err(AppError::InvalidArguments(format!(
            "Activity hierarchy TOML already exists: {}",
            destination_path.display()
        )));
    }
    let original_toml = fs::read_to_string(&source_path)
        .map_err(|e| AppError::Io(format!("Read activity hierarchy TOML failed: {e}")))?;
    let planned = plan_hierarchy_operation(
        &original_toml,
        ctx,
        json!({
            "kind": "rename_parent",
            "old_parent": old_parent,
            "new_name": args.name,
        }),
    )?;
    let mut originals = vec![(
        source_path.clone(),
        source_path.to_string_lossy().into_owned(),
        original_toml,
    )];
    let mut updated = vec![(
        destination_path.clone(),
        destination_path.to_string_lossy().into_owned(),
        planned.updated_toml_content,
    )];
    let bundle_path = source_parent
        .parent()
        .ok_or_else(|| {
            AppError::InvalidArguments("Activity hierarchy config root is invalid.".into())
        })?
        .join("meta")
        .join("bundle.toml");
    if bundle_path.is_file() {
        let bundle_content = fs::read_to_string(&bundle_path)
            .map_err(|e| AppError::Io(format!("Read config bundle manifest failed: {e}")))?;
        let old_entry = format!("activity_hierarchy/{old_parent}.toml");
        let new_entry = format!("activity_hierarchy/{}.toml", args.name);
        let updated_bundle = bundle_content.replace(&old_entry, &new_entry);
        if updated_bundle != bundle_content {
            originals.push((
                bundle_path.clone(),
                bundle_path.to_string_lossy().into_owned(),
                bundle_content,
            ));
            updated.push((
                bundle_path.clone(),
                bundle_path.to_string_lossy().into_owned(),
                updated_bundle,
            ));
        }
    }
    let updated_files = migrate_alias_document_sources(
        &originals,
        &updated,
        &planned.replacements,
        &[],
        Path::new(&args.input),
        ctx,
    )?;
    println!(
        "Renamed activity hierarchy parent `{}` to `{}`; moved TOML, rebuilt database, and updated {} TXT file(s).",
        old_parent, args.name, updated_files
    );
    Ok(())
}

fn validate_parent_file_name(name: &str) -> Result<(), AppError> {
    if name.is_empty()
        || name == "."
        || name == ".."
        || name == "group_aliases"
        || name.starts_with('_')
        || name.chars().any(char::is_whitespace)
        || name.contains('/')
        || name.contains('\\')
    {
        return Err(AppError::InvalidArguments(format!(
            "New parent must be one path-safe canonical segment: `{name}`"
        )));
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
        fs::write(&path, "parent = \"exercise\"\n\n[canonical]\n\"cardio\" = [\"有氧运动\"]\n\"running\" = [\"跑步\"]\n").unwrap();
        promote(
            AliasFileArgs {
                file: path.to_string_lossy().into_owned(),
                alias: "有氧运动".into(),
            },
            &CommandContext::default(),
        )
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[canonical]\n"));
        assert!(text.contains("[canonical.cardio]"));
        assert!(text.contains("group_aliases = [ '有氧运动' ]"));
        assert!(text.contains("running = [ '跑步' ]"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn create_writes_minimal_activity_hierarchy_toml_without_runtime() {
        let root = std::env::temp_dir().join(format!(
            "time-tracer-alias-create-{}",
            SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        let path = root.join("activity_hierarchy/study.toml");

        create_alias(AliasCreateArgs {
            file: path.to_string_lossy().into_owned(),
        })
        .unwrap();

        assert_eq!(
            fs::read_to_string(&path).unwrap(),
            "parent = \"study\"\n\n[canonical]\n"
        );
        let _ = fs::remove_dir_all(root);
    }

    #[test]
    fn parent_file_name_validation_rejects_path_like_names() {
        for name in [
            "",
            ".",
            "..",
            "group_aliases",
            "_system",
            "a/b",
            "a\\b",
            "a b",
        ] {
            assert!(validate_parent_file_name(name).is_err(), "{name}");
        }
        assert!(validate_parent_file_name("training").is_ok());
    }

    #[test]
    fn toml_path_migration_can_roll_back_after_destination_install() {
        let root = std::env::temp_dir().join(format!(
            "time-tracer-alias-parent-transaction-{}",
            SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        fs::create_dir_all(&root).unwrap();
        let old_path = root.join("exercise.toml");
        let new_path = root.join("training.toml");
        let originals = vec![(
            old_path.clone(),
            old_path.to_string_lossy().into_owned(),
            "parent = \"exercise\"\n".to_string(),
        )];
        let updated = vec![(
            new_path.clone(),
            new_path.to_string_lossy().into_owned(),
            "parent = \"training\"\n".to_string(),
        )];
        fs::write(&old_path, &originals[0].2).unwrap();

        write_alias_toml_candidates(&originals, &updated).unwrap();
        assert!(!old_path.exists());
        assert_eq!(fs::read_to_string(&new_path).unwrap(), updated[0].2);

        rollback_alias_tomls(&originals, &updated);
        assert_eq!(fs::read_to_string(&old_path).unwrap(), originals[0].2);
        assert!(!new_path.exists());
        let _ = fs::remove_dir_all(root);
    }

    #[test]
    fn add_supports_root_aliases() {
        let path = fixture_path("root-alias");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[canonical.cardio]\ngroup_aliases = [\"有氧运动\"]\n",
        )
        .unwrap();
        add_alias(
            AliasAddArgs {
                file: path.to_string_lossy().into_owned(),
                group: "root".into(),
                alias: "瑜伽".into(),
                canonical: "yoga".into(),
            },
            &CommandContext::default(),
        )
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("yoga = [ '瑜伽' ]"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn group_alias_add_and_rename_round_trip() {
        let path = fixture_path("group-alias");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[canonical.cardio]\ngroup_aliases = [\"有氧运动\"]\n",
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
        assert!(group_alias(
            AliasGroupArgs {
                file: path.to_string_lossy().into_owned(),
                group: "cardio".into(),
                alias: "有氧运动".into(),
                old_alias: None,
            },
            false,
            &CommandContext::default(),
        )
        .is_err());
        let _ = fs::remove_file(path);
    }

    #[test]
    fn promote_preserves_nested_group_location() {
        let path = fixture_path("nested-promote");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[canonical.cardio]\n\"running\" = [\"跑步\"]\n",
        )
        .unwrap();
        promote(
            AliasFileArgs {
                file: path.to_string_lossy().into_owned(),
                alias: "跑步".into(),
            },
            &CommandContext::default(),
        )
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[canonical.cardio.running]"));
        assert!(text.contains("group_aliases = [ '跑步' ]"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn move_config_moves_entire_canonical_leaf_without_txt_or_database() {
        let path = fixture_path("move-config");
        fs::write(
            &path,
            "parent = \"study\"\n\n[canonical.math.calculus]\n\"double-integral\" = [\"二重积分\", \"高等数学二重积分\"]\n\n[canonical.math.calculus.multiple-integral]\ngroup_aliases = [\"重积分\"]\n",
        )
        .unwrap();

        move_config(
            AliasMoveConfigArgs {
                file: path.to_string_lossy().into_owned(),
                alias: Some("二重积分".into()),
                group: None,
                to: "math.calculus.multiple-integral".into(),
                to_file: None,
            },
            &CommandContext::default(),
        )
        .unwrap();

        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[canonical.math.calculus.multiple-integral]"));
        assert!(text.contains("double-integral = [ '二重积分', '高等数学二重积分' ]"));
        let _ = fs::remove_file(path);
    }
}
