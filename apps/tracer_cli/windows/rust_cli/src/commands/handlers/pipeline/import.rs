use crate::cli::PipelineImportArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::build_import_request;
use super::{PipelineSessionPort, RuntimePipelineSessionPort};

pub struct ImportHandler;

impl CommandHandler<PipelineImportArgs> for ImportHandler {
    fn handle(&self, args: PipelineImportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_import_with_port(args, ctx, &RuntimePipelineSessionPort)
    }
}

pub(crate) fn run_import_with_port(
    args: PipelineImportArgs,
    ctx: &CommandContext,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let request = build_import_request(&args);
    port.import_processed("import", ctx, &request)
}
