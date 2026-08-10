use clap::{ArgAction, Args, Subcommand, ValueEnum};

use super::ChartArgs;

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum InsightsFormat {
    Md,
    Tex,
    Typ,
}

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum InsightsRenderPeriod {
    Day,
    Month,
    Week,
    Year,
    Recent,
    Range,
}

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum InsightsExportPeriod {
    Day,
    Month,
    Week,
    Year,
    Recent,
    Range,
}

#[derive(Debug, Args)]
pub struct InsightsRenderArgs {
    #[arg(value_enum)]
    pub period: InsightsRenderPeriod,
    #[arg(
        value_name = "TARGET",
        help = "Render target. day accepts YYYYMMDD or YYYY-MM-DD; month accepts YYYYMM or YYYY-MM; range accepts <from>|<to> with each side in YYYYMMDD or YYYY-MM-DD. Compact day/month/range endpoints are normalized to ISO before querying."
    )]
    pub argument: String,
    // Design intent:
    // `--as-of` makes `recent` deterministic for reproducible runs (CI/golden/manual audits).
    // The CLI resolves it into a fixed logical window [as_of-(days-1), as_of].
    #[arg(
        long = "as-of",
        value_name = "YYYY-MM-DD",
        help = "Anchor date used only for `recent`. Example: `insights render recent 7 --as-of 2026-03-07` resolves to range 2026-03-01|2026-03-07."
    )]
    pub as_of: Option<String>,
    #[arg(short = 'f', long = "format", value_enum, value_delimiter = ',')]
    pub format: Vec<InsightsFormat>,
}

#[derive(Debug, Args)]
pub struct InsightsExportArgs {
    #[arg(value_enum)]
    pub period: InsightsExportPeriod,
    #[arg(
        value_name = "TARGET",
        help = "Export target. day accepts YYYYMMDD or YYYY-MM-DD; month accepts YYYYMM or YYYY-MM. Compact day/month inputs are normalized to ISO before querying and file naming. recent expects a positive integer or comma-separated list with --all."
    )]
    pub argument: Option<String>,
    #[arg(long = "all", action = ArgAction::SetTrue)]
    pub all: bool,
    // Same intent as render: provide a stable logical "today" for `recent`
    // without changing core runtime/C ABI request contracts.
    #[arg(
        long = "as-of",
        value_name = "YYYY-MM-DD",
        help = "Anchor date used only for `recent`. Example: `insights export recent 7 --as-of 2026-03-07` exports the fixed window 2026-03-01|2026-03-07."
    )]
    pub as_of: Option<String>,
    #[arg(short = 'f', long = "format", value_enum, value_delimiter = ',')]
    pub format: Vec<InsightsFormat>,
}

#[derive(Debug, Subcommand)]
pub enum InsightsCommand {
    #[command(
        about = "Render textual insights from runtime insights queries",
        long_about = "Render textual insights from runtime insights queries.\n\nDate target rules:\n- day: YYYYMMDD or YYYY-MM-DD, normalized to ISO YYYY-MM-DD before querying\n- month: YYYYMM or YYYY-MM, normalized to ISO YYYY-MM before querying\n- range: <from>|<to>, where each side accepts YYYYMMDD or YYYY-MM-DD and is normalized to ISO before querying\n- recent accepts positive integer days and optional --as-of YYYY-MM-DD to resolve a fixed range window\n- week/year keep their canonical argument forms"
    )]
    Render(InsightsRenderArgs),
    #[command(
        about = "Export insights to md/tex/typ formats",
        long_about = "Export insights to md/tex/typ formats.\n\nDate target rules:\n- day: YYYYMMDD or YYYY-MM-DD, normalized to ISO YYYY-MM-DD before querying and output naming\n- month: YYYYMM or YYYY-MM, normalized to ISO YYYY-MM before querying and output naming\n- range: <from>|<to>, where each side accepts YYYYMMDD or YYYY-MM-DD and is normalized to ISO before querying and output naming\n- recent accepts positive integer days and optional --as-of YYYY-MM-DD to resolve a fixed range window while keeping recent export naming\n- week/year keep their canonical argument forms"
    )]
    Export(InsightsExportArgs),
    #[command(about = "Generate insights-chart HTML from database data")]
    Chart(ChartArgs),
}

#[derive(Debug, Args)]
pub struct InsightsArgs {
    #[command(subcommand)]
    pub command: InsightsCommand,
}
