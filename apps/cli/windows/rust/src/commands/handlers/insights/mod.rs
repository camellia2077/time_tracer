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

use crate::cli::{InsightsArgs, InsightsCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi, RuntimeSession};
use crate::error::AppError;

use self::chart::ChartHandler;
use self::export::ExportHandler;
use self::render::RenderHandler;

pub struct InsightsHandler;

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct InsightsWindowMetadata {
    pub(crate) has_records: bool,
    pub(crate) matched_day_count: i32,
    pub(crate) matched_record_count: i32,
    pub(crate) start_date: String,
    pub(crate) end_date: String,
    pub(crate) requested_days: i32,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct RenderedInsights {
    pub(crate) content: String,
    pub(crate) insights_window_metadata: Option<InsightsWindowMetadata>,
}

pub(crate) trait InsightsSession {
    fn cli_config(&self) -> &CliConfig;
    fn runtime_output_root(&self) -> &Path;
    fn render(&self, request: &Value) -> Result<RenderedInsights, AppError>;
    fn list_targets(&self, display_mode: &str) -> Result<Vec<String>, AppError>;
    fn export(&self, request: &Value) -> Result<(), AppError>;
}

pub(crate) trait InsightsSessionPort {
    fn open(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<Box<dyn InsightsSession>, AppError>;
}

pub(crate) struct RuntimeInsightsSessionPort;

struct RuntimeBoundInsightsSession {
    session: RuntimeSession,
}

impl InsightsSession for RuntimeBoundInsightsSession {
    fn cli_config(&self) -> &CliConfig {
        self.session.cli_config()
    }

    fn runtime_output_root(&self) -> &Path {
        Path::new(&self.session.paths().runtime_output_root)
    }

    fn render(&self, request: &Value) -> Result<RenderedInsights, AppError> {
        let rendered = self.session.insights().render(request)?;
        Ok(RenderedInsights {
            content: rendered.content,
            insights_window_metadata: rendered.insights_window_metadata.map(|metadata| {
                InsightsWindowMetadata {
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
        self.session.insights().list_targets(display_mode)
    }

    fn export(&self, request: &Value) -> Result<(), AppError> {
        self.session.insights().export(request)
    }
}

impl InsightsSessionPort for RuntimeInsightsSessionPort {
    fn open(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<Box<dyn InsightsSession>, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        Ok(Box::new(RuntimeBoundInsightsSession { session }))
    }
}

impl CommandHandler<InsightsArgs> for InsightsHandler {
    fn handle(&self, args: InsightsArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            InsightsCommand::Render(args) => RenderHandler.handle(args, ctx),
            InsightsCommand::Export(args) => ExportHandler.handle(args, ctx),
            InsightsCommand::Chart(args) => ChartHandler.handle(args, ctx),
        }
    }
}
