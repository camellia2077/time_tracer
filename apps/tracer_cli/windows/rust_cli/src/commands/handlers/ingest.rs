use serde_json::json;

use crate::cli::{DateCheckMode, IngestArgs};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct IngestHandler;

impl CommandHandler<IngestArgs> for IngestHandler {
    fn handle(&self, args: IngestArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let (runtime, cli_config) = api.bootstrap("ingest", ctx)?;

        let mut date_check_mode = cli_config
            .command_defaults
            .ingest_date_check_mode
            .or(cli_config.default_date_check_mode)
            .unwrap_or_else(|| "none".to_string());
        if let Some(mode) = args.date_check {
            date_check_mode = date_check_mode_token(mode).to_string();
        } else if args.no_date_check {
            date_check_mode = "none".to_string();
        }

        let mut save_processed_output = cli_config
            .command_defaults
            .ingest_save_processed_output
            .unwrap_or(cli_config.default_save_processed_output);
        if args.no_save {
            save_processed_output = false;
        } else if args.save {
            save_processed_output = true;
        }

        let request = json!({
            "input_path": args.path,
            "date_check_mode": date_check_mode,
            "save_processed_output": save_processed_output,
        });
        runtime.run_ingest(&request)
    }
}

fn date_check_mode_token(value: DateCheckMode) -> &'static str {
    match value {
        DateCheckMode::None => "none",
        DateCheckMode::Continuity => "continuity",
        DateCheckMode::Full => "full",
    }
}
