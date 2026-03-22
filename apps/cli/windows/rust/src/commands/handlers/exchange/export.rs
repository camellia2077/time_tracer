use serde_json::json;

use crate::cli::ExchangeExportArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{
    absolute_existing_path, print_content, require_output_path, security_level_token,
    validate_exchange_export_input,
};
use super::{
    ExchangePromptPort, ExchangeSessionPort, InteractiveExchangePromptPort,
    RuntimeExchangeSessionPort,
};

pub struct ExportHandler;

impl CommandHandler<ExchangeExportArgs> for ExportHandler {
    fn handle(&self, args: ExchangeExportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_export_with_port(
            args,
            ctx,
            &RuntimeExchangeSessionPort,
            &InteractiveExchangePromptPort,
        )
    }
}

pub(crate) fn run_export_with_port(
    args: ExchangeExportArgs,
    ctx: &CommandContext,
    port: &dyn ExchangeSessionPort,
    prompts: &dyn ExchangePromptPort,
) -> Result<(), AppError> {
    let input = absolute_existing_path(&args.input)?;
    validate_exchange_export_input(&input)?;

    let output = require_output_path(ctx.output_path.as_deref())?;
    let passphrase = prompts.prompt_export_passphrase()?;
    let cli_config = port.load_cli_config("crypto", ctx)?;
    let date_check_mode = cli_config
        .default_date_check_mode
        .clone()
        .unwrap_or_else(|| "none".to_string());

    let request = json!({
        "input_path": input.to_string_lossy().to_string(),
        "output_path": output.to_string_lossy().to_string(),
        "passphrase": passphrase,
        "security_level": security_level_token(args.security_level),
        "date_check_mode": date_check_mode,
    });
    let content = port.export_package("crypto", ctx, &request)?;
    print_content(&content);
    Ok(())
}
