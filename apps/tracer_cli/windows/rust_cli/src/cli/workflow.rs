use clap::{ArgAction, Args, Subcommand, ValueEnum};

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

#[derive(Debug, Subcommand)]
pub enum ValidateTarget {
    #[command(about = "Validate source TXT syntax and structure")]
    Structure(ValidateStructureArgs),
    #[command(about = "Validate business logic rules")]
    Logic(ValidateLogicArgs),
}

#[derive(Debug, Args)]
#[command(
    arg_required_else_help = true,
    args_conflicts_with_subcommands = true,
    subcommand_precedence_over_arg = true
)]
pub struct ValidateArgs {
    #[command(subcommand)]
    pub target: Option<ValidateTarget>,
    pub path: Option<String>,
    #[arg(long = "date-check", value_enum, conflicts_with = "no_date_check")]
    pub date_check: Option<DateCheckMode>,
    #[arg(long = "no-date-check", action = ArgAction::SetTrue)]
    pub no_date_check: bool,
}

impl ValidateArgs {
    pub fn try_into_default_logic_args(self) -> Option<ValidateLogicArgs> {
        self.path.map(|path| ValidateLogicArgs {
            path,
            date_check: self.date_check,
            no_date_check: self.no_date_check,
        })
    }
}
