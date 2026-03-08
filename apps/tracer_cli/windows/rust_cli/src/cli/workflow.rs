use clap::{ArgAction, Args, ValueEnum};

#[derive(Debug, Args)]
pub struct ConvertArgs {
    pub path: String,
}

#[derive(Debug, Args)]
pub struct ImportArgs {
    pub path: String,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum DateCheckMode {
    None,
    Continuity,
    Full,
}

#[derive(Debug, Args)]
pub struct IngestArgs {
    pub path: String,
    #[arg(long = "date-check", value_enum, conflicts_with = "no_date_check")]
    pub date_check: Option<DateCheckMode>,
    #[arg(long = "no-date-check", action = ArgAction::SetTrue)]
    pub no_date_check: bool,
    #[arg(long = "save-processed", action = ArgAction::SetTrue, conflicts_with = "no_save")]
    pub save: bool,
    #[arg(long = "no-save", action = ArgAction::SetTrue)]
    pub no_save: bool,
}

#[derive(Debug, Args)]
pub struct ValidateLogicArgs {
    pub path: String,
    #[arg(long = "date-check", value_enum, conflicts_with = "no_date_check")]
    pub date_check: Option<DateCheckMode>,
    #[arg(long = "no-date-check", action = ArgAction::SetTrue)]
    pub no_date_check: bool,
}

#[derive(Debug, Args)]
pub struct ValidateStructureArgs {
    pub path: String,
}
