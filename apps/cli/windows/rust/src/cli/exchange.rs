use clap::{Args, Subcommand, ValueEnum};

#[derive(Debug, Clone, Copy, ValueEnum)]
pub enum SecurityLevel {
    Min,
    Interactive,
    Moderate,
    High,
    Max,
}

#[derive(Debug, Args)]
pub struct ExchangeExportArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
    #[arg(
        long = "security-level",
        value_enum,
        help = "Encryption KDF profile: min|interactive|moderate|high|max"
    )]
    pub security_level: Option<SecurityLevel>,
}

#[derive(Debug, Args)]
pub struct ExchangeImportArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
}

#[derive(Debug, Args)]
pub struct ExchangeUnpackArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
}

#[derive(Debug, Args)]
pub struct ExchangeInspectArgs {
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
}

#[derive(Debug, Subcommand)]
pub enum ExchangeCommand {
    #[command(
        about = "Export a tracer exchange package from a TXT root",
        override_usage = "time_tracer_cli.exe -o <PATH> exchange export --in <PATH> [--security-level <SECURITY_LEVEL>]"
    )]
    Export(ExchangeExportArgs),
    #[command(about = "Import a tracer exchange package into the active runtime")]
    Import(ExchangeImportArgs),
    #[command(
        about = "Decrypt and unpack a tracer exchange package without applying it",
        override_usage = "time_tracer_cli.exe -o <PATH> exchange unpack --in <PATH>"
    )]
    Unpack(ExchangeUnpackArgs),
    #[command(about = "Inspect a tracer exchange package")]
    Inspect(ExchangeInspectArgs),
}

#[derive(Debug, Args)]
pub struct ExchangeArgs {
    #[command(subcommand)]
    pub command: ExchangeCommand,
}
