use std::fs;
use std::path::{Path, PathBuf};

use serde_json::json;

use crate::commands::handler::CommandContext;
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub(super) fn collect_alias_documents(
    source_path: &Path,
    destination_path: &Path,
) -> Result<Vec<(PathBuf, String, String)>, AppError> {
    let mut paths = std::collections::BTreeSet::new();
    for file in [source_path, destination_path] {
        let parent = file.parent().ok_or_else(|| {
            AppError::InvalidArguments(format!(
                "Canonical TOML has no parent directory: {}",
                file.display()
            ))
        })?;
        let entries = fs::read_dir(parent)
            .map_err(|e| AppError::Io(format!("List canonical TOML directory failed: {e}")))?;
        for entry in entries {
            let path = entry
                .map_err(|e| AppError::Io(format!("Read canonical TOML entry failed: {e}")))?
                .path();
            if path.is_file() && path.extension().and_then(|e| e.to_str()) == Some("toml") {
                paths.insert(path.canonicalize().map_err(|e| {
                    AppError::Io(format!(
                        "Resolve canonical TOML {} failed: {e}",
                        path.display()
                    ))
                })?);
            }
        }
    }
    paths.insert(source_path.to_path_buf());
    paths.insert(destination_path.to_path_buf());

    let mut documents = Vec::with_capacity(paths.len());
    for path in paths {
        let content = fs::read_to_string(&path).map_err(|e| {
            AppError::Io(format!(
                "Read canonical TOML {} failed: {e}",
                path.display()
            ))
        })?;
        documents.push((path.clone(), path.to_string_lossy().into_owned(), content));
    }
    Ok(documents)
}

pub(crate) fn migrate_alias_sources(
    path: &Path,
    original_toml: &str,
    updated_toml: &str,
    input_root: &Path,
    replacements: &[crate::core::runtime::ActivityHierarchyCanonicalReplacement],
    alias_replacements: &[crate::core::runtime::AliasKeyReplacement],
    ctx: &CommandContext,
) -> Result<usize, AppError> {
    let name = path.to_string_lossy().into_owned();
    let originals = vec![(path.to_path_buf(), name.clone(), original_toml.to_string())];
    let updated = vec![(path.to_path_buf(), name, updated_toml.to_string())];
    migrate_alias_document_sources(
        &originals,
        &updated,
        replacements,
        alias_replacements,
        input_root,
        ctx,
    )
}

pub(super) fn updated_cross_document_files(
    originals: &[(PathBuf, String, String)],
    result: &crate::core::runtime::ActivityHierarchyCrossDocumentOperationOutput,
) -> Result<Vec<(PathBuf, String, String)>, AppError> {
    let mut updated = Vec::with_capacity(result.updated_documents.len());
    for document in &result.updated_documents {
        let original = originals
            .iter()
            .find(|(_, name, _)| name == &document.source_name)
            .ok_or_else(|| {
                AppError::Logic(format!(
                    "Core returned an unknown updated canonical TOML: {}",
                    document.source_name
                ))
            })?;
        updated.push((
            original.0.clone(),
            original.1.clone(),
            document.updated_toml_content.clone(),
        ));
    }
    if updated.is_empty() {
        return Err(AppError::Logic(
            "Core cross-document node move returned no updated TOML files.".into(),
        ));
    }
    Ok(updated)
}

pub(super) fn write_alias_toml_candidates(
    originals: &[(PathBuf, String, String)],
    updated: &[(PathBuf, String, String)],
) -> Result<(), AppError> {
    let original_paths: std::collections::HashSet<&Path> = originals
        .iter()
        .map(|(path, _, _)| path.as_path())
        .collect();
    let updated_paths: std::collections::HashSet<&Path> =
        updated.iter().map(|(path, _, _)| path.as_path()).collect();
    for (path, _, _) in updated {
        if !original_paths.contains(path.as_path()) && path.exists() {
            return Err(AppError::InvalidArguments(format!(
                "Activity hierarchy TOML already exists: {}",
                path.display()
            )));
        }
    }
    for (path, _, content) in updated {
        if let Err(error) = fs::write(path, content) {
            rollback_alias_tomls(originals, updated);
            return Err(AppError::Io(format!(
                "Write canonical TOML {} failed: {error}",
                path.display()
            )));
        }
    }
    for (path, _, _) in originals {
        if !updated_paths.contains(path.as_path()) {
            if let Err(error) = fs::remove_file(path) {
                rollback_alias_tomls(originals, updated);
                return Err(AppError::Io(format!(
                    "Remove old activity hierarchy TOML {} failed: {error}",
                    path.display()
                )));
            }
        }
    }
    Ok(())
}

