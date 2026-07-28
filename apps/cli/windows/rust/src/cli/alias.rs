use clap::{Args, Subcommand};

#[derive(Debug, Args)]
#[command(after_help = r#"Workflow:
  1. Use `add` to add a leaf alias to an existing group.
  2. Use `promote` to turn an existing leaf into a recordable group.
  3. Use `move` when TXT files and the database must stay consistent.
  4. Use `move-config` for TOML-only hierarchy editing and diagnostics.

Canonical paths are derived from parent, group path, and leaf name. Moving a
leaf therefore changes its canonical path."#)]
pub struct AliasArgs {
    #[command(subcommand)]
    pub command: AliasCommand,
}

#[derive(Debug, Subcommand)]
pub enum AliasCommand {
    #[command(
        about = "Add a normal alias entry to an existing group",
        long_about = r#"Add an alias to a canonical leaf in an existing group.

Example:
  time_tracer_cli alias add --file config/aliases/study.toml \
    --group math.calculus --canonical multiple-integral --alias 重积分

The target group must already exist. Use `promote` to turn the new leaf into a
recordable group before adding children to it."#
    )]
    Add(AliasAddArgs),
    #[command(
        about = "Promote an alias leaf to a recordable group",
        long_about = r#"Promote an existing alias leaf into a recordable group.

The canonical leaf name is preserved, while the selected alias becomes the
group's `group_aliases` entry. This is useful for creating a new hierarchy
level with `add` followed by `promote`.

Example:
  time_tracer_cli alias promote --file config/aliases/study.toml --alias 重积分"#
    )]
    Promote(AliasFileArgs),
    #[command(
        about = "Move a canonical leaf and migrate TXT/database data",
        long_about = r#"Move an entire canonical leaf, including all of its aliases,
to another group. This changes the canonical path, replaces the old canonical
path in every TXT file under `--input`, ingests a candidate database, and swaps
the database only after the candidate succeeds.

Use this when source TXT data and the active database must remain consistent
with the new TOML hierarchy.

Example:
  time_tracer_cli alias move --file config/aliases/study.toml \
    --alias 二重积分 --to math.calculus.multiple-integral --input test/data"#
    )]
    Move(AliasMoveArgs),
    #[command(
        name = "rename-group",
        about = "Rename a group canonical and migrate nested canonicals, TXT, and database",
        long_about = r#"Rename a group canonical path and migrate all canonical nodes below it.

This updates the selected group, nested groups, and leaf activities in the TOML,
rewrites matching canonical activity names in every TXT file under `--input`,
then rebuilds and swaps the database after successful ingestion.

Example:
  time_tracer_cli --db data/time_data.sqlite3 alias rename-group \
    --file config/aliases/exercise.toml --group cardio --name conditioning \
    --input test/data"#
    )]
    RenameGroup(AliasRenameGroupArgs),
    #[command(
        about = "Render an alias TOML hierarchy as plaintext",
        long_about = r#"Render the hierarchy from one alias TOML file as plaintext.

By default only canonical node names are printed. Add `--show-aliases` to show
normal aliases and recordable group aliases next to their nodes.

Example:
  time_tracer_cli alias tree --file config/aliases/study.toml --show-aliases"#
    )]
    Tree(AliasTreeArgs),
    #[command(
        about = "Move a canonical leaf in TOML only",
        long_about = r#"Move an entire canonical leaf, including all of its aliases,
to another group without modifying TXT files or the database.

The command prints the old and new canonical paths. Existing TXT files that
still use the old canonical path may not resolve with the new TOML until they
are migrated separately.

Use this for configuration editing, inspection, or repair workflows where TXT
and database migration is intentionally handled later.

Example:
  time_tracer_cli alias move-config --file config/aliases/study.toml \
    --alias 二重积分 --to math.calculus.multiple-integral"#
    )]
    MoveConfig(AliasMoveConfigArgs),
    #[command(
        about = "Rename a recordable alias on a group",
        long_about = "Rename one value in a group's `group_aliases` list. The group canonical path does not change."
    )]
    RenameGroupAlias(AliasGroupArgs),
    #[command(
        about = "Add a recordable alias to a group",
        long_about = "Add a value to a group's `group_aliases` list. The group canonical path does not change."
    )]
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
pub struct AliasRenameGroupArgs {
    #[arg(long, value_name = "PATH", help = "Alias TOML file")]
    pub file: String,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Group path, e.g. cardio or cardio.running"
    )]
    pub group: String,
    #[arg(long, value_name = "NAME", help = "New canonical group name")]
    pub name: String,
    #[arg(long, value_name = "PATH", help = "TXT input directory to rebuild")]
    pub input: String,
}

#[derive(Debug, Args)]
pub struct AliasMoveConfigArgs {
    #[arg(long, value_name = "PATH", help = "Alias TOML file")]
    pub file: String,
    #[arg(
        long,
        value_name = "ALIAS",
        help = "Alias belonging to the canonical leaf"
    )]
    pub alias: String,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Target group path, e.g. cardio or math.calculus"
    )]
    pub to: String,
}

#[derive(Debug, Args)]
pub struct AliasTreeArgs {
    #[arg(long, value_name = "PATH", help = "Alias TOML file to render")]
    pub file: String,
    #[arg(
        long,
        help = "Include normal aliases and recordable group aliases in the plaintext tree"
    )]
    pub show_aliases: bool,
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
