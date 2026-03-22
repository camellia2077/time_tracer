use clap::{ArgAction, Args, Subcommand, ValueEnum};

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
pub struct QueryDataArgs {
    #[arg(help = "Semantic data action")]
    pub action: String,
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
    #[arg(long = "day-remark")]
    pub day_remark: Option<String>,
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

#[derive(Debug, Args)]
pub struct QueryTreeArgs {
    pub root: Option<String>,
    #[arg(short = 'l', long = "level")]
    pub level: Option<i32>,
    #[arg(short = 'r', long = "roots", action = ArgAction::SetTrue, conflicts_with = "root")]
    pub roots: bool,
}

#[derive(Debug, Subcommand)]
pub enum QueryCommand {
    #[command(about = "Run semantic data queries")]
    Data(QueryDataArgs),
    #[command(about = "Display project structure as a tree")]
    Tree(QueryTreeArgs),
}

#[derive(Debug, Args)]
pub struct QueryArgs {
    #[command(subcommand)]
    pub command: QueryCommand,
}
