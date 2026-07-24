use std::collections::HashSet;
use std::fs;
use std::path::{Path, PathBuf};

use serde_json::json;
use toml::{Table, Value};

use crate::cli::{
    AliasAddArgs, AliasArgs, AliasCommand, AliasFileArgs, AliasGroupArgs, AliasMoveArgs,
};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct AliasHandler;

impl CommandHandler<AliasArgs> for AliasHandler {
    fn handle(&self, args: AliasArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            AliasCommand::Add(args) => add_alias(args),
            AliasCommand::Promote(args) => promote(args),
            AliasCommand::Move(args) => move_alias(args, ctx),
            AliasCommand::RenameGroupAlias(args) => group_alias(args, true),
            AliasCommand::AddGroupAlias(args) => group_alias(args, false),
        }
    }
}

fn add_alias(args: AliasAddArgs) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let mut document = read_document(&path)?;
    let aliases = aliases_table(&mut document)?;
    let group_path = path_segments(&args.group)?;
    let table = group_table(aliases, &group_path)?;
    if table.contains_key(&args.alias) || args.alias == "group_aliases" {
        return Err(AppError::Config(format!(
            "Alias `{}` already exists or is reserved.",
            args.alias
        )));
    }
    if args.canonical.trim().is_empty() {
        return Err(AppError::InvalidArguments(
            "--canonical must not be empty.".into(),
        ));
    }
    table.insert(args.alias.clone(), Value::String(args.canonical.clone()));
    let mut seen = HashSet::new();
    collect_aliases(aliases, &mut seen)?;
    write_document(&path, &document)?;
    println!(
        "Added alias `{}` = `{}` to `{}`.",
        args.alias, args.canonical, args.group
    );
    Ok(())
}

fn read_document(path: &Path) -> Result<Value, AppError> {
    let text = fs::read_to_string(path)
        .map_err(|e| AppError::Io(format!("Read alias TOML {} failed: {e}", path.display())))?;
    text.parse::<Value>()
        .map_err(|e| AppError::Config(format!("Parse alias TOML {} failed: {e}", path.display())))
}

fn write_document(path: &Path, document: &Value) -> Result<(), AppError> {
    let mut text = toml::to_string_pretty(document)
        .map_err(|e| AppError::Config(format!("Serialize alias TOML failed: {e}")))?;
    // Keep the explicit root table in the author-facing file even when all
    // aliases have been promoted into nested groups. The TOML serializer
    // treats `[aliases]` as an implicit parent of `[aliases.foo]` and omits
    // the otherwise empty parent header.
    if document
        .get("aliases")
        .and_then(Value::as_table)
        .is_some()
        && !text.contains("[aliases]\n")
    {
        if let Some(index) = text.find("[aliases.") {
            text.insert_str(index, "[aliases]\n\n");
        }
    }
    fs::write(path, text)
        .map_err(|e| AppError::Io(format!("Write alias TOML {} failed: {e}", path.display())))
}

fn aliases_table(document: &mut Value) -> Result<&mut Table, AppError> {
    document
        .as_table_mut()
        .and_then(|table| table.get_mut("aliases"))
        .and_then(Value::as_table_mut)
        .ok_or_else(|| AppError::Config("Alias TOML must contain an [aliases] table.".into()))
}

fn group_table<'a>(aliases: &'a mut Table, path: &[String]) -> Result<&'a mut Table, AppError> {
    if path == ["root"] {
        return Ok(aliases);
    }
    let mut current = aliases;
    for segment in path {
        current = current
            .get_mut(segment)
            .and_then(Value::as_table_mut)
            .ok_or_else(|| {
                AppError::InvalidArguments(format!(
                    "Alias group `{}` was not found.",
                    path.join(".")
                ))
            })?;
    }
    Ok(current)
}

fn path_segments(path: &str) -> Result<Vec<String>, AppError> {
    let segments: Vec<String> = path
        .split('.')
        .map(str::trim)
        .filter(|s| !s.is_empty())
        .map(str::to_owned)
        .collect();
    if segments.is_empty() {
        return Err(AppError::InvalidArguments(
            "Group path must not be empty.".into(),
        ));
    }
    Ok(segments)
}

