use clap::{ArgAction, Parser, Subcommand};

mod about;
mod alias;
mod chart;
mod doctor;
mod exchange;
mod licenses;
mod pipeline;
mod query;
mod report;
mod system;
mod txt;

pub use about::{AboutArgs, AboutCommand};
pub use alias::{
    AliasAddArgs, AliasArgs, AliasCommand, AliasFileArgs, AliasGroupArgs, AliasMoveArgs,
    AliasMoveConfigArgs, AliasRenameGroupArgs, AliasTreeArgs,
};
pub use chart::{ChartArgs, ChartTheme, ChartType};
pub use doctor::DoctorArgs;
pub use exchange::{
    ExchangeArgs, ExchangeCommand, ExchangeExportArgs, ExchangeImportArgs, ExchangeInspectArgs,
    ExchangeUnpackArgs, SecurityLevel,
};
pub use licenses::LicensesArgs;
pub use pipeline::{
    DateCheckMode, PipelineArgs, PipelineCommand, PipelineConvertArgs, PipelineImportArgs,
    PipelineIngestArgs, PipelineValidateAllArgs, PipelineValidateArgs, PipelineValidateBundleArgs,
    PipelineValidateCommand, PipelineValidateLogicArgs, PipelineValidateStructureArgs,
};
pub use query::{
    DataOutputMode, QueryArgs, QueryCommand, QueryDataArgs, QueryPeriod, QueryTreeArgs,
    SuggestScoreMode,
};
pub use report::{
    ReportArgs, ReportCommand, ReportExportArgs, ReportExportPeriod, ReportFormat,
    ReportRenderArgs, ReportRenderPeriod,
};
pub use system::{SystemArgs, SystemCommand};
pub use txt::{TxtAppendEventArgs, TxtArgs, TxtCommand, TxtViewDayArgs};

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
    #[command(about = "Edit alias hierarchy TOML and migrate canonical activity paths")]
    Alias(AliasArgs),
    #[command(about = "Run semantic data and tree queries")]
    Query(QueryArgs),
    #[command(about = "Run pipeline operations against source and processed data")]
    Pipeline(PipelineArgs),
    #[command(about = "Render, export, and chart reports")]
    Report(ReportArgs),
    #[command(about = "Export/import/inspect tracer exchange packages")]
    Exchange(ExchangeArgs),
    #[command(about = "Inspect monthly TXT files through shared day-block semantics")]
    Txt(TxtArgs),
    #[command(about = "Run runtime/system inspection commands")]
    System(SystemArgs),
    #[command(about = "Print project/about information and easter eggs")]
    About(AboutArgs),
}

#[cfg(test)]
mod tests {
    use clap::{Parser, error::ErrorKind};

    use super::{
        AboutCommand, AliasCommand, Cli, Command, DataOutputMode, DateCheckMode, ExchangeCommand,
        PipelineCommand, PipelineValidateCommand, ReportCommand, ReportExportPeriod,
        ReportRenderPeriod, SystemCommand, TxtCommand,
    };

