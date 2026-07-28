use std::time::Instant;

use serde::Deserialize;
use serde_json::{Value, json};

use crate::error::{AppError, AppExitCode};

use super::codec::{read_c_json, to_request_json};
use super::env_flags::log_timing;
use super::errors::{ErrorContract, format_error_detail, format_tree_error_detail};
use super::ffi::RuntimeJsonFn;
use super::{
    AliasHierarchyCanonicalReplacement, AliasHierarchyOperationOutput, AliasKeyReplacement,
    CoreRuntime, TreeResponse,
    TxtCanonicalReplaceOutput, TxtReplaceOutput, TxtResolveOutput,
};

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct ReportWindowMetadata {
    pub(crate) has_records: bool,
    pub(crate) matched_day_count: i32,
    pub(crate) matched_record_count: i32,
    pub(crate) start_date: String,
    pub(crate) end_date: String,
    pub(crate) requested_days: i32,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct ReportTextOutput {
    pub(crate) content: String,
    pub(crate) report_window_metadata: Option<ReportWindowMetadata>,
}

#[derive(Deserialize)]
struct AckResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct TextResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    content: String,
    #[serde(default)]
    has_records: Option<bool>,
    #[serde(default)]
    matched_day_count: Option<i32>,
    #[serde(default)]
    matched_record_count: Option<i32>,
    #[serde(default)]
    start_date: Option<String>,
    #[serde(default)]
    end_date: Option<String>,
    #[serde(default)]
    requested_days: Option<i32>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct ReportTargetsResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    items: Vec<String>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct TxtResolveResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    normalized_day_marker: String,
    #[serde(default)]
    found: bool,
    #[serde(default)]
    is_marker_valid: bool,
    #[serde(default)]
    can_save: bool,
    #[serde(default)]
    day_body: String,
    #[serde(default)]
    day_content_iso_date: Option<String>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct SemanticTreeResponse {
    #[serde(default)]
    roots: Vec<super::TreeNode>,
}

#[derive(Deserialize)]
struct TxtReplaceResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    normalized_day_marker: String,
    #[serde(default)]
    found: bool,
    #[serde(default)]
    is_marker_valid: bool,
    #[serde(default)]
    updated_content: String,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct AliasCanonicalReplacementResponse {
    old_canonical: String,
    new_canonical: String,
}

#[derive(Deserialize)]
struct AliasHierarchyOperationResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    updated_toml_content: String,
    #[serde(default)]
    replacements: Vec<AliasCanonicalReplacementResponse>,
    #[serde(default)]
    alias_replacements: Vec<AliasKeyReplacementResponse>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct AliasHierarchyTextResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    content: String,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct AliasKeyReplacementResponse {
    old_alias: String,
    new_alias: String,
}

pub(crate) fn run_query_data(runtime: &CoreRuntime, request: &Value) -> Result<String, AppError> {
    let payload = run_text(runtime, runtime.api.symbols.runtime_query, request, "query")?;
    Ok(payload.content)
}

pub(crate) fn run_report_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ReportTextOutput, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report,
        request,
        "report",
    )?;
    Ok(payload.into_report_text_output())
}

pub(crate) fn run_report_batch_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ReportTextOutput, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report_batch,
        request,
        "report_batch",
    )?;
    Ok(payload.into_report_text_output())
}

pub(crate) fn run_report_targets(
    runtime: &CoreRuntime,
    display_mode: &str,
) -> Result<Vec<String>, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(&json!({
        "operation_kind": "targets",
        "display_mode": display_mode,
    }))?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_report)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<ReportTargetsResponse>(raw, "report_targets")?;
    log_timing("runtime.report_targets", run_start.elapsed());
    if payload.ok {
        return Ok(payload.items);
    }
    Err(map_runtime_text_error(
        payload.error_message,
        &payload.error_contract,
    ))
}

pub(crate) fn run_report_export(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_report,
        request,
        "report_export",
    )
}

pub(crate) fn run_pipeline_convert(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_convert,
        request,
        "convert",
    )
}

pub(crate) fn run_pipeline_import(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_import,
        request,
        "import",
    )
}

pub(crate) fn run_pipeline_ingest(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_ingest,
        request,
        "ingest",
    )
}

pub(crate) fn run_pipeline_validate_structure(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_validate_structure,
        request,
        "validate_structure",
    )
}

pub(crate) fn run_pipeline_validate_logic(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_validate_logic,
        request,
        "validate_logic",
    )
}

pub(crate) fn run_tree_query(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TreeResponse, AppError> {
    let run_start = Instant::now();
    let mut semantic_request = request.clone();
    let request_object = semantic_request
        .as_object_mut()
        .ok_or_else(|| AppError::Logic("tree request must be an object.".to_string()))?;
    request_object.insert("action".to_string(), Value::String("tree".to_string()));
    request_object.insert(
        "output_mode".to_string(),
        Value::String("semantic_json".to_string()),
    );
    if let Some(max_depth) = request_object.remove("max_depth") {
        request_object.insert("tree_max_depth".to_string(), max_depth);
    }
    if let Some(root_pattern) = request_object.remove("root_pattern") {
        request_object.insert("root".to_string(), root_pattern);
    }
    request_object.remove("list_roots");
    let text_payload = run_text(
        runtime,
        runtime.api.symbols.runtime_query,
        &semantic_request,
        "tree",
    )?;
    let semantic: SemanticTreeResponse = serde_json::from_str(&text_payload.content)
        .map_err(|error| AppError::Logic(format!("tree response decode failed: {error}")))?;
    let roots = semantic
        .roots
        .iter()
        .map(|node| node.name.clone())
        .collect();
    let payload = TreeResponse {
        ok: true,
        found: !semantic.roots.is_empty(),
        error_message: String::new(),
        roots,
        nodes: semantic.roots,
        error_code: String::new(),
        error_category: String::new(),
        hints: Vec::new(),
    };
    log_timing("runtime.tree", run_start.elapsed());
    if payload.ok {
        return Ok(payload);
    }
    Err(AppError::Logic(format_tree_error_detail(&payload)))
}

pub(crate) fn run_txt_resolve_day_block(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtResolveOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtResolveResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtResolveOutput {
        normalized_day_marker: payload.normalized_day_marker,
        found: payload.found,
        is_marker_valid: payload.is_marker_valid,
        can_save: payload.can_save,
        day_body: payload.day_body,
        day_content_iso_date: payload.day_content_iso_date,
    })
}

