use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

pub struct MottoHandler;

impl CommandHandler<()> for MottoHandler {
    fn handle(&self, _args: (), _ctx: &CommandContext) -> Result<(), AppError> {
        println!("\"Trace your time, log your life.\"");
        Ok(())
    }
}
