use serde_json::json;

use crate::cli::{QueryDataArgs, QueryPeriod, SuggestScoreMode};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::{QuerySessionPort, RuntimeQuerySessionPort};

pub struct DataHandler;

impl CommandHandler<QueryDataArgs> for DataHandler {
    fn handle(&self, args: QueryDataArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_data_with_port(args, ctx, &RuntimeQuerySessionPort)
    }
}

pub(crate) fn run_data_with_port(
    args: QueryDataArgs,
    ctx: &CommandContext,
    port: &dyn QuerySessionPort,
) -> Result<(), AppError> {
    let mut request = json!({ "action": args.action });

    if let Some(mode) = args.data_output {
        request["output_mode"] = json!(match mode {
            crate::cli::DataOutputMode::Text => "text",
            crate::cli::DataOutputMode::Json => "json",
        });
    }
    if let Some(v) = args.year {
        request["year"] = json!(v);
    }
    if let Some(v) = args.month {
        request["month"] = json!(v);
    }
    if let Some(v) = args.from {
        request["from_date"] = json!(v);
    }
    if let Some(v) = args.to {
        request["to_date"] = json!(v);
    }
    if let Some(v) = args.remark {
        request["remark"] = json!(v);
    }
    if let Some(v) = args.day_remark {
        request["day_remark"] = json!(v);
    }
    if let Some(v) = args.root {
        request["root"] = json!(v);
    }
    if args.overnight {
        request["overnight"] = json!(true);
    }
    if let Some(v) = args.exercise {
        request["exercise"] = json!(v);
    }
    if let Some(v) = args.status {
        request["status"] = json!(v);
    }
    if let Some(v) = args.numbers {
        request["limit"] = json!(v);
    }
    if let Some(v) = args.top {
        request["top_n"] = json!(v);
    }
    if let Some(v) = args.lookback_days {
        request["lookback_days"] = json!(v);
    }
    if let Some(v) = args.activity_prefix {
        request["activity_prefix"] = json!(v);
    }
    if let Some(v) = args.score_mode {
        request["activity_score_by_duration"] = json!(matches!(v, SuggestScoreMode::Duration));
    }
    if let Some(v) = args.period {
        request["tree_period"] = json!(query_period_token(v));
    }
    if let Some(v) = args.period_arg {
        request["tree_period_argument"] = json!(v);
    }
    if let Some(v) = args.level {
        request["tree_max_depth"] = json!(v);
    }
    if args.reverse {
        request["reverse"] = json!(true);
    }

    let content = port.data("query", ctx, &request)?;
    print!("{content}");
    Ok(())
}

fn query_period_token(value: QueryPeriod) -> &'static str {
    match value {
        QueryPeriod::Day => "day",
        QueryPeriod::Week => "week",
        QueryPeriod::Month => "month",
        QueryPeriod::Year => "year",
        QueryPeriod::Recent => "recent",
        QueryPeriod::Range => "range",
    }
}
