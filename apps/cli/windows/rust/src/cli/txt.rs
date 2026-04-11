use clap::{Args, Subcommand};

#[derive(Debug, Args)]
pub struct TxtViewDayArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
    #[arg(long = "day", value_name = "MMDD")]
    pub day: String,
}

#[derive(Debug, Subcommand)]
pub enum TxtCommand {
    #[command(about = "Display a single day block from a monthly TXT file")]
    ViewDay(TxtViewDayArgs),
}

#[derive(Debug, Args)]
pub struct TxtArgs {
    #[command(subcommand)]
    pub command: TxtCommand,
}
