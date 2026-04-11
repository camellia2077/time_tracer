use std::fs;
use std::path::Path;

use serde_json::{Value, json};

use crate::cli::{TxtArgs, TxtCommand, TxtViewDayArgs};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CoreApi, TxtResolveOutput};
use crate::error::AppError;

pub struct TxtHandler;

pub(crate) trait TxtSessionPort {
    fn resolve_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtResolveOutput, AppError>;
}

pub(crate) struct RuntimeTxtSessionPort;

impl TxtSessionPort for RuntimeTxtSessionPort {
    fn resolve_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtResolveOutput, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.txt().resolve_day_block(request)
    }
}

impl CommandHandler<TxtArgs> for TxtHandler {
    fn handle(&self, args: TxtArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            TxtCommand::ViewDay(args) => ViewDayHandler.handle(args, ctx),
        }
    }
}

struct ViewDayHandler;

impl CommandHandler<TxtViewDayArgs> for ViewDayHandler {
    fn handle(&self, args: TxtViewDayArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let body = run_view_day_with_port(args, ctx, &RuntimeTxtSessionPort)?;
        print!("{body}");
        Ok(())
    }
}

pub(crate) fn run_view_day_with_port(
    args: TxtViewDayArgs,
    ctx: &CommandContext,
    port: &dyn TxtSessionPort,
) -> Result<String, AppError> {
    let input_path = Path::new(&args.input);
    let content = fs::read_to_string(input_path)
        .map_err(|error| AppError::Io(format!("Read input txt failed: {error}")))?;
    let selected_month = infer_selected_month_from_path(input_path);
    let request = json!({
        "action": "resolve_day_block",
        "content": content,
        "day_marker": args.day,
        "selected_month": selected_month,
    });
    let resolved = port.resolve_day_block("txt", &ctx.without_output(), &request)?;
    if !resolved.is_marker_valid {
        return Err(AppError::InvalidArguments(
            "`--day` must be a valid MMDD marker.".to_string(),
        ));
    }
    if !resolved.found {
        return Err(AppError::Logic(format!(
            "Day block {} was not found in {}.",
            resolved.normalized_day_marker,
            input_path.display()
        )));
    }
    Ok(resolved.day_body)
}

fn infer_selected_month_from_path(path: &Path) -> String {
    // CLI keeps filename-to-month inference as host policy; runtime txt still
    // receives raw content plus an optional selected_month hint.
    let Some(filename) = path.file_name().and_then(|value| value.to_str()) else {
        return String::new();
    };
    if filename.len() != 11 {
        return String::new();
    }
    let bytes = filename.as_bytes();
    if !filename.ends_with(".txt")
        || bytes[4] != b'-'
        || !bytes[0..4].iter().all(u8::is_ascii_digit)
        || !bytes[5..7].iter().all(u8::is_ascii_digit)
    {
        return String::new();
    }
    filename[..7].to_string()
}

#[cfg(test)]
mod tests {
    use serde_json::Value;

    use crate::cli::TxtViewDayArgs;
    use crate::commands::testing::{RecordedTxtSession, default_context, temp_output_path};
    use crate::core::runtime::TxtResolveOutput;
    use crate::error::AppError;

    use super::{TxtSessionPort, infer_selected_month_from_path, run_view_day_with_port};

