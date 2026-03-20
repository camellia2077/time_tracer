use clap::{ArgAction, Parser, Subcommand};

mod chart;
mod crypto;
mod doctor;
mod export;
mod licenses;
mod query;
mod tree;
mod workflow;

pub use chart::{ChartArgs, ChartTheme, ChartType};
pub use crypto::{CryptoAction, CryptoArgs, SecurityLevel};
pub use doctor::DoctorArgs;
pub use export::{ExportArgs, ExportFormat, ExportType};
pub use licenses::LicensesArgs;
pub use query::{DataOutputMode, QueryArgs, QueryFormat, QueryPeriod, QueryType, SuggestScoreMode};
pub use tree::TreeArgs;
pub use workflow::{
    ConvertArgs, DateCheckMode, ImportArgs, IngestArgs, ValidateArgs, ValidateLogicArgs,
    ValidateStructureArgs, ValidateTarget,
};

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
        visible_alias = "out",
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
    #[command(about = "Validate source structure and business logic (default: all)")]
    Validate(ValidateArgs),
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

#[cfg(test)]
mod tests {
    use clap::{Parser, error::ErrorKind};

    use super::{Cli, Command, CryptoAction, DataOutputMode, DateCheckMode, ValidateTarget};

    #[test]
    fn version_flag_still_uses_clap_display_version() {
        let error = Cli::try_parse_from(["time_tracer_cli", "--version"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayVersion);
    }

    #[test]
    fn query_still_parses_data_output() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "query",
            "data",
            "days_duration",
            "--data-output",
            "json",
        ])
        .unwrap();

        match cli.command {
            Command::Query(args) => {
                assert!(matches!(args.data_output, Some(DataOutputMode::Json)));
                assert_eq!(args.argument, "days_duration");
            }
            _ => panic!("expected query command"),
        }
    }

    #[test]
    fn chart_palette_listing_still_conflicts_with_filters() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "chart",
            "--list-heatmap-palettes",
            "--root",
            "study",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::ArgumentConflict);
    }

    #[test]
    fn crypto_encrypt_still_parses_required_fields() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "crypto",
            "encrypt",
            "--in",
            "input_dir",
            "--out",
            "output.tracer",
            "--security-level",
            "high",
        ])
        .unwrap();
        assert_eq!(cli.output.as_deref(), Some("output.tracer"));

        match cli.command {
            Command::Crypto(args) => {
                assert!(matches!(args.action, CryptoAction::Encrypt));
                assert_eq!(args.input, "input_dir");
            }
            _ => panic!("expected crypto command"),
        }
    }

    #[test]
    fn crypto_decrypt_no_longer_requires_out() {
        let cli = Cli::try_parse_from(["time_tracer_cli", "crypto", "decrypt", "--in", "a.tracer"])
            .unwrap();
        assert!(cli.output.is_none());

        match cli.command {
            Command::Crypto(args) => {
                assert!(matches!(args.action, CryptoAction::Decrypt));
                assert_eq!(args.input, "a.tracer");
            }
            _ => panic!("expected crypto command"),
        }
    }

    #[test]
    fn ingest_still_rejects_conflicting_date_check_flags() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "ingest",
            "input.txt",
            "--date-check",
            "none",
            "--no-date-check",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::ArgumentConflict);
    }

    #[test]
    fn validate_logic_still_parses_shared_date_check_mode() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "validate",
            "logic",
            "input.txt",
            "--date-check",
            "continuity",
        ])
        .unwrap();

        match cli.command {
            Command::Validate(args) => match args.target {
                Some(ValidateTarget::Logic(args)) => {
                    assert!(matches!(args.date_check, Some(DateCheckMode::Continuity)));
                    assert_eq!(args.path, "input.txt");
                }
                _ => panic!("expected validate logic command"),
            },
            _ => panic!("expected validate command"),
        }
    }

    #[test]
    fn validate_structure_still_parses_path_only_form() {
        let cli =
            Cli::try_parse_from(["time_tracer_cli", "validate", "structure", "input.txt"]).unwrap();

        match cli.command {
            Command::Validate(args) => match args.target {
                Some(ValidateTarget::Structure(args)) => assert_eq!(args.path, "input.txt"),
                _ => panic!("expected validate structure command"),
            },
            _ => panic!("expected validate command"),
        }
    }

    #[test]
    fn validate_default_path_parses_as_all_form() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "validate",
            "input.txt",
            "--date-check",
            "full",
        ])
        .unwrap();

        match cli.command {
            Command::Validate(args) => {
                assert!(args.target.is_none());
                assert_eq!(args.path.as_deref(), Some("input.txt"));
                assert!(matches!(args.date_check, Some(DateCheckMode::Full)));
            }
            _ => panic!("expected validate command"),
        }
    }

    #[test]
    fn tree_still_rejects_root_and_roots_together() {
        let error =
            Cli::try_parse_from(["time_tracer_cli", "tree", "study", "--roots"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::ArgumentConflict);
    }
}
