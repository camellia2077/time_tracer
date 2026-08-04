use std::time::Instant;

use serde_json::Value;

use crate::error::AppError;

use super::super::env_flags::log_timing;
use super::super::errors::format_tree_error_detail;
use super::super::CoreRuntime;
use super::super::TreeResponse;
use super::responses::SemanticTreeResponse;
use super::transport::run_text;

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
