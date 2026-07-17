use std::fs;
use std::path::Path;

use serde_json::{Value, json};

use crate::cli::{TxtAppendEventArgs, TxtArgs, TxtCommand, TxtViewDayArgs};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CoreApi, TxtReplaceOutput, TxtResolveOutput};
use crate::error::AppError;

pub struct TxtHandler;

pub(crate) trait TxtSessionPort {
    fn resolve_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtResolveOutput, AppError>;
    fn replace_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtReplaceOutput, AppError>;
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

    fn replace_day_block(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<TxtReplaceOutput, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.txt().replace_day_block(request)
    }
}

impl CommandHandler<TxtArgs> for TxtHandler {
    fn handle(&self, args: TxtArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            TxtCommand::ViewDay(args) => ViewDayHandler.handle(args, ctx),
            TxtCommand::AppendEvent(args) => AppendEventHandler.handle(args, ctx),
        }
    }
}

struct ViewDayHandler;
struct AppendEventHandler;

impl CommandHandler<TxtViewDayArgs> for ViewDayHandler {
    fn handle(&self, args: TxtViewDayArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let body = run_view_day_with_port(args, ctx, &RuntimeTxtSessionPort)?;
        print!("{body}");
        Ok(())
    }
}

impl CommandHandler<TxtAppendEventArgs> for AppendEventHandler {
    fn handle(&self, args: TxtAppendEventArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let output = run_append_event_with_port(args, ctx, &RuntimeTxtSessionPort)?;
        print!("{output}");
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

pub(crate) fn run_append_event_with_port(
    args: TxtAppendEventArgs,
    ctx: &CommandContext,
    port: &dyn TxtSessionPort,
) -> Result<String, AppError> {
    let input_path = Path::new(&args.input);
    let content = fs::read_to_string(input_path)
        .map_err(|error| AppError::Io(format!("Read input txt failed: {error}")))?;
    let selected_month = infer_selected_month_from_path(input_path);
    let resolve_request = json!({
        "action": "resolve_day_block",
        "content": content,
        "day_marker": args.day,
        "selected_month": selected_month,
    });
    let resolved = port.resolve_day_block("txt", &ctx.without_output(), &resolve_request)?;
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

    let event_line = build_authored_event_line(&args)?;
    let edited_day_body = append_event_to_day_body(&resolved.day_body, &event_line);
    let replace_request = json!({
        "action": "replace_day_block",
        "content": content,
        "day_marker": args.day,
        "edited_day_body": edited_day_body,
    });
    let replaced = port.replace_day_block("txt", &ctx.without_output(), &replace_request)?;
    if !replaced.is_marker_valid {
        return Err(AppError::InvalidArguments(
            "`--day` must be a valid MMDD marker.".to_string(),
        ));
    }
    if !replaced.found {
        return Err(AppError::Logic(format!(
            "Day block {} was not found in {}.",
            replaced.normalized_day_marker,
            input_path.display()
        )));
    }

    fs::write(input_path, &replaced.updated_content)
        .map_err(|error| AppError::Io(format!("Write updated txt failed: {error}")))?;
    Ok(format!(
        "Appended event to {} in {}:\n{}\n",
        replaced.normalized_day_marker,
        input_path.display(),
        event_line
    ))
}

fn build_authored_event_line(args: &TxtAppendEventArgs) -> Result<String, AppError> {
    let mut line = if let Some(time) = &args.time {
        validate_hhmm(time, "--time")?;
        format!("{time}{}", args.activity)
    } else {
        let start = args.start.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("`--start` is required for interval events.".to_string())
        })?;
        let end = args.end.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("`--end` is required for interval events.".to_string())
        })?;
        validate_hhmm(start, "--start")?;
        validate_hhmm(end, "--end")?;
        format!("{start}-{end}{}", args.activity)
    };

    if let Some(remark) = &args.remark {
        if !remark.is_empty() {
            line.push_str(" //");
            line.push_str(remark);
        }
    }
    Ok(line)
}

fn append_event_to_day_body(day_body: &str, event_line: &str) -> String {
    let lines: Vec<&str> = day_body.split('\n').collect();
    let trailing_empty_count = lines.iter().rev().take_while(|line| line.is_empty()).count();
    let insert_at = lines.len().saturating_sub(trailing_empty_count);

    let mut rebuilt = Vec::with_capacity(lines.len().saturating_add(1));
    rebuilt.extend(lines[..insert_at].iter().copied().map(str::to_string));
    rebuilt.push(event_line.to_string());
    rebuilt.extend(lines[insert_at..].iter().copied().map(str::to_string));
    rebuilt.join("\n")
}

