pub mod chart;
mod dates;
pub mod export;
mod formats;
pub mod presentation;
pub mod render;
mod requests;
#[cfg(test)]
mod tests;

use std::path::Path;

use serde_json::Value;

use crate::cli::{ReportArgs, ReportCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi, RuntimeSession};
use crate::error::AppError;

use self::chart::ChartHandler;
use self::export::ExportHandler;
use self::render::RenderHandler;

pub struct ReportHandler;

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
pub(crate) struct RenderedReport {
    pub(crate) content: String,
    pub(crate) report_window_metadata: Option<ReportWindowMetadata>,
}

pub(crate) trait ReportSession {
    fn cli_config(&self) -> &CliConfig;
    fn runtime_output_root(&self) -> &Path;
    fn render(&self, request: &Value) -> Result<RenderedReport, AppError>;
    fn list_targets(&self, display_mode: &str) -> Result<Vec<String>, AppError>;
    fn export(&self, request: &Value) -> Result<(), AppError>;
}

pub(crate) trait ReportSessionPort {
    fn open(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<Box<dyn ReportSession>, AppError>;
}

pub(crate) struct RuntimeReportSessionPort;

struct RuntimeBoundReportSession {
    session: RuntimeSession,
}

impl ReportSession for RuntimeBoundReportSession {
    fn cli_config(&self) -> &CliConfig {
        self.session.cli_config()
    }

    fn runtime_output_root(&self) -> &Path {
        Path::new(&self.session.paths().runtime_output_root)
    }

    fn render(&self, request: &Value) -> Result<RenderedReport, AppError> {
        let rendered = self.session.report().render(request)?;
        Ok(RenderedReport {
            content: rendered.content,
            report_window_metadata: rendered.report_window_metadata.map(|metadata| {
                ReportWindowMetadata {
                    has_records: metadata.has_records,
                    matched_day_count: metadata.matched_day_count,
                    matched_record_count: metadata.matched_record_count,
                    start_date: metadata.start_date,
                    end_date: metadata.end_date,
                    requested_days: metadata.requested_days,
                }
            }),
        })
    }

    fn list_targets(&self, display_mode: &str) -> Result<Vec<String>, AppError> {
        self.session.report().list_targets(display_mode)
    }

    fn export(&self, request: &Value) -> Result<(), AppError> {
        self.session.report().export(request)
    }
}

impl ReportSessionPort for RuntimeReportSessionPort {
    fn open(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<Box<dyn ReportSession>, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        Ok(Box::new(RuntimeBoundReportSession { session }))
    }
}

impl CommandHandler<ReportArgs> for ReportHandler {
    fn handle(&self, args: ReportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            ReportCommand::Render(args) => RenderHandler.handle(args, ctx),
            ReportCommand::Export(args) => ExportHandler.handle(args, ctx),
            ReportCommand::Chart(args) => ChartHandler.handle(args, ctx),
        }
    }
}
