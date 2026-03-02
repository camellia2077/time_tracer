use crate::cli::LicensesArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::error::AppError;
use crate::licenses;

pub struct LicensesHandler;

impl CommandHandler<LicensesArgs> for LicensesHandler {
    fn handle(&self, args: LicensesArgs, _ctx: &CommandContext) -> Result<(), AppError> {
        if args.full {
            println!("{}", licenses::render_third_party_full());
            return Ok(());
        }

        println!("{}", licenses::render_third_party_summary());
        Ok(())
    }
}
