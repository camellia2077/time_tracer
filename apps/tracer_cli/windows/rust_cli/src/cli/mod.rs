use clap::{ArgAction, Args, Parser, Subcommand, ValueEnum};

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
        visible_alias = "database",
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
        help = "Output path override"
    )]
    pub output: Option<String>,
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Debug, Subcommand)]
pub enum Command {
    #[command(about = "Query statistics from the database")]
    Query(QueryArgs),
    #[command(about = "Generate report-chart HTML from database data")]
    Chart(ChartArgs),
    #[command(about = "Encrypt/decrypt/inspect transfer files")]
    Crypto(CryptoArgs),
    #[command(about = "Export reports to md/tex/typ formats")]
    Export(ExportArgs),
    #[command(about = "Convert source files to processed JSON")]
    Convert(ConvertArgs),
    #[command(about = "Import processed JSON data into the database")]
    Import(ImportArgs),
    #[command(about = "Run full ingestion pipeline", visible_alias = "blink")]
    Ingest(IngestArgs),
    #[command(name = "validate-logic", about = "Validate business logic rules")]
    ValidateLogic(ValidateLogicArgs),
    #[command(
        name = "validate-structure",
        about = "Validate source TXT syntax and structure"
    )]
    ValidateStructure(ValidateStructureArgs),
    #[command(about = "Display project structure as a tree.")]
    Tree(TreeArgs),
    #[command(about = "Run runtime dependency/config diagnostics")]
    Doctor(DoctorArgs),
    #[command(about = "Print third-party dependency licenses")]
    Licenses(LicensesArgs),
    #[command(about = "Print the tracer easter egg line")]
    Tracer,
    #[command(about = "Print the project motto easter egg", visible_alias = "zen")]
    Motto,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum QueryType {
    Day,
    Month,
    Week,
    Year,
    Recent,
    Range,
    Data,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum QueryFormat {
    Md,
    Tex,
    Typ,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum DataOutputMode {
    Text,
    Json,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum SuggestScoreMode {
    Frequency,
    Duration,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum QueryPeriod {
    Day,
    Week,
    Month,
    Year,
    Recent,
    Range,
}

#[derive(Debug, Args)]
pub struct QueryArgs {
    #[arg(value_enum, help = "Query type")]
    pub query_type: QueryType,
    #[arg(help = "Date/range arg or data action")]
    pub argument: String,
    #[arg(short = 'f', long = "format", value_enum, value_delimiter = ',')]
    pub format: Vec<QueryFormat>,
    #[arg(long = "data-output", value_enum)]
    pub data_output: Option<DataOutputMode>,
    #[arg(long)]
    pub year: Option<i32>,
    #[arg(long)]
    pub month: Option<i32>,
    #[arg(long)]
    pub from: Option<String>,
    #[arg(long)]
    pub to: Option<String>,
    #[arg(long)]
    pub remark: Option<String>,
    #[arg(long = "day-remark", alias = "remark-day")]
    pub day_remark: Option<String>,
    #[arg(
        long = "project",
        help = "Deprecated legacy contains filter by project path (prefer --root)"
    )]
    pub project: Option<String>,
    #[arg(long)]
    pub root: Option<String>,
    #[arg(long, action = ArgAction::SetTrue)]
    pub overnight: bool,
    #[arg(long)]
    pub exercise: Option<i32>,
    #[arg(long)]
    pub status: Option<i32>,
    #[arg(short = 'n', long = "numbers")]
    pub numbers: Option<i32>,
    #[arg(long)]
    pub top: Option<i32>,
    #[arg(long = "lookback-days")]
    pub lookback_days: Option<i32>,
    #[arg(long = "activity-prefix")]
    pub activity_prefix: Option<String>,
    #[arg(long = "score-mode", value_enum)]
    pub score_mode: Option<SuggestScoreMode>,
    #[arg(long, value_enum)]
    pub period: Option<QueryPeriod>,
    #[arg(long = "period-arg")]
    pub period_arg: Option<String>,
    #[arg(short = 'l', long = "level")]
    pub level: Option<i32>,
    #[arg(short = 'r', long = "reverse", action = ArgAction::SetTrue)]
    pub reverse: bool,
}

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

#[derive(Debug, Clone, ValueEnum)]
pub enum CryptoAction {
    Encrypt,
    Decrypt,
    Inspect,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum SecurityLevel {
    Min,
    Interactive,
    Moderate,
    #[value(alias = "sensitive")]
    High,
    Max,
}

#[derive(Debug, Args)]
pub struct CryptoArgs {
    #[arg(value_enum)]
    pub action: CryptoAction,
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
    #[arg(
        long = "out",
        value_name = "PATH",
        required_if_eq_any = [("action", "encrypt"), ("action", "decrypt")]
    )]
    pub output: Option<String>,
    #[arg(
        long = "security-level",
        value_enum,
        help = "Encryption KDF profile (encrypt only): min|interactive|moderate|high|max (alias: sensitive)"
    )]
    pub security_level: Option<SecurityLevel>,
}

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

#[derive(Debug, Args)]
pub struct TreeArgs {
    #[arg(help = "Root project path filter (e.g., study, study_math)")]
    pub root: Option<String>,
    #[arg(short = 'l', long = "level", help = "Max depth level")]
    pub level: Option<i32>,
    #[arg(
        short = 'r',
        long = "roots",
        action = ArgAction::SetTrue,
        conflicts_with = "root",
        help = "List all root projects"
    )]
    pub roots: bool,
}

#[derive(Debug, Args)]
pub struct DoctorArgs {
    #[arg(long = "json", action = ArgAction::SetTrue, help = "Reserved: emit machine-readable diagnostics")]
    pub json: bool,
}

#[derive(Debug, Args)]
pub struct LicensesArgs {
    #[arg(
        long = "full",
        action = ArgAction::SetTrue,
        help = "Emit detailed third-party license fields"
    )]
    pub full: bool,
}
