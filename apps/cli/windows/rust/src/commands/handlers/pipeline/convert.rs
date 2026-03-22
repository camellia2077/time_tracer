use crate::cli::PipelineConvertArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::build_convert_request;
use super::{PipelineSessionPort, RuntimePipelineSessionPort};

pub struct ConvertHandler;

impl CommandHandler<PipelineConvertArgs> for ConvertHandler {
    fn handle(&self, args: PipelineConvertArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_convert_with_port(args, ctx, &RuntimePipelineSessionPort)
    }
}

pub(crate) fn run_convert_with_port(
    args: PipelineConvertArgs,
    ctx: &CommandContext,
    port: &dyn PipelineSessionPort,
) -> Result<(), AppError> {
    let cli_config = port.load_cli_config("convert", ctx)?;
    let request = build_convert_request(&args, &cli_config);
    port.convert("convert", ctx, &request)
}
