use serde_json::Value;

use crate::cli::{TxtAppendEventArgs, TxtViewDayArgs};
use crate::commands::testing::{default_context, temp_output_path, RecordedTxtSession};
use crate::core::runtime::{TxtReplaceOutput, TxtResolveOutput};
use crate::error::AppError;

use super::{
    append_event_to_day_body, build_authored_event_line, infer_selected_month_from_path,
    run_append_event_with_port, run_view_day_with_port, TxtSessionPort,
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
    std::fs::write(&temp_path, "y2025\nm01\n\nd0102\nline 1\nline 2\n").expect("write temp txt");
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
    assert_eq!(
        requests[1]["edited_day_body"],
        "0638醒\n0900-1030study //focus\n"
    );
    let written = std::fs::read_to_string(month_path).expect("read updated txt");
    assert!(written.contains("0900-1030study //focus"));
}
