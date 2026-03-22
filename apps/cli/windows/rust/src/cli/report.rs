use clap::{ArgAction, Args, Subcommand, ValueEnum};

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum ReportFormat {
    Md,
    Tex,
    Typ,
}

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum ReportRenderPeriod {
    Day,
    Month,
    Week,
    Year,
    Recent,
    Range,
}

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum ReportExportPeriod {
    Day,
    Month,
    Week,
    Year,
    Recent,
}

#[derive(Debug, Args)]
pub struct ReportRenderArgs {
    #[arg(value_enum)]
    pub period: ReportRenderPeriod,
    pub argument: String,
    #[arg(short = 'f', long = "format", value_enum, value_delimiter = ',')]
    pub format: Vec<ReportFormat>,
}

#[derive(Debug, Args)]
pub struct ReportExportArgs {
    #[arg(value_enum)]
    pub period: ReportExportPeriod,
    pub argument: Option<String>,
    #[arg(long = "all", action = ArgAction::SetTrue)]
    pub all: bool,
    #[arg(short = 'f', long = "format", value_enum, value_delimiter = ',')]
    pub format: Vec<ReportFormat>,
}

#[derive(Debug, Subcommand)]
pub enum ReportCommand {
    #[command(about = "Render textual reports from runtime report queries")]
    Render(ReportRenderArgs),
    #[command(about = "Export reports to md/tex/typ formats")]
    Export(ReportExportArgs),
}

#[derive(Debug, Args)]
pub struct ReportArgs {
    #[command(subcommand)]
    pub command: ReportCommand,
}
