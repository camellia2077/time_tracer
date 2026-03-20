use crate::cli::{ValidateArgs, ValidateTarget};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::commands::handlers::validate_logic::ValidateLogicHandler;
use crate::commands::handlers::validate_structure::ValidateStructureHandler;
use crate::error::AppError;

pub struct ValidateHandler;

impl CommandHandler<ValidateArgs> for ValidateHandler {
    fn handle(&self, args: ValidateArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.target {
            Some(ValidateTarget::Structure(args)) => ValidateStructureHandler.handle(args, ctx),
            Some(ValidateTarget::Logic(args)) => ValidateLogicHandler.handle(args, ctx),
            None => match args.try_into_default_logic_args() {
                Some(args) => ValidateLogicHandler.handle(args, ctx),
                None => Err(AppError::InvalidArguments(
                    "Missing required argument: <path>".to_string(),
                )),
            },
        }
    }
}
