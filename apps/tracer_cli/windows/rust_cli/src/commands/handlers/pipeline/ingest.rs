use crate::cli::PipelineIngestArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::build_ingest_request;
use super::{PipelineSessionPort, RuntimePipelineSessionPort};

pub struct IngestHandler;

impl CommandHandler<PipelineIngestArgs> for IngestHandler {
    fn handle(&self, args: PipelineIngestArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_ingest_with_port(args, ctx, &RuntimePipelineSessionPort)
    }
}

pub(crate) fn run_ingest_with_port(
    args: PipelineIngestArgs,
    ctx: &CommandContext,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let cli_config = port.load_cli_config("ingest", ctx)?;
    let request = build_ingest_request(&args, &cli_config);
    port.ingest("ingest", ctx, &request)
}
