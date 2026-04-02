use clap::{ArgAction, Parser, Subcommand};

mod chart;
mod doctor;
mod exchange;
mod licenses;
mod pipeline;
mod query;
mod report;

pub use chart::{ChartArgs, ChartTheme, ChartType};
pub use doctor::DoctorArgs;
pub use exchange::{
    ExchangeArgs, ExchangeCommand, ExchangeExportArgs, ExchangeImportArgs, ExchangeInspectArgs,
    SecurityLevel,
};
pub use licenses::LicensesArgs;
pub use pipeline::{
    DateCheckMode, PipelineArgs, PipelineCommand, PipelineConvertArgs, PipelineImportArgs,
    PipelineIngestArgs, PipelineValidateAllArgs, PipelineValidateArgs, PipelineValidateCommand,
    PipelineValidateLogicArgs, PipelineValidateStructureArgs,
};
pub use query::{
    DataOutputMode, QueryArgs, QueryCommand, QueryDataArgs, QueryPeriod, QueryTreeArgs,
    SuggestScoreMode,
};
pub use report::{
    ReportArgs, ReportCommand, ReportExportArgs, ReportExportPeriod, ReportFormat,
    ReportRenderArgs, ReportRenderPeriod,
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
        help = "Output path override. Required for `exchange export`."
    )]
    pub output: Option<String>,
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Debug, Subcommand)]
pub enum Command {
    #[command(about = "Run semantic data and tree queries")]
    Query(QueryArgs),
    #[command(about = "Generate report-chart HTML from database data")]
    Chart(ChartArgs),
    #[command(about = "Run pipeline operations against source and processed data")]
    Pipeline(PipelineArgs),
    #[command(about = "Render and export textual reports")]
    Report(ReportArgs),
    #[command(about = "Export/import/inspect tracer exchange packages")]
    Exchange(ExchangeArgs),
    #[command(about = "Run runtime dependency/config diagnostics")]
    Doctor(DoctorArgs),
    #[command(about = "Print third-party dependency licenses")]
    Licenses(LicensesArgs),
    #[command(about = "Print the tracer easter egg line")]
    Tracer,
    #[command(about = "Print the project motto easter egg")]
    Motto,
}

#[cfg(test)]
mod tests {
    use clap::{Parser, error::ErrorKind};

    use super::{
        Cli, Command, DataOutputMode, DateCheckMode, ExchangeCommand, PipelineCommand,
        PipelineValidateCommand, ReportCommand, ReportExportPeriod, ReportRenderPeriod,
    };

    #[test]
    fn version_flag_still_uses_clap_display_version() {
        let error = Cli::try_parse_from(["time_tracer_cli", "--version"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayVersion);
    }

    #[test]
    fn query_data_still_parses_data_output() {
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
            Command::Query(args) => match args.command {
                super::QueryCommand::Data(args) => {
                    assert!(matches!(args.data_output, Some(DataOutputMode::Json)));
                    assert_eq!(args.action, "days_duration");
                }
                _ => panic!("expected query data command"),
            },
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
    fn exchange_export_still_parses_required_fields() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "exchange",
            "export",
            "--in",
            "input_dir",
            "--security-level",
            "high",
        ])
        .unwrap();

