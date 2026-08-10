use serde_json::{json, Value};

use crate::cli::{InsightsExportPeriod, InsightsFormat, InsightsRenderPeriod};
use crate::error::AppError;

use super::dates::{
    month_to_range, normalize_day_argument, normalize_recent_argument, normalize_year_argument,
    split_normalized_range, week_to_range,
};
use super::formats::{export_period_token, format_token};

pub(crate) fn build_render_request(
    period: InsightsRenderPeriod,
    argument: &str,
    as_of: Option<&str>,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    ensure_as_of_is_recent_render(period, as_of)?;

    if matches!(period, InsightsRenderPeriod::Recent) {
        let days = parse_positive_int_list("`insights render recent`", argument)?;
        if days.len() == 1 {
            // `--as-of` maps to canonical `anchor_date`; the CLI no longer rewrites
            // recent requests into range payloads.
            return Ok(build_recent_query_request(days[0], as_of, format)?);
        }
        if as_of.is_some() {
            return Err(AppError::InvalidArguments(
                "`insights render recent --as-of` currently requires a single days value (for example `7`)."
                    .to_string(),
            ));
        }
        return Ok(json!({
            "days_list": days,
            "format": format_token(format),
        }));
    }

    build_temporal_query_request(period, argument, format)
}

pub(crate) fn build_export_render_request(
    period: InsightsExportPeriod,
    argument: &str,
    as_of: Option<&str>,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    ensure_as_of_is_recent_export(period, as_of)?;

    build_single_export_request(period, argument, as_of, format)
}

pub(crate) fn require_export_argument(
    period: InsightsExportPeriod,
    argument: Option<&str>,
) -> Result<&str, AppError> {
    let Some(argument) = argument else {
        return Err(AppError::InvalidArguments(format!(
            "`insights export {}` requires <argument>.",
            export_period_token(period)
        )));
    };
    if argument.trim().is_empty() {
        return Err(AppError::InvalidArguments(format!(
            "`insights export {}` requires a non-empty <argument>.",
            export_period_token(period)
        )));
    }
    Ok(argument)
}

pub(crate) fn reject_argument_when_all(
    period: InsightsExportPeriod,
    argument: Option<&str>,
) -> Result<(), AppError> {
    if let Some(argument) = argument {
        if !argument.trim().is_empty() {
            return Err(AppError::InvalidArguments(format!(
                "`insights export {}` does not accept <argument> when --all is set.",
                export_period_token(period)
            )));
        }
    }
    Ok(())
}

fn ensure_as_of_is_recent_render(
    period: InsightsRenderPeriod,
    as_of: Option<&str>,
) -> Result<(), AppError> {
    if as_of.is_some() && !matches!(period, InsightsRenderPeriod::Recent) {
        return Err(AppError::InvalidArguments(
            "`--as-of` is supported only for `insights render recent`.".to_string(),
        ));
    }
    Ok(())
}

fn ensure_as_of_is_recent_export(
    period: InsightsExportPeriod,
    as_of: Option<&str>,
) -> Result<(), AppError> {
    if as_of.is_some() && !matches!(period, InsightsExportPeriod::Recent) {
        return Err(AppError::InvalidArguments(
            "`--as-of` is supported only for `insights export recent`.".to_string(),
        ));
    }
    Ok(())
}

