use crate::cli::ReportExportArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{build_export_request, resolve_export_formats};
use super::{ReportSessionPort, RuntimeReportSessionPort};

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
    let cli_config = port.load_cli_config("export", ctx)?;
    let formats = resolve_export_formats(&args, &cli_config);

    for format in &formats {
        let request =
            build_export_request(args.period, args.argument.as_deref(), args.all, format)?;
        port.export("export", ctx, &request)?;
    }
    Ok(())
}
