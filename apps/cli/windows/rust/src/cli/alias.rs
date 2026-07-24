use clap::{Args, Subcommand};

#[derive(Debug, Args)]
pub struct AliasArgs {
    #[command(subcommand)]
    pub command: AliasCommand,
}

#[derive(Debug, Subcommand)]
pub enum AliasCommand {
    #[command(about = "Add a normal alias entry to an existing group")]
    Add(AliasAddArgs),
    #[command(about = "Promote an alias leaf to a recordable group")]
    Promote(AliasFileArgs),
    #[command(about = "Move an alias leaf into another group and rebuild the database")]
    Move(AliasMoveArgs),
    #[command(about = "Rename a recordable alias on a group")]
    RenameGroupAlias(AliasGroupArgs),
    #[command(about = "Add a recordable alias to a group")]
    AddGroupAlias(AliasGroupArgs),
}

#[derive(Debug, Args)]
pub struct AliasAddArgs {
    #[arg(long, value_name = "PATH", help = "Alias TOML file")]
    pub file: String,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Target group path, e.g. cardio or cardio.running; use root for [aliases]"
    )]
    pub group: String,
    #[arg(long, value_name = "ALIAS", help = "Alias key")]
    pub alias: String,
    #[arg(long, value_name = "CANONICAL", help = "Canonical leaf")]
    pub canonical: String,
}

#[derive(Debug, Args)]
pub struct AliasFileArgs {
    #[arg(long, value_name = "PATH", help = "Alias TOML file")]
    pub file: String,
    #[arg(long, value_name = "ALIAS", help = "Alias key")]
    pub alias: String,
}

#[derive(Debug, Args)]
pub struct AliasMoveArgs {
    #[command(flatten)]
    pub file_args: AliasFileArgs,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Target group path, e.g. cardio or math.calculus"
    )]
    pub to: String,
    #[arg(long, value_name = "PATH", help = "TXT input directory to rebuild")]
    pub input: String,
}

#[derive(Debug, Args)]
pub struct AliasGroupArgs {
    #[arg(long, value_name = "PATH", help = "Alias TOML file")]
    pub file: String,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Group path, e.g. cardio or math.calculus"
    )]
    pub group: String,
    #[arg(long, value_name = "ALIAS", help = "Recordable group alias")]
    pub alias: String,
    #[arg(
        long,
        value_name = "ALIAS",
        requires = "alias",
        help = "Existing alias to rename"
    )]
    pub old_alias: Option<String>,
}
