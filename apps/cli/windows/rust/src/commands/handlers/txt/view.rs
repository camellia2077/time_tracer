use std::fs;
use std::path::Path;

use serde_json::json;

use crate::cli::TxtViewDayArgs;
use crate::commands::handler::CommandContext;
use crate::error::AppError;

use super::{format, TxtSessionPort};

pub(crate) fn run(
    args: TxtViewDayArgs,
    ctx: &CommandContext,
    port: &dyn TxtSessionPort,
) -> Result<String, AppError> {
    let input_path = Path::new(&args.input);
    let content = fs::read_to_string(input_path)
        .map_err(|error| AppError::Io(format!("Read input txt failed: {error}")))?;
    let selected_month = format::infer_selected_month_from_path(input_path);
    let request = json!({
        "action": "resolve_day_block",
        "content": content,
        "day_marker": args.day,
        "selected_month": selected_month,
    });
    let resolved = port.resolve_day_block("txt", &ctx.without_output(), &request)?;
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
    Ok(resolved.day_body)
}
