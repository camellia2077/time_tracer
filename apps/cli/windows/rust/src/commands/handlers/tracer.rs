use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;

pub struct TracerHandler;

impl CommandHandler<()> for TracerHandler {
    fn handle(&self, _args: (), _ctx: &CommandContext) -> Result<(), AppError> {
        println!("\"Cheers, love! The timetracer is here.\"");
        Ok(())
    }
}
