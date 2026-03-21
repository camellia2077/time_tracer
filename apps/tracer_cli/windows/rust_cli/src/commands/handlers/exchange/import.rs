use serde_json::json;

use crate::cli::ExchangeImportArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{
    absolute_existing_path, print_content, resolve_output_path, validate_exchange_package_input,
};
use super::{
    ExchangePromptPort, ExchangeSessionPort, InteractiveExchangePromptPort,
    RuntimeExchangeSessionPort,
};

pub struct ImportHandler;

impl CommandHandler<ExchangeImportArgs> for ImportHandler {
    fn handle(&self, args: ExchangeImportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_import_with_port(
            args,
            ctx,
            &RuntimeExchangeSessionPort,
            &InteractiveExchangePromptPort,
        )
    }
}

pub(crate) fn run_import_with_port(
    args: ExchangeImportArgs,
    ctx: &CommandContext,
    port: &dyn ExchangeSessionPort,
    prompts: &dyn ExchangePromptPort,
) -> Result<(), AppError> {
    let input = absolute_existing_path(&args.input)?;
    validate_exchange_package_input(&input)?;

    let passphrase = prompts.prompt_package_passphrase()?;
    let output = resolve_output_path(ctx.output_path.as_deref())?;

    let mut request = json!({
        "input_path": input.to_string_lossy().to_string(),
        "passphrase": passphrase,
    });
    if let Some(output) = output {
        request["output_path"] = json!(output.to_string_lossy().to_string());
    }

    let content = port.import_package("crypto", ctx, &request)?;
    print_content(&content);
    Ok(())
}