pub(crate) fn run_txt_replace_day_block(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtReplaceOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtReplaceResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtReplaceOutput {
        normalized_day_marker: payload.normalized_day_marker,
        found: payload.found,
        is_marker_valid: payload.is_marker_valid,
        updated_content: payload.updated_content,
    })
}

pub(crate) fn run_txt_replace_canonical_activity_names(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtCanonicalReplaceOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtReplaceResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtCanonicalReplaceOutput {
        updated_content: payload.updated_content,
    })
}

pub(crate) fn run_alias_hierarchy_operation(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<AliasHierarchyOperationOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<AliasHierarchyOperationResponse>(raw, "alias hierarchy")?;
    log_timing("runtime.alias_hierarchy", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(AliasHierarchyOperationOutput {
        updated_toml_content: payload.updated_toml_content,
        replacements: payload
            .replacements
            .into_iter()
            .map(|replacement| AliasHierarchyCanonicalReplacement {
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

pub(crate) fn run_alias_hierarchy_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<AliasHierarchyTextResponse>(raw, "alias hierarchy")?;
    log_timing("runtime.alias_hierarchy", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(payload.content)
}

pub(crate) fn run_tracer_exchange_export(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_encrypt,
        request,
        "exchange_export",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_tracer_exchange_import(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_decrypt,
        request,
        "exchange_import",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_tracer_exchange_unpack(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_unpack,
        request,
        "exchange_unpack",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_tracer_exchange_inspect(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_inspect,
        request,
        "exchange_inspect",
    )?;
    Ok(payload.content)
}

fn run_ack(
    runtime: &CoreRuntime,
    function: RuntimeJsonFn,
    request: &Value,
    context: &str,
) -> Result<(), AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { function(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<AckResponse>(raw, context)?;
    log_timing(&format!("runtime.{context}"), run_start.elapsed());
    if payload.ok {
        return Ok(());
    }
    Err(AppError::Logic(format_error_detail(
        payload.error_message,
        &payload.error_contract,
    )))
}

fn run_text(
    runtime: &CoreRuntime,
    function: RuntimeJsonFn,
    request: &Value,
    context: &str,
) -> Result<TextResponse, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { function(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TextResponse>(raw, context)?;
    log_timing(&format!("runtime.{context}"), run_start.elapsed());
    if payload.ok {
        return Ok(payload);
    }
    Err(map_runtime_text_error(
        payload.error_message,
        &payload.error_contract,
    ))
}

#[cfg(test)]
mod tests {
    use crate::error::AppExitCode;

    use super::{ErrorContract, map_runtime_text_error};

    #[test]
    fn reporting_target_not_found_maps_to_dedicated_exit_code() {
        let error = map_runtime_text_error(
            "Report target not found: day `2024-12-31`.".to_string(),
            &ErrorContract {
                error_code: "reporting.target.not_found".to_string(),
                error_category: "reporting".to_string(),
                hints: vec![
                    "Check that the requested report target exists in the current database."
                        .to_string(),
                ],
            },
        );

        assert!(matches!(
            error,
            crate::error::AppError::Plain {
                code: AppExitCode::ReportTargetNotFound,
                ..
            }
        ));
        assert!(
            error
                .render_for_stderr()
                .contains("reporting.target.not_found")
        );
    }
}

fn map_runtime_text_error(error_message: String, contract: &ErrorContract) -> AppError {
    let detail = format_error_detail(error_message, contract);
    if contract.error_code == "reporting.target.not_found" {
        return AppError::Plain {
            message: detail,
            code: AppExitCode::ReportTargetNotFound,
        };
    }
    AppError::Logic(detail)
}

impl TextResponse {
    fn into_report_text_output(self) -> ReportTextOutput {
        let report_window_metadata = self.report_window_metadata();
        ReportTextOutput {
            content: self.content,
            report_window_metadata,
        }
    }

    fn report_window_metadata(&self) -> Option<ReportWindowMetadata> {
        let has_any = self.has_records.is_some()
            || self.matched_day_count.is_some()
            || self.matched_record_count.is_some()
            || self.start_date.is_some()
            || self.end_date.is_some()
            || self.requested_days.is_some();
        if !has_any {
            return None;
        }

        Some(ReportWindowMetadata {
            has_records: self.has_records.unwrap_or(false),
            matched_day_count: self.matched_day_count.unwrap_or(0),
            matched_record_count: self.matched_record_count.unwrap_or(0),
            start_date: self.start_date.clone().unwrap_or_default(),
            end_date: self.end_date.clone().unwrap_or_default(),
            requested_days: self.requested_days.unwrap_or(0),
        })
    }
}
