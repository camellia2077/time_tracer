use clap::{ArgAction, Args, Subcommand, ValueEnum};

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum DateCheckMode {
    None,
    Continuity,
    Full,
}

#[derive(Debug, Args)]
pub struct PipelineConvertArgs {
    pub path: String,
}

#[derive(Debug, Args)]
pub struct PipelineImportArgs {
    pub path: String,
}

#[derive(Debug, Args)]
pub struct PipelineIngestArgs {
    pub path: String,
    #[arg(long = "date-check", value_enum, conflicts_with = "no_date_check")]
    pub date_check: Option<DateCheckMode>,
    #[arg(long = "no-date-check", action = ArgAction::SetTrue)]
    pub no_date_check: bool,
    #[arg(
        long = "save-processed",
        action = ArgAction::SetTrue,
        conflicts_with = "no_save_processed"
    )]
    pub save_processed: bool,
    #[arg(long = "no-save-processed", action = ArgAction::SetTrue)]
    pub no_save_processed: bool,
}

#[derive(Debug, Args)]
pub struct PipelineValidateStructureArgs {
    pub path: String,
}

#[derive(Debug, Args)]
pub struct PipelineValidateLogicArgs {
    pub path: String,
    #[arg(long = "date-check", value_enum, conflicts_with = "no_date_check")]
    pub date_check: Option<DateCheckMode>,
    #[arg(long = "no-date-check", action = ArgAction::SetTrue)]
    pub no_date_check: bool,
}

#[derive(Debug, Args)]
pub struct PipelineValidateAllArgs {
    pub path: String,
    #[arg(long = "date-check", value_enum, conflicts_with = "no_date_check")]
    pub date_check: Option<DateCheckMode>,
    #[arg(long = "no-date-check", action = ArgAction::SetTrue)]
    pub no_date_check: bool,
}

#[derive(Debug, Subcommand)]
pub enum PipelineValidateCommand {
    #[command(about = "Validate source TXT syntax and structure")]
    Structure(PipelineValidateStructureArgs),
    #[command(about = "Validate business logic rules")]
    Logic(PipelineValidateLogicArgs),
    #[command(about = "Run structure and logic validation in order")]
    All(PipelineValidateAllArgs),
}

#[derive(Debug, Args)]
pub struct PipelineValidateArgs {
    #[command(subcommand)]
    pub command: PipelineValidateCommand,
}

#[derive(Debug, Subcommand)]
pub enum PipelineCommand {
    #[command(about = "Convert source files to processed JSON")]
    Convert(PipelineConvertArgs),
    #[command(about = "Import processed JSON data into the database")]
    Import(PipelineImportArgs),
    #[command(about = "Run full ingestion pipeline")]
    Ingest(PipelineIngestArgs),
    #[command(about = "Validate source structure and business logic")]
    Validate(PipelineValidateArgs),
}

#[derive(Debug, Args)]
pub struct PipelineArgs {
    #[command(subcommand)]
    pub command: PipelineCommand,
}
