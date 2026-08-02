use clap::{Args, Subcommand};

#[derive(Debug, Args)]
pub struct ActivityArgs {
    #[command(subcommand)]
    pub command: ActivityCommand,
}

#[derive(Debug, Subcommand)]
pub enum ActivityCommand {
    #[command(
        name = "merge",
        about = "Merge one leaf activity into another and migrate TXT/database",
        long_about = r#"Merge one canonical leaf activity into another leaf activity.

The source leaf and all of its aliases are removed from TOML. Existing TXT
events using the source canonical or aliases are rewritten to the destination,
then the database is rebuilt and swapped only after successful ingestion.
Group merge is intentionally not supported.

Example:
  time_tracer_cli --db data/time_data.sqlite3 activity merge \
    --file config/user/activity_hierarchy/exercise.toml \
    --from cardio.running.treadmill --into cardio.running.track-running \
    --input test/data"#
    )]
    Merge(ActivityMergeArgs),
}

#[derive(Debug, Args)]
pub struct ActivityMergeArgs {
    #[arg(long, value_name = "PATH", help = "Canonical TOML file")]
    pub file: String,
    #[arg(long, value_name = "LEAF", help = "Source leaf activity to merge away")]
    pub from: String,
    #[arg(long, value_name = "LEAF", help = "Destination leaf activity to keep")]
    pub into: String,
    #[arg(long, value_name = "PATH", help = "TXT input directory to rebuild")]
    pub input: String,
}
