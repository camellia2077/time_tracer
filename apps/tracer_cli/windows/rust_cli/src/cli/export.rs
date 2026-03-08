use clap::{Args, ValueEnum};

#[derive(Debug, Clone, ValueEnum)]
pub enum ExportType {
    Day,
    Month,
    Week,
    Year,
    Recent,
    #[value(name = "all-day")]
    AllDay,
    #[value(name = "all-month")]
    AllMonth,
    #[value(name = "all-week")]
    AllWeek,
    #[value(name = "all-year")]
    AllYear,
    #[value(name = "all-recent")]
    AllRecent,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum ExportFormat {
    Md,
    Tex,
    Typ,
}

#[derive(Debug, Args)]
pub struct ExportArgs {
    #[arg(value_enum)]
    pub export_type: ExportType,
    #[arg(
        required_if_eq_any = [
            ("export_type", "day"),
            ("export_type", "month"),
            ("export_type", "week"),
            ("export_type", "year"),
            ("export_type", "recent"),
            ("export_type", "all-recent")
        ]
    )]
    pub argument: Option<String>,
    #[arg(short = 'f', long = "format", value_enum, value_delimiter = ',')]
    pub format: Vec<ExportFormat>,
}
