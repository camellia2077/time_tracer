use clap::{Args, Subcommand};

use super::DoctorArgs;

#[derive(Debug, Subcommand)]
pub enum SystemCommand {
    #[command(about = "Run runtime dependency/config diagnostics")]
    Doctor(DoctorArgs),
}

#[derive(Debug, Args)]
pub struct SystemArgs {
    #[command(subcommand)]
    pub command: SystemCommand,
}