    #[test]
    fn version_flag_still_uses_clap_display_version() {
        let error = Cli::try_parse_from(["time_tracer_cli", "--version"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayVersion);
    }

    #[test]
    fn alias_rename_group_parses_txt_input_and_new_name() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "--db",
            "data/time_data.sqlite3",
            "alias",
            "rename-group",
            "--file",
            "config/aliases/exercise.toml",
            "--group",
            "cardio",
            "--name",
            "conditioning",
            "--input",
            "test/data",
        ])
        .unwrap();

        match cli.command {
            Command::Alias(args) => match args.command {
                AliasCommand::RenameGroup(args) => {
                    assert_eq!(args.group, "cardio");
                    assert_eq!(args.name, "conditioning");
                    assert_eq!(args.input, "test/data");
                }
                _ => panic!("expected alias rename-group command"),
            },
            _ => panic!("expected alias command"),
        }
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
            "report",
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
    fn exchange_unpack_still_requires_output_before_execution() {
        let cli =
            Cli::try_parse_from(["time_tracer_cli", "exchange", "unpack", "--in", "a.tracer"])
                .unwrap();
        assert!(cli.output.is_none());

        match cli.command {
            Command::Exchange(args) => match args.command {
                ExchangeCommand::Unpack(args) => {
                    assert_eq!(args.input, "a.tracer");
                }
                _ => panic!("expected exchange unpack command"),
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
    fn txt_view_day_parses_input_and_marker() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "txt",
            "view-day",
            "--in",
            "test/data/2025/2025-01.txt",
            "--day",
            "0102",
        ])
        .unwrap();

        match cli.command {
            Command::Txt(args) => match args.command {
                TxtCommand::ViewDay(args) => {
                    assert_eq!(args.input, "test/data/2025/2025-01.txt");
                    assert_eq!(args.day, "0102");
                }
                _ => panic!("expected txt view-day command"),
            },
            _ => panic!("expected txt command"),
        }
    }

    #[test]
    fn txt_append_event_parses_interval_shape() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "txt",
            "append-event",
            "--in",
            "test/data/2025/2025-01.txt",
            "--day",
            "0102",
            "--start",
            "0900",
            "--end",
            "1030",
            "--activity",
            "study",
            "--remark",
            "focus",
        ])
        .unwrap();

        match cli.command {
            Command::Txt(args) => match args.command {
                TxtCommand::AppendEvent(args) => {
                    assert_eq!(args.input, "test/data/2025/2025-01.txt");
                    assert_eq!(args.day, "0102");
                    assert_eq!(args.start.as_deref(), Some("0900"));
                    assert_eq!(args.end.as_deref(), Some("1030"));
                    assert_eq!(args.activity, "study");
                    assert_eq!(args.remark.as_deref(), Some("focus"));
                }
                _ => panic!("expected txt append-event command"),
            },
            _ => panic!("expected txt command"),
        }
    }

    #[test]
    fn txt_append_event_rejects_point_interval_conflict() {
        let error = Cli::try_parse_from([
            "time_tracer_cli",
            "txt",
            "append-event",
            "--in",
            "test/data/2025/2025-01.txt",
            "--day",
            "0102",
            "--time",
            "0904",
            "--start",
            "0900",
            "--end",
            "1030",
            "--activity",
            "study",
        ])
        .unwrap_err();
        assert_eq!(error.kind(), ErrorKind::ArgumentConflict);
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
    fn pipeline_validate_bundle_parses_external_txt_and_config_paths() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "pipeline",
            "validate",
            "bundle",
            "--txt",
            "temp/txt",
            "--config",
            "temp/config",
            "--no-date-check",
        ])
        .unwrap();

        match cli.command {
            Command::Pipeline(args) => match args.command {
                PipelineCommand::Validate(args) => match args.command {
                    PipelineValidateCommand::Bundle(args) => {
                        assert_eq!(args.txt_path, "temp/txt");
                        assert_eq!(args.config_path, "temp/config");
                        assert!(args.no_date_check);
                    }
                    _ => panic!("expected pipeline validate bundle command"),
                },
                _ => panic!("expected pipeline validate command"),
            },
            _ => panic!("expected pipeline command"),
        }
    }

    #[test]
    fn pipeline_help_mentions_interval_authored_lines() {
        let error = Cli::try_parse_from(["time_tracer_cli", "pipeline", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayHelp);
        let help = error.to_string();
        assert!(help.contains("HHMMtoken"));
        assert!(help.contains("HHMM-HHMMtoken"));
    }

    #[test]
    fn alias_help_explains_move_and_move_config() {
        let error = Cli::try_parse_from(["time_tracer_cli", "alias", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayHelp);
        let help = error.to_string();
        assert!(help.contains("move-config"));
        assert!(help.contains("TXT files and the database"));
        assert!(help.contains("TOML-only hierarchy editing"));

        let error =
            Cli::try_parse_from(["time_tracer_cli", "alias", "move-config", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::DisplayHelp);
        let help = error.to_string();
        assert!(help.contains("without modifying TXT files or the database"));
        assert!(help.contains("--file config/aliases/study.toml"));
    }

    #[test]
    fn alias_tree_parses_file_and_show_aliases() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "alias",
            "tree",
            "--file",
            "config/aliases/study.toml",
            "--show-aliases",
        ])
        .unwrap();
        match cli.command {
            Command::Alias(args) => match args.command {
                super::AliasCommand::Tree(args) => {
                    assert_eq!(args.file, "config/aliases/study.toml");
                    assert!(args.show_aliases);
                }
                _ => panic!("expected alias tree command"),
            },
            _ => panic!("expected alias command"),
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
                    assert_eq!(args.as_of, None);
                    assert!(matches!(args.period, ReportExportPeriod::Recent));
                }
                _ => panic!("expected report export command"),
            },
            _ => panic!("expected report command"),
        }
    }

    #[test]
    fn report_recent_as_of_still_parses_for_render_and_export() {
        let render_cli = Cli::try_parse_from([
            "time_tracer_cli",
            "report",
            "render",
            "recent",
            "7",
            "--as-of",
            "2026-03-07",
        ])
        .unwrap();
        let export_cli = Cli::try_parse_from([
            "time_tracer_cli",
            "report",
            "export",
            "recent",
            "7",
            "--as-of",
            "2026-03-07",
        ])
        .unwrap();

        match render_cli.command {
            Command::Report(args) => match args.command {
                ReportCommand::Render(args) => {
                    assert!(matches!(args.period, ReportRenderPeriod::Recent));
                    assert_eq!(args.as_of.as_deref(), Some("2026-03-07"));
                }
                _ => panic!("expected report render command"),
            },
            _ => panic!("expected report command"),
        }
        match export_cli.command {
            Command::Report(args) => match args.command {
                ReportCommand::Export(args) => {
                    assert!(matches!(args.period, ReportExportPeriod::Recent));
                    assert_eq!(args.as_of.as_deref(), Some("2026-03-07"));
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
        assert!(help.contains("--as-of"));
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
        assert!(help.contains("--as-of"));
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
    fn system_doctor_parses_json_flag() {
        let cli = Cli::try_parse_from(["time_tracer_cli", "system", "doctor", "--json"]).unwrap();

        match cli.command {
            Command::System(args) => match args.command {
                SystemCommand::Doctor(args) => assert!(args.json),
            },
            _ => panic!("expected system command"),
        }
    }

    #[test]
    fn about_licenses_full_parses_under_about_family() {
        let cli = Cli::try_parse_from(["time_tracer_cli", "about", "licenses", "--full"]).unwrap();

        match cli.command {
            Command::About(args) => match args.command {
                AboutCommand::Licenses(args) => assert!(args.full),
                _ => panic!("expected about licenses command"),
            },
            _ => panic!("expected about command"),
        }
    }

    #[test]
    fn report_chart_parses_under_report_family() {
        let cli = Cli::try_parse_from([
            "time_tracer_cli",
            "report",
            "chart",
            "--type",
            "line",
            "--from",
            "20260101",
            "--to",
            "20260107",
        ])
        .unwrap();

        match cli.command {
            Command::Report(args) => match args.command {
                ReportCommand::Chart(args) => {
                    assert_eq!(args.from.as_deref(), Some("20260101"));
                    assert_eq!(args.to.as_deref(), Some("20260107"));
                }
                _ => panic!("expected report chart command"),
            },
            _ => panic!("expected report command"),
        }
    }

    #[test]
    fn legacy_chart_top_level_is_removed() {
        let error = Cli::try_parse_from(["time_tracer_cli", "chart", "--help"]).unwrap_err();
        assert_eq!(error.kind(), ErrorKind::InvalidSubcommand);
    }

    #[test]
    fn legacy_utility_top_levels_are_removed() {
        let doctor_error =
            Cli::try_parse_from(["time_tracer_cli", "doctor", "--help"]).unwrap_err();
        let licenses_error =
            Cli::try_parse_from(["time_tracer_cli", "licenses", "--help"]).unwrap_err();
        let tracer_error = Cli::try_parse_from(["time_tracer_cli", "tracer"]).unwrap_err();
        let motto_error = Cli::try_parse_from(["time_tracer_cli", "motto"]).unwrap_err();

        assert_eq!(doctor_error.kind(), ErrorKind::InvalidSubcommand);
        assert_eq!(licenses_error.kind(), ErrorKind::InvalidSubcommand);
        assert_eq!(tracer_error.kind(), ErrorKind::InvalidSubcommand);
        assert_eq!(motto_error.kind(), ErrorKind::InvalidSubcommand);
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