fn remove_alias(
    table: &mut Table,
    alias: &str,
    path: &mut Vec<String>,
) -> Option<(String, String, Vec<String>)> {
    if let Some(value) = table.get(alias).and_then(Value::as_str).map(str::to_owned) {
        table.remove(alias);
        return Some((alias.to_owned(), value, path.clone()));
    }
    let keys: Vec<String> = table.keys().cloned().collect();
    for key in keys {
        if table.get(&key).and_then(Value::as_table).is_some() && key != "group_aliases" {
            path.push(key.clone());
            let result = table
                .get_mut(&key)
                .and_then(Value::as_table_mut)
                .and_then(|child| remove_alias(child, alias, path));
            path.pop();
            if result.is_some() {
                return result;
            }
        }
    }
    None
}

fn collect_aliases(table: &Table, seen: &mut HashSet<String>) -> Result<(), AppError> {
    for (key, value) in table {
        if key == "group_aliases" {
            let aliases = value
                .as_array()
                .ok_or_else(|| AppError::Config("group_aliases must be an array.".into()))?;
            for alias in aliases {
                let alias = alias.as_str().ok_or_else(|| {
                    AppError::Config("group_aliases must contain strings.".into())
                })?;
                if !seen.insert(alias.to_owned()) {
                    return Err(AppError::Config(format!("Duplicate alias key `{alias}`.")));
                }
            }
        } else if let Some(leaf) = value.as_str() {
            if !seen.insert(key.clone()) {
                return Err(AppError::Config(format!("Duplicate alias key `{key}`.")));
            }
            if leaf.trim().is_empty() {
                return Err(AppError::Config(format!(
                    "Alias `{key}` has an empty canonical leaf."
                )));
            }
        } else if let Some(child) = value.as_table() {
            collect_aliases(child, seen)?;
        } else {
            return Err(AppError::Config(format!(
                "Alias field `{key}` must be a string or table."
            )));
        }
    }
    Ok(())
}

fn parent(document: &Value) -> Result<String, AppError> {
    document
        .as_table()
        .and_then(|t| t.get("parent"))
        .and_then(Value::as_str)
        .filter(|p| !p.trim().is_empty())
        .map(str::to_owned)
        .ok_or_else(|| AppError::Config("Alias TOML must contain a non-empty `parent`.".into()))
}

fn canonical(parent: &str, groups: &[String], leaf: &str) -> String {
    std::iter::once(parent.to_owned())
        .chain(groups.iter().cloned())
        .chain(std::iter::once(leaf.to_owned()))
        .collect::<Vec<_>>()
        .join("_")
}

fn promote(args: AliasFileArgs) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let mut document = read_document(&path)?;
    let aliases = aliases_table(&mut document)?;
    let mut source_path = Vec::new();
    let (alias, leaf, source_groups) = remove_alias(aliases, &args.alias, &mut source_path)
        .ok_or_else(|| {
            AppError::InvalidArguments(format!("Alias `{}` was not found.", args.alias))
        })?;
    let parent_table = group_table(aliases, &source_groups)?;
    if leaf == "group_aliases" || parent_table.contains_key(&leaf) {
        return Err(AppError::Config(format!(
            "A group or alias named `{leaf}` already exists."
        )));
    }
    let mut group = Table::new();
    group.insert(
        "group_aliases".into(),
        Value::Array(vec![Value::String(alias.clone())]),
    );
    parent_table.insert(leaf.clone(), Value::Table(group));
    let mut seen = HashSet::new();
    collect_aliases(aliases, &mut seen)?;
    write_document(&path, &document)?;
    println!(
        "Promoted alias `{alias}` to group `{leaf}` in {}.",
        path.display()
    );
    Ok(())
}

