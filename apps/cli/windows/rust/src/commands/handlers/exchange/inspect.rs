use serde_json::json;

use crate::cli::ExchangeInspectArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{
    absolute_existing_path, print_content, reject_output_path, validate_exchange_package_input,
};
use super::{
    ExchangePromptPort, ExchangeSessionPort, InteractiveExchangePromptPort,
    RuntimeExchangeSessionPort,
};

pub struct InspectHandler;

impl CommandHandler<ExchangeInspectArgs> for InspectHandler {
    fn handle(&self, args: ExchangeInspectArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_inspect_with_port(
            args,
            ctx,
            &RuntimeExchangeSessionPort,
            &InteractiveExchangePromptPort,
        )
    }
}

pub(crate) fn run_inspect_with_port(
    args: ExchangeInspectArgs,
    ctx: &CommandContext,
    port: &dyn ExchangeSessionPort,
    prompts: &dyn ExchangePromptPort,
) -> Result<(), AppError> {
    reject_output_path(ctx.output_path.as_deref(), "exchange inspect")?;
    let input = absolute_existing_path(&args.input)?;
    validate_exchange_package_input(&input)?;

    let passphrase = prompts.prompt_package_passphrase()?;
    let request = json!({
        "input_path": input.to_string_lossy().to_string(),
        "passphrase": passphrase,
    });
    let content = port.inspect_package("crypto", ctx, &request)?;
    print_content(&content);
    Ok(())
}