    struct TestTxtPort<'a> {
        recorded: &'a RecordedTxtSession,
    }

    impl TxtSessionPort for TestTxtPort<'_> {
        fn resolve_day_block(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<TxtResolveOutput, AppError> {
            self.recorded.record_resolve(command_name, request)
        }
    }

    #[test]
    fn view_day_reads_file_and_forwards_selected_month() {
        let temp_path = temp_output_path("txt_view_day", "txt");
        std::fs::write(&temp_path, "y2025\nm01\n\n0102\nline 1\nline 2\n")
            .expect("write temp txt");
        let month_path = temp_path.with_file_name("2025-01.txt");
        std::fs::rename(&temp_path, &month_path).expect("rename temp txt");

        let recorded = RecordedTxtSession::new(TxtResolveOutput {
            normalized_day_marker: "0102".to_string(),
            found: true,
            is_marker_valid: true,
            can_save: true,
            day_body: "line 1\nline 2\n".to_string(),
            day_content_iso_date: Some("2025-01-02".to_string()),
        });
        let port = TestTxtPort {
            recorded: &recorded,
        };

        let output = run_view_day_with_port(
            TxtViewDayArgs {
                input: month_path.to_string_lossy().to_string(),
                day: "0102".to_string(),
            },
            &default_context(),
            &port,
        )
        .expect("view day should succeed");

        assert_eq!(output, "line 1\nline 2\n");
        assert_eq!(recorded.command_names(), vec!["txt".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["action"], "resolve_day_block");
        assert_eq!(request["day_marker"], "0102");
        assert_eq!(request["selected_month"], "2025-01");
        assert!(request["content"].as_str().unwrap().contains("0102"));
    }

    #[test]
    fn view_day_returns_invalid_argument_for_bad_marker() {
        let temp_path = temp_output_path("txt_invalid_day", "txt");
        std::fs::write(&temp_path, "0102\nline 1\n").expect("write temp txt");

        let recorded = RecordedTxtSession::new(TxtResolveOutput {
            normalized_day_marker: "0132".to_string(),
            found: false,
            is_marker_valid: false,
            can_save: false,
            day_body: String::new(),
            day_content_iso_date: None,
        });
        let port = TestTxtPort {
            recorded: &recorded,
        };

        let error = run_view_day_with_port(
            TxtViewDayArgs {
                input: temp_path.to_string_lossy().to_string(),
                day: "0132".to_string(),
            },
            &default_context(),
            &port,
        )
        .expect_err("invalid marker should fail");

        match error {
            AppError::InvalidArguments(message) => {
                assert!(message.contains("valid MMDD"));
            }
            other => panic!("expected invalid arguments error, got {other:?}"),
        }
    }

    #[test]
    fn view_day_returns_logic_error_when_block_is_missing() {
        let temp_path = temp_output_path("txt_missing_day", "txt");
        std::fs::write(&temp_path, "0102\nline 1\n").expect("write temp txt");

        let recorded = RecordedTxtSession::new(TxtResolveOutput {
            normalized_day_marker: "0103".to_string(),
            found: false,
            is_marker_valid: true,
            can_save: false,
            day_body: String::new(),
            day_content_iso_date: None,
        });
        let port = TestTxtPort {
            recorded: &recorded,
        };

        let error = run_view_day_with_port(
            TxtViewDayArgs {
                input: temp_path.to_string_lossy().to_string(),
                day: "0103".to_string(),
            },
            &default_context(),
            &port,
        )
        .expect_err("missing marker should fail");

        match error {
            AppError::Logic(message) => {
                assert!(message.contains("0103"));
                assert!(message.contains("was not found"));
            }
            other => panic!("expected logic error, got {other:?}"),
        }
    }

    #[test]
    fn view_day_uses_empty_selected_month_when_filename_is_not_month_shaped() {
        let temp_path = temp_output_path("journal", "txt");
        std::fs::write(&temp_path, "0102\nline 1\n").expect("write temp txt");

        let recorded = RecordedTxtSession::new(TxtResolveOutput {
            normalized_day_marker: "0102".to_string(),
            found: true,
            is_marker_valid: true,
            can_save: true,
            day_body: "line 1\n".to_string(),
            day_content_iso_date: None,
        });
        let port = TestTxtPort {
            recorded: &recorded,
        };

        let output = run_view_day_with_port(
            TxtViewDayArgs {
                input: temp_path.to_string_lossy().to_string(),
                day: "0102".to_string(),
            },
            &default_context(),
            &port,
        )
        .expect("view day should still succeed");

        assert_eq!(output, "line 1\n");
        let request = recorded.requests().remove(0);
        assert_eq!(request["selected_month"], "");
    }

    #[test]
    fn infer_selected_month_requires_yyyy_mm_txt_filename() {
        assert_eq!(
            infer_selected_month_from_path(std::path::Path::new("2025-01.txt")),
            "2025-01"
        );
        assert_eq!(
            infer_selected_month_from_path(std::path::Path::new("journal.txt")),
            ""
        );
    }
}
