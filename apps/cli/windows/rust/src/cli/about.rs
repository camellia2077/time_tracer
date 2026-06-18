use clap::{Args, Subcommand};

use super::LicensesArgs;

#[derive(Debug, Subcommand)]
pub enum AboutCommand {
    #[command(about = "Print third-party dependency licenses")]
    Licenses(LicensesArgs),
    #[command(about = "Print the tracer easter egg line")]
    Tracer,
    #[command(about = "Print the project motto easter egg")]
    Motto,
}

#[derive(Debug, Args)]
pub struct AboutArgs {
    #[command(subcommand)]
    pub command: AboutCommand,
}
