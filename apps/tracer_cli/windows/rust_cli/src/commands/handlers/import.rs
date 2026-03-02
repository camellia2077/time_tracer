use serde_json::json;

use crate::cli::ImportArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct ImportHandler;

impl CommandHandler<ImportArgs> for ImportHandler {
    fn handle(&self, args: ImportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let (runtime, _cli_config) = api.bootstrap("import", ctx)?;
        let request = json!({ "processed_path": args.path });
        runtime.run_import(&request)
    }
}
