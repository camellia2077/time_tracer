use crate::cli::ReportRenderArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::formats::resolve_render_formats;
use super::presentation::build_empty_window_hint;
use super::requests::build_render_request;
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
    let session = port.open("query", ctx)?;
    let formats = resolve_render_formats(&args, session.cli_config());

    for (index, format) in formats.iter().enumerate() {
        if index > 0 {
            println!("\n{}", "=".repeat(40));
        }
        let request =
            build_render_request(args.period, &args.argument, args.as_of.as_deref(), format)?;
        let rendered = session.render(&request)?;
        print!("{}", rendered.content);
        if let Some(hint) = build_empty_window_hint(&rendered) {
            print!("{hint}");
        }
    }
    Ok(())
}
