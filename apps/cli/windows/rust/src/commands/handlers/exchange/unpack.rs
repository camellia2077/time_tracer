use serde_json::json;

use crate::cli::ExchangeUnpackArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

use super::support::{
    absolute_existing_path, print_content, require_output_path_for, validate_exchange_package_input,
};
use super::{
    ExchangePromptPort, ExchangeSessionPort, InteractiveExchangePromptPort,
    RuntimeExchangeSessionPort,
};

pub struct UnpackHandler;

impl CommandHandler<ExchangeUnpackArgs> for UnpackHandler {
    fn handle(&self, args: ExchangeUnpackArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_unpack_with_port(
            args,
            ctx,
            &RuntimeExchangeSessionPort,
            &InteractiveExchangePromptPort,
        )
    }
}

pub(crate) fn run_unpack_with_port(
    args: ExchangeUnpackArgs,
    ctx: &CommandContext,
    port: &dyn ExchangeSessionPort,
    prompts: &dyn ExchangePromptPort,
) -> Result<(), AppError> {
    let input = absolute_existing_path(&args.input)?;
    validate_exchange_package_input(&input)?;

    let output = require_output_path_for(ctx.output_path.as_deref(), "exchange unpack")?;
    let passphrase = prompts.prompt_package_passphrase()?;

    let request = json!({
        "input_path": input.to_string_lossy().to_string(),
        "output_path": output.to_string_lossy().to_string(),
        "passphrase": passphrase,
    });
    let content = port.unpack_package("crypto", ctx, &request)?;
    print_content(&content);
    Ok(())
}
