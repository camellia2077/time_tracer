use serde_json::json;

use crate::cli::{QueryArgs, QueryFormat, QueryPeriod, QueryType, SuggestScoreMode};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct QueryHandler;

impl CommandHandler<QueryArgs> for QueryHandler {
    fn handle(&self, args: QueryArgs, ctx: &CommandContext) -> Result<(), AppError> {
        if args.project.is_some() {
            eprintln!("Warning: `--project` is deprecated, prefer `--root`.");
        }

        let api = CoreApi::load()?;
        let (runtime, cli_config) = api.bootstrap("query", ctx)?;
        let formats = resolve_query_formats(
            &args,
            &cli_config.command_defaults.query_format,
            &cli_config.defaults.default_format,
        );

        match args.query_type {
            QueryType::Data => {
                let mut request = json!({ "action": args.argument });
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
                if let Some(v) = args.project {
                    request["project"] = json!(v);
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
                    request["activity_score_by_duration"] =
                        json!(matches!(v, SuggestScoreMode::Duration));
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

                let content = runtime.run_query(&request)?;
                print!("{content}");
                return Ok(());
            }
            QueryType::Recent => {
                let days = parse_int_list(&args.argument)?;
                for (index, format) in formats.iter().enumerate() {
                    if index > 0 {
                        println!("\n{}", "=".repeat(40));
                    }
                    let request = json!({
                        "days_list": days,
                        "format": query_format_token(format),
                    });
                    let content = runtime.run_report_batch(&request)?;
                    print!("{content}");
                }
            }
            other => {
                let query_type = query_type_token(&other);
                for (index, format) in formats.iter().enumerate() {
                    if index > 0 {
                        println!("\n{}", "=".repeat(40));
                    }
                    let request = json!({
                        "type": query_type,
                        "argument": &args.argument,
                        "format": query_format_token(format),
                    });
                    let content = runtime.run_report(&request)?;
                    print!("{content}");
                }
            }
        }
        Ok(())
    }
}

fn resolve_query_formats(
    args: &QueryArgs,
    command_default: &Option<String>,
    global_default: &Option<String>,
) -> Vec<QueryFormat> {
    if !args.format.is_empty() {
        return args.format.clone();
    }
    if let Some(value) = command_default {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    if let Some(value) = global_default {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    vec![QueryFormat::Md]
}

fn parse_format_tokens(value: &str) -> Vec<QueryFormat> {
    value
        .split(',')
        .filter_map(|token| match token.trim().to_ascii_lowercase().as_str() {
            "md" => Some(QueryFormat::Md),
            "tex" => Some(QueryFormat::Tex),
            "typ" => Some(QueryFormat::Typ),
            _ => None,
        })
        .collect()
}

fn parse_int_list(value: &str) -> Result<Vec<i32>, AppError> {
    let mut out = Vec::new();
    for token in value.split(',') {
        let t = token.trim();
        if t.is_empty() {
            continue;
        }
        let n = t.parse::<i32>().map_err(|e| {
            AppError::InvalidArguments(format!("Invalid integer in list `{value}`: {e}"))
        })?;
        out.push(n);
    }
    Ok(out)
}

fn query_type_token(value: &QueryType) -> &'static str {
    match value {
        QueryType::Day => "day",
        QueryType::Month => "month",
        QueryType::Week => "week",
        QueryType::Year => "year",
        QueryType::Recent => "recent",
        QueryType::Range => "range",
        QueryType::Data => "data",
    }
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

fn query_format_token(value: &QueryFormat) -> &'static str {
    match value {
        QueryFormat::Md => "md",
        QueryFormat::Tex => "tex",
        QueryFormat::Typ => "typ",
    }
}
