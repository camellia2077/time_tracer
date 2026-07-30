use clap::{Args, Subcommand};

#[derive(Debug, Args)]
#[command(after_help = r#"Workflow:
  1. Use `add` to add a leaf alias to an existing group.
  2. Use `promote` to turn an existing leaf into a recordable group.
  3. Use `move` when TXT files and the database must stay consistent.
  4. Use `move-config` for TOML-only hierarchy editing and diagnostics.
  5. Use `rename-parent` when the TOML file name, parent, TXT files, and
     database must move together.

Canonical paths are derived from parent, group path, and leaf name. Moving a
leaf or group move therefore changes canonical paths. Add --to-file to move
between two existing alias TOMLs; use --group for a complete group subtree.
Omit --to-file for the legacy same-file leaf move."#)]
pub struct AliasArgs {
    #[command(subcommand)]
    pub command: AliasCommand,
}

#[derive(Debug, Subcommand)]
pub enum AliasCommand {
    #[command(
        name = "create",
        about = "Create a new activity hierarchy TOML file",
        long_about = r#"Create a new empty activity hierarchy TOML file.

The file name is also used as the hierarchy parent. The command creates the
file under an `activity_hierarchy` directory and does not require runtime
initialization.

Example:
  time_tracer_cli alias create --file config/user/activity_hierarchy/study.toml"#
    )]
    Create(AliasCreateArgs),
    #[command(
        about = "Add a normal alias entry to an existing group",
        long_about = r#"Add an alias to a canonical leaf in an existing group.

Example:
  time_tracer_cli alias add --file config/user/activity_hierarchy/study.toml \
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
  time_tracer_cli alias promote --file config/user/activity_hierarchy/study.toml --alias 重积分"#
    )]
    Promote(AliasFileArgs),
    #[command(
        about = "Move a leaf or group subtree and migrate TXT/database data",
        long_about = r#"Move a canonical leaf or a complete group subtree to another group.
This changes the canonical paths, replaces old canonical paths in every TXT
file under `--input`, ingests a candidate database, and swaps
the database only after the candidate succeeds.

Use `--alias` for a leaf or `--group` for a group subtree. Add `--to-file` to
move to another existing alias TOML. Group moves require `--to-file`.

Use this when source TXT data and the active database must remain consistent
with the new TOML hierarchy.

Example:
  time_tracer_cli alias move --file config/user/activity_hierarchy/study.toml \
    --to-file config/user/activity_hierarchy/meal.toml --alias 二重积分 --to root --input test/data"#
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
    --file config/user/activity_hierarchy/exercise.toml --group cardio --name conditioning \
    --input test/data"#
    )]
    RenameGroup(AliasRenameGroupArgs),
    #[command(
        name = "rename-parent",
        about = "Rename an activity hierarchy parent and migrate TOML, TXT, and database",
        long_about = r#"Rename the parent of one activity hierarchy document.

The TOML `parent` and its filename are treated as one value. This command
updates the parent, renames `<old-parent>.toml` to `<new-parent>.toml`,
rewrites matching canonical activity paths in every TXT file under `--input`,
then rebuilds and swaps the database after successful ingestion.

Example:
  time_tracer_cli --db data/time_data.sqlite3 alias rename-parent \
    --file config/user/activity_hierarchy/exercise.toml --name training \
    --input test/data"#
    )]
    RenameParent(AliasRenameParentArgs),
    #[command(
        about = "Render an alias TOML hierarchy as plaintext",
        long_about = r#"Render the hierarchy from one alias TOML file as plaintext.

By default only canonical node names are printed. Add `--show-aliases` to show
normal aliases and recordable group aliases next to their nodes.

Example:
  time_tracer_cli alias tree --file config/user/activity_hierarchy/study.toml --show-aliases"#
    )]
    Tree(AliasTreeArgs),
    #[command(
        about = "Move a leaf or group subtree in TOML only",
        long_about = r#"Move a canonical leaf or a complete group subtree to another group
without modifying TXT files or the database.

The command prints the old and new canonical paths. Existing TXT files that
still use the old canonical path may not resolve with the new TOML until they
are migrated separately.

Use `--alias` for a leaf or `--group` for a group subtree. Add `--to-file` for
a Core-validated cross-document move; group moves require it. Without it, the
command keeps the same-file leaf behavior.

Use this for configuration editing, inspection, or repair workflows where TXT
and database migration is intentionally handled later.

Example:
  time_tracer_cli alias move-config --file config/user/activity_hierarchy/study.toml \
    --to-file config/user/activity_hierarchy/meal.toml --alias 二重积分 --to root"#
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
pub struct AliasCreateArgs {
    #[arg(long, value_name = "PATH", help = "New activity hierarchy TOML file")]
    pub file: String,
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
    #[arg(long, value_name = "PATH", help = "Source alias TOML file")]
    pub file: String,
    #[arg(
        long,
        value_name = "ALIAS",
        conflicts_with = "group",
        required_unless_present = "group",
        help = "Alias belonging to the canonical leaf"
    )]
    pub alias: Option<String>,
    #[arg(
        long,
        value_name = "GROUP",
        conflicts_with = "alias",
        required_unless_present = "alias",
        help = "Canonical group path to move with its complete subtree"
    )]
    pub group: Option<String>,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Target group path, e.g. cardio or math.calculus"
    )]
    pub to: String,
    #[arg(long, value_name = "PATH", help = "Destination alias TOML file")]
    pub to_file: Option<String>,
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
        conflicts_with = "group",
        required_unless_present = "group",
        help = "Alias belonging to the canonical leaf"
    )]
    pub alias: Option<String>,
    #[arg(
        long,
        value_name = "GROUP",
        conflicts_with = "alias",
        required_unless_present = "alias",
        help = "Canonical group path to move with its complete subtree"
    )]
    pub group: Option<String>,
    #[arg(
        long,
        value_name = "GROUP",
        help = "Target group path, e.g. cardio or math.calculus"
    )]
    pub to: String,
    #[arg(long, value_name = "PATH", help = "Destination alias TOML file")]
    pub to_file: Option<String>,
}

#[derive(Debug, Args)]
pub struct AliasRenameParentArgs {
    #[arg(
        long,
        value_name = "PATH",
        help = "Current activity hierarchy TOML file"
    )]
    pub file: String,
    #[arg(long, value_name = "NAME", help = "New parent and TOML file stem")]
    pub name: String,
    #[arg(long, value_name = "PATH", help = "TXT input directory to rebuild")]
    pub input: String,
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
