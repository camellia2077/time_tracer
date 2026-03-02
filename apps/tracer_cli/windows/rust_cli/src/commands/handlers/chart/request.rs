use serde_json::{Value, json};

use crate::cli::ChartArgs;
use crate::error::AppError;

pub(crate) fn build_chart_query_request(args: &ChartArgs) -> Value {
    let mut request = json!({
        "action": "report-chart",
        "output_mode": "semantic_json",
    });

    if let Some(value) = args.root.as_ref() {
        request["root"] = json!(value);
    }
    if let Some(value) = args.year {
        request["year"] = json!(value);
    }
    if let Some(value) = args.month {
        request["month"] = json!(value);
    }
    if let Some(value) = args.from.as_ref() {
        request["from_date"] = json!(value);
    }
    if let Some(value) = args.to.as_ref() {
        request["to_date"] = json!(value);
    }
    if let Some(value) = args.lookback_days {
        request["lookback_days"] = json!(value);
    }

    request
}

pub(crate) fn parse_chart_payload(semantic_payload: &str) -> Result<Value, AppError> {
    serde_json::from_str(semantic_payload)
        .map_err(|e| AppError::Logic(format!("Chart payload is not valid JSON: {e}")))
}
