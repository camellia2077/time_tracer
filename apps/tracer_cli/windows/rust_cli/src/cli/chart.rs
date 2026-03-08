use clap::{ArgAction, Args, ValueEnum};

#[derive(Debug, Clone, ValueEnum)]
pub enum ChartType {
    Line,
    Bar,
    Pie,
    #[value(name = "heatmap-year")]
    HeatmapYear,
    #[value(name = "heatmap-month")]
    HeatmapMonth,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum ChartTheme {
    Default,
    Github,
}

#[derive(Debug, Args)]
pub struct ChartArgs {
    #[arg(
        long = "type",
        value_enum,
        default_value_t = ChartType::Line,
        help = "Chart type"
    )]
    pub chart_type: ChartType,
    #[arg(
        long = "theme",
        value_enum,
        default_value_t = ChartTheme::Default,
        help = "Chart theme"
    )]
    pub theme: ChartTheme,
    #[arg(long = "heatmap-palette")]
    pub heatmap_palette: Option<String>,
    #[arg(
        long = "list-heatmap-palettes",
        action = ArgAction::SetTrue,
        conflicts_with_all = ["heatmap_palette", "root", "year", "month", "from", "to", "lookback_days"]
    )]
    pub list_heatmap_palettes: bool,
    #[arg(long)]
    pub root: Option<String>,
    #[arg(long)]
    pub year: Option<i32>,
    #[arg(long)]
    pub month: Option<i32>,
    #[arg(long)]
    pub from: Option<String>,
    #[arg(long)]
    pub to: Option<String>,
    #[arg(long = "lookback-days")]
    pub lookback_days: Option<i32>,
}
