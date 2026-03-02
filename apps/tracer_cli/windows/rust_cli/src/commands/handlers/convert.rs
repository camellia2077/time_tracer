use serde_json::json;

use crate::cli::ConvertArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct ConvertHandler;

impl CommandHandler<ConvertArgs> for ConvertHandler {
    fn handle(&self, args: ConvertArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let (runtime, cli_config) = api.bootstrap("convert", ctx)?;

        let date_check_mode = cli_config
            .command_defaults
            .convert_date_check_mode
            .or(cli_config.default_date_check_mode)
            .unwrap_or_else(|| "none".to_string());
        let save_processed_output = cli_config
            .command_defaults
            .convert_save_processed_output
            .unwrap_or(cli_config.default_save_processed_output);
        let validate_logic = cli_config
            .command_defaults
            .convert_validate_logic
            .unwrap_or(true);
        let validate_structure = cli_config
            .command_defaults
            .convert_validate_structure
            .unwrap_or(true);

        let request = json!({
            "input_path": args.path,
            "date_check_mode": date_check_mode,
            "save_processed_output": save_processed_output,
            "validate_logic": validate_logic,
            "validate_structure": validate_structure,
        });

        runtime.run_convert(&request)
    }
}