        match cli.command {
            Command::Exchange(args) => match args.command {
                ExchangeCommand::Export(args) => {
                    assert_eq!(args.input, "input_dir");
                }
                _ => panic!("expected exchange export command"),
            },
            _ => panic!("expected exchange command"),
        }
    }

    #[test]
    fn exchange_import_still_parses_without_output() {
        let cli =
            Cli::try_parse_from(["time_tracer_cli", "exchange", "import", "--in", "a.tracer"])
                .unwrap();
        assert!(cli.output.is_none());

        match cli.command {
            Command::Exchange(args) => match args.command {
                ExchangeCommand::Import(args) => {
                    assert_eq!(args.input, "a.tracer");
                }
                _ => panic!("expected exchange import command"),
            },
            _ => panic!("expected exchange command"),
        }
    }

    #[test]
    fn exchange_export_help_mentions_required_output_and_usage() {
        let error =
            Cli::try_parse_from(["time_tracer_cli", "exchange", "export", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayHelp);
        let help = error.to_string();
        assert!(help.contains(
            "Usage: time_tracer_cli.exe -o <PATH> exchange export --in <PATH> [--security-level <SECURITY_LEVEL>]"
        ));
        assert!(help.contains("Output path override. Required for `exchange export`."));
    }

    #[test]
    fn pipeline_ingest_still_rejects_conflicting_date_check_flags() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "pipeline",
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
    fn pipeline_validate_logic_still_parses_shared_date_check_mode() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "pipeline",
            "validate",
            "logic",
            "input.txt",
            "--date-check",
            "continuity",
        ])
        .unwrap();

        match cli.command {
            Command::Pipeline(args) => match args.command {
                PipelineCommand::Validate(args) => match args.command {
                    PipelineValidateCommand::Logic(args) => {
                        assert!(matches!(args.date_check, Some(DateCheckMode::Continuity)));
                        assert_eq!(args.path, "input.txt");
                    }
                    _ => panic!("expected pipeline validate logic command"),
                },
                _ => panic!("expected pipeline validate command"),
            },
            _ => panic!("expected pipeline command"),
        }
    }

    #[test]
    fn pipeline_validate_all_still_parses_path_only_form() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "pipeline",
            "validate",
            "all",
            "input.txt",
        ])
        .unwrap();

        match cli.command {
            Command::Pipeline(args) => match args.command {
                PipelineCommand::Validate(args) => match args.command {
                    PipelineValidateCommand::All(args) => assert_eq!(args.path, "input.txt"),
                    _ => panic!("expected pipeline validate all command"),
                },
                _ => panic!("expected pipeline validate command"),
            },
            _ => panic!("expected pipeline command"),
        }
    }

    #[test]
    fn report_export_all_recent_still_parses_argument() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "report",
            "export",
            "recent",
            "7,10",
            "--all",
        ])
        .unwrap();

        match cli.command {
            Command::Report(args) => match args.command {
                ReportCommand::Export(args) => {
                    assert!(args.all);
                    assert_eq!(args.argument.as_deref(), Some("7,10"));
                    assert!(matches!(args.period, ReportExportPeriod::Recent));
                }
                _ => panic!("expected report export command"),
            },
            _ => panic!("expected report command"),
        }
    }

    #[test]
    fn report_render_range_still_parses_period_and_argument() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "report",
            "render",
            "range",
            "20260101|20260131",
        ])
        .unwrap();

        match cli.command {
            Command::Report(args) => match args.command {
                ReportCommand::Render(args) => {
                    assert!(matches!(args.period, ReportRenderPeriod::Range));
                    assert_eq!(args.argument, "20260101|20260131");
                }
                _ => panic!("expected report render command"),
            },
            _ => panic!("expected report command"),
        }
    }

    #[test]
    fn report_render_help_mentions_iso_normalization_rules() {
        let error =
            Cli::try_parse_from(["time_tracer_cli", "report", "render", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayHelp);
        let help = error.to_string();
        assert!(help.contains("day: YYYYMMDD or YYYY-MM-DD"));
        assert!(help.contains("month: YYYYMM or YYYY-MM"));
        assert!(help.contains("range: <from>|<to>"));
        assert!(help.contains("normalized to ISO YYYY-MM-DD before querying"));
    }

    #[test]
    fn report_export_help_mentions_iso_normalization_rules() {
        let error =
            Cli::try_parse_from(["time_tracer_cli", "report", "export", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayHelp);
        let help = error.to_string();
        assert!(help.contains("day: YYYYMMDD or YYYY-MM-DD"));
        assert!(help.contains("month: YYYYMM or YYYY-MM"));
        assert!(help.contains("normalized to ISO YYYY-MM-DD before querying and output naming"));
    }

    #[test]
    fn query_tree_still_rejects_root_and_roots_together() {
        let error = Cli::try_parse_from(["time_tracer_cli", "query", "tree", "study", "--roots"])
            .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::ArgumentConflict);
    }

    #[test]
    fn global_database_alias_is_removed() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "--database",
            "time_data.sqlite3",
            "query",
            "data",
            "years",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::UnknownArgument);
    }

    #[test]
    fn global_out_alias_is_removed() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "--out",
            "report.md",
            "report",
            "render",
            "day",
            "20260101",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::UnknownArgument);
    }

    #[test]
    fn motto_zen_alias_is_removed() {
        let error = Cli::try_parse_from(["time_tracer_cli", "zen"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::InvalidSubcommand);
    }

    #[test]
    fn query_project_option_is_removed() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "query",
            "data",
            "search",
            "--project",
            "study",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::UnknownArgument);
    }

    #[test]
    fn query_remark_day_alias_is_removed() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "query",
            "data",
            "search",
            "--remark-day",
            "focus",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::UnknownArgument);
    }

    #[test]
    fn exchange_sensitive_alias_is_removed() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "exchange",
            "export",
            "--in",
            "input_dir",
            "--security-level",
            "sensitive",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::InvalidValue);
    }
}
