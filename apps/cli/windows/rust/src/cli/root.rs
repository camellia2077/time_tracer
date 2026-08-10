use clap::{ArgAction, Parser, Subcommand};

use super::{
    AboutArgs, ActivityArgs, AliasArgs, ExchangeArgs, PipelineArgs, QueryArgs, InsightsArgs,
    SystemArgs, TxtArgs,
};

#[derive(Debug, Parser)]
#[command(
    name = "time_tracer_cli",
    version,
    about = "Rust CLI shell for time tracer",
    long_about = None,
    arg_required_else_help = true,
    disable_version_flag = true,
    propagate_version = true
)]
pub struct Cli {
    #[arg(
        short = 'v',
        long = "version",
        action = ArgAction::Version,
        global = true,
        help = "Print version information and exit"
    )]
    pub version: (),
    #[arg(
        long = "db",
        value_name = "PATH",
        global = true,
        help = "Database path override"
    )]
    pub db: Option<String>,
    #[arg(
        short = 'o',
        long = "output",
        value_name = "PATH",
        global = true,
        help = "Output path override. Required for `exchange export`."
    )]
    pub output: Option<String>,
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Debug, Subcommand)]
pub enum Command {
    #[command(about = "Merge canonical leaf activities and migrate TXT/database")]
    Activity(ActivityArgs),
    #[command(about = "Edit activity hierarchy TOML and migrate canonical activity paths")]
    Alias(AliasArgs),
    #[command(about = "Run semantic data and tree queries")]
    Query(QueryArgs),
    #[command(about = "Run pipeline operations against source and processed data")]
    Pipeline(PipelineArgs),
    #[command(about = "Render, export, and chart insights")]
    Insights(InsightsArgs),
    #[command(about = "Export/import/inspect tracer exchange packages")]
    Exchange(ExchangeArgs),
    #[command(about = "Inspect monthly TXT files through shared day-block semantics")]
    Txt(TxtArgs),
    #[command(about = "Run runtime/system inspection commands")]
    System(SystemArgs),
    #[command(about = "Print project/about information and easter eggs")]
    About(AboutArgs),
}
