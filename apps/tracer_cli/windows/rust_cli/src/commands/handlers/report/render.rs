use crate::cli::ReportRenderArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{build_render_request, resolve_render_formats};
use super::{ReportSessionPort, RuntimeReportSessionPort};

pub struct RenderHandler;

impl CommandHandler<ReportRenderArgs> for RenderHandler {
    fn handle(&self, args: ReportRenderArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_render_with_port(args, ctx, &RuntimeReportSessionPort)
    }
}

pub(crate) fn run_render_with_port(
    args: ReportRenderArgs,
    ctx: &CommandContext,
    port: &dyn ReportSessionPort,
) -> Result<(), AppError> {
    let cli_config = port.load_cli_config("query", ctx)?;
    let formats = resolve_render_formats(&args, &cli_config);

    for (index, format) in formats.iter().enumerate() {
        if index > 0 {
            println!("\n{}", "=".repeat(40));
        }
        let request = build_render_request(args.period, &args.argument, format)?;
        let content = port.render("query", ctx, &request)?;
        print!("{content}");
    }
    Ok(())
}