fn build_temporal_query_request(
    period: InsightsRenderPeriod,
    argument: &str,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    match period {
        InsightsRenderPeriod::Day => Ok(json!({
            "operation_kind": "query",
            "display_mode": "day",
            "selection_kind": "single_day",
            "date": normalize_day_argument("`insights render day`", argument)?,
            "format": format_token(format),
        })),
        InsightsRenderPeriod::Month => {
            let (start_date, end_date) = month_to_range("`insights render month`", argument)?;
            Ok(json!({
                "operation_kind": "query",
                "display_mode": "month",
                "selection_kind": "date_range",
                "start_date": start_date,
                "end_date": end_date,
                "format": format_token(format),
            }))
        }
        InsightsRenderPeriod::Week => {
            let (start_date, end_date) = week_to_range("`insights render week`", argument)?;
            Ok(json!({
                "operation_kind": "query",
                "display_mode": "week",
                "selection_kind": "date_range",
                "start_date": start_date,
                "end_date": end_date,
                "format": format_token(format),
            }))
        }
        InsightsRenderPeriod::Year => {
            let year = normalize_year_argument("`insights render year`", argument)?;
            Ok(json!({
                "operation_kind": "query",
                "display_mode": "year",
                "selection_kind": "date_range",
                "start_date": format!("{year}-01-01"),
                "end_date": format!("{year}-12-31"),
                "format": format_token(format),
            }))
        }
        InsightsRenderPeriod::Range => {
            let (start_date, end_date) = split_normalized_range("`insights render range`", argument)?;
            Ok(json!({
                "operation_kind": "query",
                "display_mode": "range",
                "selection_kind": "date_range",
                "start_date": start_date,
                "end_date": end_date,
                "format": format_token(format),
            }))
        }
        InsightsRenderPeriod::Recent => unreachable!("recent is handled separately"),
    }
}

fn build_recent_query_request(
    days: i32,
    as_of: Option<&str>,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    if days <= 0 {
        return Err(AppError::InvalidArguments(
            "`insights render recent` expects a positive integer.".to_string(),
        ));
    }
    let mut request = json!({
        "operation_kind": "query",
        "display_mode": "recent",
        "selection_kind": "recent_days",
        "days": days,
        "format": format_token(format),
    });
    if let Some(as_of_value) = as_of {
        request["anchor_date"] = json!(normalize_day_argument("`--as-of`", as_of_value)?);
    }
    Ok(request)
}

pub(crate) fn build_single_export_request(
    period: InsightsExportPeriod,
    argument: &str,
    as_of: Option<&str>,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    match period {
        InsightsExportPeriod::Day => Ok(json!({
            "operation_kind": "export",
            "display_mode": "day",
            "export_scope": "single",
            "selection_kind": "single_day",
            "date": normalize_day_argument("`insights export day`", argument)?,
            "format": format_token(format),
        })),
        InsightsExportPeriod::Month => {
            let (start_date, end_date) = month_to_range("`insights export month`", argument)?;
            Ok(json!({
                "operation_kind": "export",
                "display_mode": "month",
                "export_scope": "single",
                "selection_kind": "date_range",
                "start_date": start_date,
                "end_date": end_date,
                "format": format_token(format),
            }))
        }
        InsightsExportPeriod::Week => {
            let (start_date, end_date) = week_to_range("`insights export week`", argument)?;
            Ok(json!({
                "operation_kind": "export",
                "display_mode": "week",
                "export_scope": "single",
                "selection_kind": "date_range",
                "start_date": start_date,
                "end_date": end_date,
                "format": format_token(format),
            }))
        }
        InsightsExportPeriod::Year => {
            let year = normalize_year_argument("`insights export year`", argument)?;
            Ok(json!({
                "operation_kind": "export",
                "display_mode": "year",
                "export_scope": "single",
                "selection_kind": "date_range",
                "start_date": format!("{year}-01-01"),
                "end_date": format!("{year}-12-31"),
                "format": format_token(format),
            }))
        }
        InsightsExportPeriod::Recent => {
            let days = normalize_recent_argument("`insights export recent`", argument)?
                .parse::<i32>()
                .map_err(|error| {
                    AppError::InvalidArguments(format!(
                        "`insights export recent` expects a positive integer, got `{argument}`: {error}"
                    ))
                })?;
            let mut request = json!({
                "operation_kind": "export",
                "display_mode": "recent",
                "export_scope": "single",
                "selection_kind": "recent_days",
                "days": days,
                "format": format_token(format),
            });
            if let Some(as_of_value) = as_of {
                // `--as-of` maps directly to canonical `anchor_date`; export does
                // not rewrite anchored recent into a range request anymore.
                request["anchor_date"] = json!(normalize_day_argument("`--as-of`", as_of_value)?);
            }
            Ok(request)
        }
        InsightsExportPeriod::Range => {
            let (start_date, end_date) = split_normalized_range("`insights export range`", argument)?;
            Ok(json!({
                "operation_kind": "export",
                "display_mode": "range",
                "export_scope": "single",
                "selection_kind": "date_range",
                "start_date": start_date,
                "end_date": end_date,
                "format": format_token(format),
            }))
        }
    }
}

