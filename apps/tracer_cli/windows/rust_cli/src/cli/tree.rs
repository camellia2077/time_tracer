use clap::{ArgAction, Args};

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
