use serde_json::json;

use crate::cli::{DateCheckMode, ValidateLogicArgs};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct ValidateLogicHandler;

impl CommandHandler<ValidateLogicArgs> for ValidateLogicHandler {
    fn handle(&self, args: ValidateLogicArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let (runtime, cli_config) = api.bootstrap("validate-logic", ctx)?;

        let mut date_check_mode = cli_config
            .command_defaults
            .validate_logic_date_check_mode
            .or(cli_config.default_date_check_mode)
            .unwrap_or_else(|| "none".to_string());
        if let Some(mode) = args.date_check {
            date_check_mode = date_check_mode_token(mode).to_string();
        } else if args.no_date_check {
            date_check_mode = "none".to_string();
        }

        let request = json!({
            "input_path": args.path,
            "date_check_mode": date_check_mode,
        });
        runtime.run_validate_logic(&request)
    }
}

fn date_check_mode_token(value: DateCheckMode) -> &'static str {
    match value {
        DateCheckMode::None => "none",
        DateCheckMode::Continuity => "continuity",
        DateCheckMode::Full => "full",
    }
}