fn validate_hhmm(value: &str, flag_name: &str) -> Result<(), AppError> {
    if value.len() != 4 || !value.as_bytes().iter().all(u8::is_ascii_digit) {
        return Err(AppError::InvalidArguments(format!(
            "{flag_name} expects HHMM digits, got `{value}`."
        )));
    }

    let hour: i32 = value[0..2]
        .parse()
        .map_err(|_| AppError::InvalidArguments(format!(
            "{flag_name} expects HHMM digits, got `{value}`."
        )))?;
    let minute: i32 = value[2..4]
        .parse()
        .map_err(|_| AppError::InvalidArguments(format!(
            "{flag_name} expects HHMM digits, got `{value}`."
        )))?;
    if !(0..24).contains(&hour) || !(0..60).contains(&minute) {
        return Err(AppError::InvalidArguments(format!(
            "{flag_name} expects a valid HHMM time, got `{value}`."
        )));
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use serde_json::Value;

    use crate::cli::{TxtAppendEventArgs, TxtViewDayArgs};
    use crate::commands::testing::{RecordedTxtSession, default_context, temp_output_path};
    use crate::core::runtime::{TxtReplaceOutput, TxtResolveOutput};
    use crate::error::AppError;

    use super::{
        TxtSessionPort, append_event_to_day_body, build_authored_event_line,
        infer_selected_month_from_path, run_append_event_with_port, run_view_day_with_port,
    };

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

        fn replace_day_block(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<TxtReplaceOutput, AppError> {
            self.recorded.record_replace(command_name, request)
        }
    }

    #[test]
    fn view_day_reads_file_and_forwards_selected_month() {
        let temp_path = temp_output_path("txt_view_day", "txt");
        std::fs::write(&temp_path, "y2025\nm01\n\nd0102\nline 1\nline 2\n")
            .expect("write temp txt");
        let month_dir = temp_path.with_extension("");
        std::fs::create_dir_all(&month_dir).expect("create month temp dir");
        let month_path = month_dir.join("2025-01.txt");
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
        assert!(request["content"].as_str().unwrap().contains("d0102"));
    }

    #[test]
    fn view_day_returns_invalid_argument_for_bad_marker() {
        let temp_path = temp_output_path("txt_invalid_day", "txt");
        std::fs::write(&temp_path, "d0102\nline 1\n").expect("write temp txt");

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
        std::fs::write(&temp_path, "d0102\nline 1\n").expect("write temp txt");

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
        std::fs::write(&temp_path, "d0102\nline 1\n").expect("write temp txt");

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

    #[test]
    fn append_event_builds_point_line_with_optional_remark() {
        let line = build_authored_event_line(&TxtAppendEventArgs {
            input: "input.txt".to_string(),
            day: "0102".to_string(),
            time: Some("0904".to_string()),
            start: None,
            end: None,
            activity: "study".to_string(),
            remark: Some("focus".to_string()),
        })
        .expect("point line should build");

        assert_eq!(line, "0904study //focus");
    }

    #[test]
    fn append_event_builds_interval_line() {
        let line = build_authored_event_line(&TxtAppendEventArgs {
            input: "input.txt".to_string(),
            day: "0102".to_string(),
            time: None,
            start: Some("0900".to_string()),
            end: Some("1030".to_string()),
            activity: "study".to_string(),
            remark: None,
        })
        .expect("interval line should build");

        assert_eq!(line, "0900-1030study");
    }

    #[test]
    fn append_event_rejects_invalid_hhmm() {
        let error = build_authored_event_line(&TxtAppendEventArgs {
            input: "input.txt".to_string(),
            day: "0102".to_string(),
            time: Some("2460".to_string()),
            start: None,
            end: None,
            activity: "study".to_string(),
            remark: None,
        })
        .expect_err("invalid hhmm should fail");

        match error {
            AppError::InvalidArguments(message) => {
                assert!(message.contains("--time"));
                assert!(message.contains("2460"));
            }
            other => panic!("expected invalid arguments error, got {other:?}"),
        }
    }

    #[test]
    fn append_event_keeps_trailing_blank_lines_after_insert() {
        let edited = append_event_to_day_body("0638醒\n\n", "0900-1030study");
        assert_eq!(edited, "0638醒\n0900-1030study\n\n");
    }

    #[test]
    fn append_event_updates_existing_day_block_and_writes_file() {
        let temp_path = temp_output_path("txt_append_event", "txt");
        std::fs::write(&temp_path, "y2025\nm01\n\nd0102\n0638醒\n").expect("write temp txt");
        let month_dir = temp_path.with_extension("");
        std::fs::create_dir_all(&month_dir).expect("create month temp dir");
        let month_path = month_dir.join("2025-01.txt");
        std::fs::rename(&temp_path, &month_path).expect("rename temp txt");

        let recorded = RecordedTxtSession::new(TxtResolveOutput {
            normalized_day_marker: "0102".to_string(),
            found: true,
            is_marker_valid: true,
            can_save: true,
            day_body: "0638醒\n".to_string(),
            day_content_iso_date: Some("2025-01-02".to_string()),
        })
        .with_replace_response(TxtReplaceOutput {
            normalized_day_marker: "0102".to_string(),
            found: true,
            is_marker_valid: true,
            updated_content: "y2025\nm01\n\nd0102\n0638醒\n0900-1030study //focus\n".to_string(),
        });
        let port = TestTxtPort {
            recorded: &recorded,
        };

        let output = run_append_event_with_port(
            TxtAppendEventArgs {
                input: month_path.to_string_lossy().to_string(),
                day: "0102".to_string(),
                time: None,
                start: Some("0900".to_string()),
                end: Some("1030".to_string()),
                activity: "study".to_string(),
                remark: Some("focus".to_string()),
            },
            &default_context(),
            &port,
        )
        .expect("append event should succeed");

        assert!(output.contains("Appended event to 0102"));
        assert!(output.contains("0900-1030study //focus"));
        let requests = recorded.requests();
        assert_eq!(requests[0]["action"], "resolve_day_block");
        assert_eq!(requests[1]["action"], "replace_day_block");
        assert_eq!(requests[1]["edited_day_body"], "0638醒\n0900-1030study //focus\n");
        let written = std::fs::read_to_string(month_path).expect("read updated txt");
        assert!(written.contains("0900-1030study //focus"));
    }
}