pub(super) fn rollback_alias_tomls(
    originals: &[(PathBuf, String, String)],
    updated: &[(PathBuf, String, String)],
) {
    let original_paths: std::collections::HashSet<&Path> = originals
        .iter()
        .map(|(path, _, _)| path.as_path())
        .collect();
    for (path, _, _) in updated {
        if !original_paths.contains(path.as_path()) {
            let _ = fs::remove_file(path);
        }
    }
    for (path, _, content) in originals {
        let _ = fs::write(path, content);
    }
}

pub(super) fn migrate_alias_document_sources(
    originals: &[(PathBuf, String, String)],
    updated: &[(PathBuf, String, String)],
    replacements: &[crate::core::runtime::ActivityHierarchyCanonicalReplacement],
    alias_replacements: &[crate::core::runtime::AliasKeyReplacement],
    input_root: &Path,
    ctx: &CommandContext,
) -> Result<usize, AppError> {
    if replacements.is_empty() {
        return Err(AppError::Config(
            "No canonical replacements were generated.".into(),
        ));
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
    let alias_replacement_values: Vec<serde_json::Value> = alias_replacements
        .iter()
        .map(|replacement| {
            json!({
                "old_alias": replacement.old_alias,
                "new_alias": replacement.new_alias,
            })
        })
        .collect();
    let mut txt_candidates = Vec::new();
    for file in &txt_files {
        let content = fs::read_to_string(file)
            .map_err(|e| AppError::Io(format!("Read TXT {} failed: {e}", file.display())))?;
        let canonical_replaced = active.txt().replace_canonical_activity_names(&json!({
            "action": "replace_canonical_activity_names",
            "content": content,
            "replacements": replacement_values.clone(),
        }))?;
        let replaced = active.txt().replace_alias_activity_names(&json!({
            "action": "replace_alias_activity_names",
            "content": canonical_replaced.updated_content,
            "replacements": alias_replacement_values.clone(),
        }))?;
        if replaced.updated_content != content {
            txt_candidates.push((file.clone(), content, replaced.updated_content));
        }
    }
    drop(active);

    for (file, _, updated_content) in &txt_candidates {
        if let Err(error) = fs::write(file, updated_content) {
            rollback_alias_tomls(originals, updated);
            rollback_txt_candidates(&txt_candidates);
            return Err(AppError::Io(format!(
                "Write TXT {} failed: {error}",
                file.display()
            )));
        }
    }
    if let Err(error) = write_alias_toml_candidates(originals, updated) {
        rollback_txt_candidates(&txt_candidates);
        return Err(error);
    }

    let tx = active_db
        .parent()
        .unwrap_or(Path::new("."))
        .join(".alias-canonical-migration-tmp");
    let candidate_db = tx.join("candidate.sqlite3");
    let candidate_output = tx.join("output");
    let _ = fs::remove_dir_all(&tx);
    if let Err(error) = fs::create_dir_all(&tx) {
        rollback_alias_tomls(originals, updated);
        rollback_txt_candidates(&txt_candidates);
        return Err(AppError::Io(format!(
            "Create migration directory failed: {error}"
        )));
    }
    if let Err(error) = fs::create_dir_all(&candidate_output) {
        rollback_alias_tomls(originals, updated);
        rollback_txt_candidates(&txt_candidates);
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
        rollback_alias_tomls(originals, updated);
        rollback_txt_candidates(&txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }

    let backup = tx.join("backup.sqlite3");
    if let Err(error) = move_sqlite_files(&active_db, &backup) {
        rollback_alias_tomls(originals, updated);
        rollback_txt_candidates(&txt_candidates);
        let _ = fs::remove_dir_all(&tx);
        return Err(error);
    }
    if let Err(error) = move_sqlite_files(&candidate_db, &active_db) {
        let _ = move_sqlite_files(&backup, &active_db);
        rollback_alias_tomls(originals, updated);
        rollback_txt_candidates(&txt_candidates);
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

fn rollback_txt_candidates(candidates: &[(PathBuf, String, String)]) {
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