fn group_alias(args: AliasGroupArgs, rename: bool) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file);
    let mut document = read_document(&path)?;
    let aliases = aliases_table(&mut document)?;
    let group_path = path_segments(&args.group)?;
    let table = group_table(aliases, &group_path)?;
    let list = table
        .entry("group_aliases")
        .or_insert_with(|| Value::Array(Vec::new()))
        .as_array_mut()
        .ok_or_else(|| AppError::Config("group_aliases must be an array.".into()))?;
    if rename {
        let old = args.old_alias.ok_or_else(|| {
            AppError::InvalidArguments("--old-alias is required for rename-group-alias.".into())
        })?;
        let index = list
            .iter()
            .position(|v| v.as_str() == Some(old.as_str()))
            .ok_or_else(|| {
                AppError::InvalidArguments(format!("Group alias `{old}` was not found."))
            })?;
        if list.iter().any(|v| v.as_str() == Some(args.alias.as_str())) {
            return Err(AppError::Config(format!(
                "Duplicate alias key `{}`.",
                args.alias
            )));
        }
        list[index] = Value::String(args.alias.clone());
    } else {
        if list.iter().any(|v| v.as_str() == Some(args.alias.as_str())) {
            return Err(AppError::Config(format!(
                "Duplicate alias key `{}`.",
                args.alias
            )));
        }
        list.push(Value::String(args.alias.clone()));
    }
    let mut seen = HashSet::new();
    collect_aliases(aliases, &mut seen)?;
    write_document(&path, &document)?;
    println!("Updated group aliases in {}.", path.display());
    Ok(())
}

