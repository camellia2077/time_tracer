use std::fs;
use std::path::Path;

use serde_json::json;

use crate::cli::TxtAppendEventArgs;
use crate::commands::handler::CommandContext;
use crate::error::AppError;

use super::{format, TxtSessionPort};

pub(crate) fn run(
    args: TxtAppendEventArgs,
    ctx: &CommandContext,
    port: &dyn TxtSessionPort,
) -> Result<String, AppError> {
    let input_path = Path::new(&args.input);
    let content = fs::read_to_string(input_path)
        .map_err(|error| AppError::Io(format!("Read input txt failed: {error}")))?;
    let selected_month = format::infer_selected_month_from_path(input_path);
    let resolve_request = json!({
        "action": "resolve_day_block",
        "content": content,
        "day_marker": args.day,
        "selected_month": selected_month,
    });
    let resolved = port.resolve_day_block("txt", &ctx.without_output(), &resolve_request)?;
    if !resolved.is_marker_valid {
        return Err(AppError::InvalidArguments(
            "`--day` must be a valid MMDD marker.".to_string(),
        ));
    }
    if !resolved.found {
        return Err(AppError::Logic(format!(
            "Day block {} was not found in {}.",
            resolved.normalized_day_marker,
            input_path.display()
        )));
    }

    let event_line = format::build_authored_event_line(&args)?;
    let edited_day_body = format::append_event_to_day_body(&resolved.day_body, &event_line);
    let replace_request = json!({
        "action": "replace_day_block",
        "content": content,
        "day_marker": args.day,
        "edited_day_body": edited_day_body,
    });
    let replaced = port.replace_day_block("txt", &ctx.without_output(), &replace_request)?;
    if !replaced.is_marker_valid {
        return Err(AppError::InvalidArguments(
            "`--day` must be a valid MMDD marker.".to_string(),
        ));
    }
    if !replaced.found {
        return Err(AppError::Logic(format!(
            "Day block {} was not found in {}.",
            replaced.normalized_day_marker,
            input_path.display()
        )));
    }

    fs::write(input_path, &replaced.updated_content)
        .map_err(|error| AppError::Io(format!("Write updated txt failed: {error}")))?;
    Ok(format!(
        "Appended event to {} in {}:\n{}\n",
        replaced.normalized_day_marker,
        input_path.display(),
        event_line
    ))
}
