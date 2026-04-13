use crate::cli::{ReportExportArgs, ReportExportPeriod, ReportFormat};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{
    build_all_matching_export_request, build_recent_batch_export_request,
    build_single_export_request, parse_int_list, reject_argument_when_all,
    require_export_argument, resolve_export_formats,
};
use super::{ReportSession, ReportSessionPort, RuntimeReportSessionPort};

pub struct ExportHandler;

impl CommandHandler<ReportExportArgs> for ExportHandler {
    fn handle(&self, args: ReportExportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_export_with_port(args, ctx, &RuntimeReportSessionPort)
    }
}

pub(crate) fn run_export_with_port(
    args: ReportExportArgs,
    ctx: &CommandContext,
    port: &dyn ReportSessionPort,
) -> Result<(), AppError> {
    let session = port.open("export", ctx)?;
    let formats = resolve_export_formats(&args, session.cli_config());

    for format in &formats {
        for request in build_export_requests(&args, session.as_ref(), format)? {
            session.export(&request)?;
        }
    }
    Ok(())
}

fn build_export_requests(
    args: &ReportExportArgs,
    _session: &dyn ReportSession,
    format: &ReportFormat,
) -> Result<Vec<serde_json::Value>, AppError> {
    if args.all {
        return build_all_export_requests(args, format);
    }

    let raw_argument = require_export_argument(args.period, args.argument.as_deref())?;
    Ok(vec![build_single_export_request(
        args.period,
        raw_argument,
        args.as_of.as_deref(),
        format,
    )?])
}

fn build_all_export_requests(
    args: &ReportExportArgs,
    format: &ReportFormat,
) -> Result<Vec<serde_json::Value>, AppError> {
    match args.period {
        ReportExportPeriod::Day
        | ReportExportPeriod::Month
        | ReportExportPeriod::Week
        | ReportExportPeriod::Year => {
            reject_argument_when_all(args.period, args.argument.as_deref())?;
            Ok(vec![build_all_matching_export_request(args.period, format)?])
        }
        ReportExportPeriod::Range => Err(AppError::InvalidArguments(
            "`report export range --all` is not supported; specify an explicit range."
                .to_string(),
        )),
        ReportExportPeriod::Recent => {
            let argument = require_export_argument(args.period, args.argument.as_deref())?;
            let mut days = parse_int_list(argument)?;
            if days.is_empty() {
                return Err(AppError::InvalidArguments(
                    "`report export recent --all` expects at least one positive integer."
                        .to_string(),
                ));
            }
            if let Some(invalid) = days.iter().find(|days| **days <= 0) {
                return Err(AppError::InvalidArguments(format!(
                    "`report export recent --all` expects positive integers, got `{invalid}`."
                )));
            }
            if let Some(as_of) = args.as_of.as_deref() {
                days.sort_unstable();
                days.dedup();
                // Anchored recent export expands into single temporal exports
                // because batch_recent_list is reserved for rolling recent
                // requests without a fixed anchor window.
                return days
                    .into_iter()
                    .map(|days| {
                        build_single_export_request(
                            ReportExportPeriod::Recent,
                            &days.to_string(),
                            Some(as_of),
                            format,
                        )
                    })
                    .collect();
            }
            Ok(vec![build_recent_batch_export_request(days, format)?])
        }
    }
}
