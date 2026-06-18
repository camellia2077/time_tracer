use clap::{ArgGroup, Args, Subcommand};

#[derive(Debug, Args)]
pub struct TxtViewDayArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
    #[arg(long = "day", value_name = "MMDD")]
    pub day: String,
}

#[derive(Debug, Args)]
#[command(
    group(
        ArgGroup::new("event_shape")
            .required(true)
            .args(["time", "start"])
    )
)]
pub struct TxtAppendEventArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
    #[arg(long = "day", value_name = "MMDD")]
    pub day: String,
    #[arg(
        long = "time",
        value_name = "HHMM",
        conflicts_with_all = ["start", "end"]
    )]
    pub time: Option<String>,
    #[arg(long = "start", value_name = "HHMM", conflicts_with = "time", requires = "end")]
    pub start: Option<String>,
    #[arg(long = "end", value_name = "HHMM", conflicts_with = "time", requires = "start")]
    pub end: Option<String>,
    #[arg(long = "activity", value_name = "TOKEN")]
    pub activity: String,
    #[arg(long = "remark", value_name = "TEXT")]
    pub remark: Option<String>,
}

#[derive(Debug, Subcommand)]
pub enum TxtCommand {
    #[command(about = "Display a single day block from a monthly TXT file")]
    ViewDay(TxtViewDayArgs),
    #[command(
        about = "Append one authored event line to an existing day block",
        after_help = "Point events use --time HHMM and render as HHMMtoken.\nInterval events use --start HHMM --end HHMM and render as HHMM-HHMMtoken."
    )]
    AppendEvent(TxtAppendEventArgs),
}

#[derive(Debug, Args)]
#[command(
    after_help = "Shared TXT day-block semantics stay in core.\nAuthored event lines may be written as HHMMtoken or HHMM-HHMMtoken."
)]
pub struct TxtArgs {
    #[command(subcommand)]
    pub command: TxtCommand,
}