pub(crate) fn build_all_matching_export_request(
    period: InsightsExportPeriod,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    let display_mode = list_targets_type(period)?;
    Ok(json!({
        "operation_kind": "export",
        "display_mode": display_mode,
        "export_scope": "all_matching",
        "format": format_token(format),
    }))
}

pub(crate) fn build_recent_batch_export_request(
    days: Vec<i32>,
    format: &InsightsFormat,
) -> Result<Value, AppError> {
    if days.is_empty() {
        return Err(AppError::InvalidArguments(
            "`insights export recent --all` expects at least one positive integer.".to_string(),
        ));
    }
    Ok(json!({
        "operation_kind": "export",
        "display_mode": "recent",
        "export_scope": "batch_recent_list",
        "recent_days_list": days,
        "format": format_token(format),
    }))
}

pub(crate) fn parse_int_list(value: &str) -> Result<Vec<i32>, AppError> {
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

fn parse_positive_int_list(command_label: &str, value: &str) -> Result<Vec<i32>, AppError> {
    let out = parse_int_list(value)?;
    if out.is_empty() {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects a positive integer."
        )));
    }
    if let Some(invalid) = out.iter().find(|days| **days <= 0) {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects positive integers, got `{invalid}`."
        )));
    }
    Ok(out)
}

pub(crate) fn list_targets_type(period: InsightsExportPeriod) -> Result<&'static str, AppError> {
    match period {
        InsightsExportPeriod::Day => Ok("day"),
        InsightsExportPeriod::Month => Ok("month"),
        InsightsExportPeriod::Week => Ok("week"),
        InsightsExportPeriod::Year => Ok("year"),
        InsightsExportPeriod::Recent => Err(AppError::InvalidArguments(
            "`insights export recent --all` must enumerate days locally.".to_string(),
        )),
        InsightsExportPeriod::Range => Err(AppError::InvalidArguments(
            "`insights export range --all` is not supported; specify an explicit range.".to_string(),
        )),
    }
}

#[cfg(test)]
mod tests {
    use std::path::Path;

    use crate::cli::{InsightsExportPeriod, InsightsFormat, InsightsRenderPeriod};

    use super::super::formats::{build_export_output_path, normalize_export_name};
    use super::{build_export_render_request, build_render_request};

    #[test]
    fn normalize_month_accepts_compact_and_dashed_input() {
        assert_eq!(
            normalize_export_name(InsightsExportPeriod::Month, "202603").expect("compact month"),
            "2026-03"
        );
        assert_eq!(
            normalize_export_name(InsightsExportPeriod::Month, "2026-03").expect("dashed month"),
            "2026-03"
        );
    }

    #[test]
    fn build_month_export_output_path_uses_dashed_filename() {
        let path = build_export_output_path(
            Path::new("C:/tmp/out"),
            &InsightsFormat::Md,
            InsightsExportPeriod::Month,
            "2026-03",
        )
        .expect("month path");
        assert_eq!(
            path,
            Path::new("C:/tmp/out")
                .join("markdown")
                .join("month")
                .join("2026-03.md")
        );
    }

    #[test]
    fn build_render_request_normalizes_compact_day_and_month_input() {
        let day_request =
            build_render_request(InsightsRenderPeriod::Day, "20260103", None, &InsightsFormat::Md)
                .expect("day request");
        assert_eq!(day_request["operation_kind"], "query");
        assert_eq!(day_request["display_mode"], "day");
        assert_eq!(day_request["selection_kind"], "single_day");
        assert_eq!(day_request["date"], "2026-01-03");

        let month_request =
            build_render_request(InsightsRenderPeriod::Month, "202603", None, &InsightsFormat::Md)
                .expect("month request");
        assert_eq!(month_request["operation_kind"], "query");
        assert_eq!(month_request["display_mode"], "month");
        assert_eq!(month_request["selection_kind"], "date_range");
        assert_eq!(month_request["start_date"], "2026-03-01");
        assert_eq!(month_request["end_date"], "2026-03-31");
    }

