use serde_json::json;

use crate::cli::ValidateStructureArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct ValidateStructureHandler;

impl CommandHandler<ValidateStructureArgs> for ValidateStructureHandler {
    fn handle(&self, args: ValidateStructureArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let (runtime, _cli_config) = api.bootstrap("validate-structure", ctx)?;
        let request = json!({ "input_path": args.path });
        runtime.run_validate_structure(&request)
    }
}
