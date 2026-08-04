use std::time::Instant;

use serde::Deserialize;
use serde_json::Value;

use crate::error::AppError;

use super::super::codec::{read_c_json, to_request_json};
use super::super::env_flags::log_timing;
use super::super::errors::ErrorContract;
use super::super::CoreRuntime;
use super::super::{
    ActivityHierarchyCanonicalReplacement, ActivityHierarchyCrossDocumentOperationOutput,
    ActivityHierarchyDocumentOutput, ActivityHierarchyOperationOutput, ActivityHierarchyTree,
    AliasKeyReplacement,
};
use super::responses::{
    ActivityHierarchyCrossDocumentOperationResponse, ActivityHierarchyOperationResponse,
};
use super::transport::map_runtime_text_error;

pub(crate) fn run_activity_hierarchy_operation(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ActivityHierarchyOperationOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<ActivityHierarchyOperationResponse>(raw, "activity hierarchy")?;
    log_timing("runtime.activity_hierarchy", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(ActivityHierarchyOperationOutput {
        updated_toml_content: payload.updated_toml_content,
        replacements: payload
            .replacements
            .into_iter()
            .map(|replacement| ActivityHierarchyCanonicalReplacement {
                old_canonical: replacement.old_canonical,
                new_canonical: replacement.new_canonical,
            })
            .collect(),
        alias_replacements: payload
            .alias_replacements
            .into_iter()
            .map(|replacement| AliasKeyReplacement {
                old_alias: replacement.old_alias,
                new_alias: replacement.new_alias,
            })
            .collect(),
    })
}

#[derive(Deserialize)]
struct ActivityHierarchyDescribeResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    hierarchy: ActivityHierarchyTree,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

pub(crate) fn run_activity_hierarchy_node_move(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ActivityHierarchyCrossDocumentOperationOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<ActivityHierarchyCrossDocumentOperationResponse>(
        raw,
        "activity hierarchy leaf move",
    )?;
    log_timing("runtime.activity_hierarchy_leaf_move", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(ActivityHierarchyCrossDocumentOperationOutput {
        updated_documents: payload
            .updated_documents
            .into_iter()
            .map(|document| ActivityHierarchyDocumentOutput {
                source_name: document.source_name,
                updated_toml_content: document.updated_toml_content,
            })
            .collect(),
        replacements: payload
            .replacements
            .into_iter()
            .map(|replacement| ActivityHierarchyCanonicalReplacement {
                old_canonical: replacement.old_canonical,
                new_canonical: replacement.new_canonical,
            })
            .collect(),
        alias_replacements: payload
            .alias_replacements
            .into_iter()
            .map(|replacement| AliasKeyReplacement {
                old_alias: replacement.old_alias,
                new_alias: replacement.new_alias,
            })
            .collect(),
    })
}

pub(crate) fn run_activity_hierarchy_describe(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ActivityHierarchyTree, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<ActivityHierarchyDescribeResponse>(raw, "activity hierarchy")?;
    log_timing("runtime.activity_hierarchy.describe", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(payload.hierarchy)
}
