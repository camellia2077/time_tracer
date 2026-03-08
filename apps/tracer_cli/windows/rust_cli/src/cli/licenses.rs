use clap::{ArgAction, Args};

#[derive(Debug, Args)]
pub struct LicensesArgs {
    #[arg(
        long = "full",
        action = ArgAction::SetTrue,
        help = "Emit detailed third-party license fields"
    )]
    pub full: bool,
}
