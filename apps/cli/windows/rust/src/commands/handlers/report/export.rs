use std::fs;
use std::path::{Path, PathBuf};

use serde_json::Value;

use crate::cli::{ReportExportArgs, ReportExportPeriod, ReportFormat};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{
    build_export_output_path, build_export_render_request, list_targets_type,
    normalize_export_name, parse_int_list, reject_argument_when_all, require_export_argument,
    resolve_export_formats,
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
        for export in build_export_plan(&args, session.as_ref(), format)? {
            let content = session.render(&export.render_request)?;
            write_report_file(&export.output_path, &content)?;
        }
    }
    Ok(())
}

struct PlannedExport {
    render_request: Value,
    output_path: PathBuf,
}

fn build_export_plan(
    args: &ReportExportArgs,
    session: &dyn ReportSession,
    format: &ReportFormat,
) -> Result<Vec<PlannedExport>, AppError> {
    if args.all {
        return build_all_export_plan(args, session, format);
    }

    let raw_argument = require_export_argument(args.period, args.argument.as_deref())?;
    let normalized_id = normalize_export_name(args.period, raw_argument)?;
    Ok(vec![PlannedExport {
        render_request: build_export_render_request(args.period, raw_argument, format),
        output_path: build_export_output_path(
            session.runtime_output_root(),
            format,
            args.period,
            &normalized_id,
        )?,
    }])
}

fn build_all_export_plan(
    args: &ReportExportArgs,
    session: &dyn ReportSession,
    format: &ReportFormat,
) -> Result<Vec<PlannedExport>, AppError> {
    match args.period {
        ReportExportPeriod::Day
        | ReportExportPeriod::Month
        | ReportExportPeriod::Week
        | ReportExportPeriod::Year => {
            reject_argument_when_all(args.period, args.argument.as_deref())?;
            let target_type = list_targets_type(args.period)?;
            session
                .list_targets(target_type)?
                .into_iter()
                .map(|canonical_id| {
                    Ok(PlannedExport {
                        render_request: build_export_render_request(
                            args.period,
                            &canonical_id,
                            format,
                        ),
                        output_path: build_export_output_path(
                            session.runtime_output_root(),
                            format,
                            args.period,
                            &canonical_id,
                        )?,
                    })
                })
                .collect()
        }
        ReportExportPeriod::Recent => {
            let argument = require_export_argument(args.period, args.argument.as_deref())?;
            parse_int_list(argument)?
                .into_iter()
                .map(|days| {
                    if days <= 0 {
                        return Err(AppError::InvalidArguments(format!(
                            "`report export recent --all` expects positive integers, got `{days}`."
                        )));
                    }
                    let normalized_id = days.to_string();
                    Ok(PlannedExport {
                        render_request: build_export_render_request(
                            args.period,
                            &normalized_id,
                            format,
                        ),
                        output_path: build_export_output_path(
                            session.runtime_output_root(),
                            format,
                            args.period,
                            &normalized_id,
                        )?,
                    })
                })
                .collect()
        }
    }
}

fn write_report_file(output_path: &Path, content: &str) -> Result<(), AppError> {
    if let Some(parent) = output_path.parent() {
        fs::create_dir_all(parent).map_err(|error| {
            AppError::Io(format!(
                "Create export directory `{}` failed: {error}",
                parent.display()
            ))
        })?;
    }
    fs::write(output_path, content.as_bytes()).map_err(|error| {
        AppError::Io(format!(
            "Write export report `{}` failed: {error}",
            output_path.display()
        ))
    })
}