    #[test]
    fn build_render_request_normalizes_range_and_single_recent_input() {
        let range_request = build_render_request(
            InsightsRenderPeriod::Range,
            "20260101|20260131",
            None,
            &InsightsFormat::Md,
        )
        .expect("range request");
        assert_eq!(range_request["operation_kind"], "query");
        assert_eq!(range_request["display_mode"], "range");
        assert_eq!(range_request["selection_kind"], "date_range");
        assert_eq!(range_request["start_date"], "2026-01-01");
        assert_eq!(range_request["end_date"], "2026-01-31");

        let recent_request =
            build_render_request(InsightsRenderPeriod::Recent, "7", None, &InsightsFormat::Md)
                .expect("recent request");
        assert_eq!(recent_request["operation_kind"], "query");
        assert_eq!(recent_request["display_mode"], "recent");
        assert_eq!(recent_request["selection_kind"], "recent_days");
        assert_eq!(recent_request["days"], 7);
    }

    #[test]
    fn build_render_request_recent_with_as_of_uses_anchor_date() {
        let recent_request = build_render_request(
            InsightsRenderPeriod::Recent,
            "7",
            Some("2026-03-07"),
            &InsightsFormat::Md,
        )
        .expect("recent as-of request");
        // Anchored recent stays on the canonical recent contract and adds anchor_date
        // instead of rewriting the request into a synthetic range payload.
        assert_eq!(recent_request["operation_kind"], "query");
        assert_eq!(recent_request["display_mode"], "recent");
        assert_eq!(recent_request["selection_kind"], "recent_days");
        assert_eq!(recent_request["days"], 7);
        assert_eq!(recent_request["anchor_date"], "2026-03-07");
    }

    #[test]
    fn build_render_request_rejects_as_of_for_non_recent() {
        let error = build_render_request(
            InsightsRenderPeriod::Range,
            "2026-03-01|2026-03-07",
            Some("2026-03-07"),
            &InsightsFormat::Md,
        )
        .expect_err("non-recent as-of should fail");
        assert!(error
            .to_string()
            .contains("`--as-of` is supported only for `insights render recent`"));
    }

    #[test]
    fn build_export_render_request_normalizes_compact_day_and_month_input() {
        let day_request = build_export_render_request(
            InsightsExportPeriod::Day,
            "20260103",
            None,
            &InsightsFormat::Md,
        )
        .expect("day request");
        assert_eq!(day_request["operation_kind"], "export");
        assert_eq!(day_request["display_mode"], "day");
        assert_eq!(day_request["selection_kind"], "single_day");
        assert_eq!(day_request["date"], "2026-01-03");

        let month_request = build_export_render_request(
            InsightsExportPeriod::Month,
            "202603",
            None,
            &InsightsFormat::Md,
        )
        .expect("month request");
        assert_eq!(month_request["operation_kind"], "export");
        assert_eq!(month_request["display_mode"], "month");
        assert_eq!(month_request["selection_kind"], "date_range");
        assert_eq!(month_request["start_date"], "2026-03-01");
        assert_eq!(month_request["end_date"], "2026-03-31");
    }

    #[test]
    fn build_export_render_request_recent_with_as_of_uses_anchor_date() {
        let recent_request = build_export_render_request(
            InsightsExportPeriod::Recent,
            "7",
            Some("2026-03-07"),
            &InsightsFormat::Md,
        )
        .expect("recent as-of export request");
        assert_eq!(recent_request["operation_kind"], "export");
        assert_eq!(recent_request["display_mode"], "recent");
        assert_eq!(recent_request["selection_kind"], "recent_days");
        assert_eq!(recent_request["days"], 7);
        assert_eq!(recent_request["anchor_date"], "2026-03-07");
    }
}