fn move_alias(args: AliasMoveArgs, ctx: &CommandContext) -> Result<(), AppError> {
    let path = PathBuf::from(&args.file_args.file);
    let original_toml = fs::read_to_string(&path)
        .map_err(|e| AppError::Io(format!("Read alias TOML failed: {e}")))?;
    let mut document: Value = original_toml
        .parse()
        .map_err(|e| AppError::Config(format!("Parse alias TOML failed: {e}")))?;
    let parent_name = parent(&document)?;
    let aliases = aliases_table(&mut document)?;
    let mut source_path = Vec::new();
    let (alias, leaf, source_groups) =
        remove_alias(aliases, &args.file_args.alias, &mut source_path).ok_or_else(|| {
            AppError::InvalidArguments(format!("Alias `{}` was not found.", args.file_args.alias))
        })?;
    let target_path = path_segments(&args.to)?;
    let target = group_table(aliases, &target_path)?;
    if target.contains_key(&alias)
        || target
            .get("group_aliases")
            .and_then(Value::as_array)
            .is_some_and(|a| a.iter().any(|v| v.as_str() == Some(alias.as_str())))
    {
        return Err(AppError::Config(format!(
            "Target group already contains `{alias}` or `{leaf}`."
        )));
    }
    target.insert(alias.clone(), Value::String(leaf.clone()));
    let mut seen = HashSet::new();
    collect_aliases(aliases, &mut seen)?;
    let old_canonical = canonical(&parent_name, &source_groups, &leaf);
    let new_canonical = canonical(&parent_name, &target_path, &leaf);
    if old_canonical == new_canonical {
        return Err(AppError::InvalidArguments(
            "Alias is already in the target group.".into(),
        ));
    }
    let updated_toml = toml::to_string_pretty(&document)
        .map_err(|e| AppError::Config(format!("Serialize alias TOML failed: {e}")))?;

    let input_root = PathBuf::from(&args.input);
    let api = CoreApi::load()?;
    let active = api.bootstrap("alias-move", ctx)?;
    let active_db = PathBuf::from(&active.paths().db_path);
    let txt_files = collect_txt_files(&input_root)?;
    let mut txt_candidates = Vec::new();
    for file in &txt_files {
        let content = fs::read_to_string(file)
            .map_err(|e| AppError::Io(format!("Read TXT {} failed: {e}", file.display())))?;
        let replaced = active.txt().replace_canonical_activity_names(&json!({
            "action":"replace_canonical_activity_names", "content":content,
            "replacements":[{"old_canonical":old_canonical,"new_canonical":new_canonical}]
        }))?;
        if replaced.updated_content != content {
            txt_candidates.push((file.clone(), content, replaced.updated_content));
        }
    }
    drop(active);

    for (file, _, updated) in &txt_candidates {
        fs::write(file, updated)
            .map_err(|e| AppError::Io(format!("Write TXT {} failed: {e}", file.display())))?;
    }
    if let Err(error) = fs::write(&path, &updated_toml) {
        rollback_sources(&path, &original_toml, &txt_candidates);
        return Err(AppError::Io(format!("Write alias TOML failed: {error}")));
    }

    let tx = active_db
        .parent()
        .unwrap_or(Path::new("."))
        .join(".alias-move-tmp");
    let candidate_db = tx.join("candidate.sqlite3");
    let candidate_output = tx.join("output");
    let _ = fs::remove_dir_all(&tx);
    if let Err(error) = fs::create_dir_all(&tx) {
        rollback_sources(&path, &original_toml, &txt_candidates);
        return Err(AppError::Io(format!(
            "Create migration directory failed: {error}"
        )));
    }
    fs::create_dir_all(&candidate_output).map_err(|error| {
        rollback_sources(&path, &original_toml, &txt_candidates);
        AppError::Io(format!("Create candidate output directory failed: {error}"))
    })?;
    let candidate_ctx = CommandContext {
        db_path: Some(candidate_db.to_string_lossy().into_owned()),
        output_path: Some(candidate_output.to_string_lossy().into_owned()),
    };
    let candidate_result = (|| -> Result<(), AppError> {
        let candidate_api = CoreApi::load()?;
        let candidate = candidate_api.bootstrap("alias-move", &candidate_ctx)?;
        candidate.pipeline().ingest(&json!({"input_path":input_root,"date_check_mode":"continuity","save_processed_output":false}))?;
        drop(candidate);
        Ok(())
    })();
    if let Err(error) = candidate_result {
        rollback_sources(&path, &original_toml, &txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }
    let backup = tx.join("backup.sqlite3");
    if let Err(error) = move_sqlite_files(&active_db, &backup) {
        rollback_sources(&path, &original_toml, &txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }
    if let Err(error) = move_sqlite_files(&candidate_db, &active_db) {
        let _ = move_sqlite_files(&backup, &active_db);
        rollback_sources(&path, &original_toml, &txt_candidates);
        return Err(error);
    }
    let _ = fs::remove_dir_all(&tx);
    println!(
        "Moved `{alias}` from `{}` to `{}`; rebuilt database and updated {} TXT file(s).",
        source_groups.join("."),
        args.to,
        txt_candidates.len()
    );
    Ok(())
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
        fs::write(&path, "parent = \"exercise\"\n\n[aliases]\n\"有氧运动\" = \"cardio\"\n\"跑步\" = \"running\"\n").unwrap();
        promote(AliasFileArgs {
            file: path.to_string_lossy().into_owned(),
            alias: "有氧运动".into(),
        })
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[aliases]\n"));
        assert!(text.contains("[aliases.cardio]"));
        assert!(text.contains("group_aliases = [\"有氧运动\"]"));
        assert!(text.contains("\"跑步\" = \"running\""));
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
        })
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("\"瑜伽\" = \"yoga\""));
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
                alias: "锻炼".into(),
                old_alias: None,
            },
            false,
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
        )
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("有氧运动"));
        assert!(text.contains("运动"));
        assert!(!text.contains("锻炼"));
        let _ = fs::remove_file(path);
    }

    #[test]
    fn promote_preserves_nested_group_location() {
        let path = fixture_path("nested-promote");
        fs::write(
            &path,
            "parent = \"exercise\"\n\n[aliases.cardio]\n\"跑步\" = \"running\"\n",
        )
        .unwrap();
        promote(AliasFileArgs {
            file: path.to_string_lossy().into_owned(),
            alias: "跑步".into(),
        })
        .unwrap();
        let text = fs::read_to_string(&path).unwrap();
        assert!(text.contains("[aliases.cardio.running]"));
        assert!(text.contains("group_aliases = [\"跑步\"]"));
        let _ = fs::remove_file(path);
    }
}
