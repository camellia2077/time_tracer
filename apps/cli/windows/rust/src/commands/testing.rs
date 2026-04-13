use std::cell::RefCell;
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

use serde_json::Value;

use crate::commands::handlers::report::{RenderedReport, ReportWindowMetadata};
use crate::core::runtime::{
    CliCommandDefaults, CliConfig, CliDefaults, TreeResponse, TxtResolveOutput,
};
use crate::error::AppError;

use super::handler::CommandContext;

pub(crate) fn sample_cli_config() -> CliConfig {
    CliConfig {
        default_save_processed_output: false,
        default_date_check_mode: Some("none".to_string()),
        defaults: CliDefaults {
            default_format: Some("md".to_string()),
        },
        command_defaults: CliCommandDefaults {
            export_format: Some("md".to_string()),
            query_format: Some("md".to_string()),
            convert_date_check_mode: Some("continuity".to_string()),
            convert_save_processed_output: Some(true),
            convert_validate_logic: Some(true),
            convert_validate_structure: Some(true),
            ingest_date_check_mode: Some("full".to_string()),
            ingest_save_processed_output: Some(false),
            validate_logic_date_check_mode: Some("continuity".to_string()),
        },
    }
}

pub(crate) struct RecordedQuerySession {
    command_names: RefCell<Vec<String>>,
    requests: RefCell<Vec<Value>>,
    data_response: String,
    tree_response: TreeResponse,
}

impl RecordedQuerySession {
    pub(crate) fn new_success(response: impl Into<String>) -> Self {
        Self {
            command_names: RefCell::new(Vec::new()),
            requests: RefCell::new(Vec::new()),
            data_response: response.into(),
            tree_response: TreeResponse {
                ok: true,
                found: true,
                error_message: String::new(),
                roots: Vec::new(),
                nodes: Vec::new(),
                error_code: String::new(),
                error_category: String::new(),
                hints: Vec::new(),
            },
        }
    }

    pub(crate) fn record(&self, command_name: &str, request: &Value) -> Result<String, AppError> {
        self.record_data(command_name, request)
    }

    pub(crate) fn with_tree_response(mut self, response: TreeResponse) -> Self {
        self.tree_response = response;
        self
    }

    pub(crate) fn record_data(
        &self,
        command_name: &str,
        request: &Value,
    ) -> Result<String, AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        Ok(self.data_response.clone())
    }

    pub(crate) fn record_tree(
        &self,
        command_name: &str,
        request: &Value,
    ) -> Result<TreeResponse, AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        Ok(self.tree_response.clone())
    }

    pub(crate) fn command_names(&self) -> Vec<String> {
        self.command_names.borrow().clone()
    }

    pub(crate) fn requests(&self) -> Vec<Value> {
        self.requests.borrow().clone()
    }
}

pub(crate) struct RecordedPipelineSession {
    command_names: RefCell<Vec<String>>,
    requests: RefCell<Vec<Value>>,
    cli_config: CliConfig,
    fail_on_command: Option<String>,
}

impl RecordedPipelineSession {
    pub(crate) fn new(cli_config: CliConfig) -> Self {
        Self {
            command_names: RefCell::new(Vec::new()),
            requests: RefCell::new(Vec::new()),
            cli_config,
            fail_on_command: None,
        }
    }

    pub(crate) fn with_failure_on(mut self, command_name: &str) -> Self {
        self.fail_on_command = Some(command_name.to_string());
        self
    }

    pub(crate) fn load_cli_config(&self) -> CliConfig {
        self.cli_config.clone()
    }

    pub(crate) fn record_ack(&self, command_name: &str, request: &Value) -> Result<(), AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        if self.fail_on_command.as_deref() == Some(command_name) {
            return Err(AppError::Logic(format!(
                "forced failure for `{command_name}`"
            )));
        }
        Ok(())
    }

    pub(crate) fn command_names(&self) -> Vec<String> {
        self.command_names.borrow().clone()
    }

    pub(crate) fn requests(&self) -> Vec<Value> {
        self.requests.borrow().clone()
    }
}

pub(crate) struct RecordedReportSession {
    command_names: RefCell<Vec<String>>,
    requests: RefCell<Vec<Value>>,
    cli_config: CliConfig,
    render_response: String,
    runtime_output_root: String,
    target_lists: RefCell<HashMap<String, Vec<String>>>,
    report_window_metadata: Option<ReportWindowMetadata>,
}

impl RecordedReportSession {
    pub(crate) fn new(cli_config: CliConfig, render_response: impl Into<String>) -> Self {
        Self {
            command_names: RefCell::new(Vec::new()),
            requests: RefCell::new(Vec::new()),
            cli_config,
            render_response: render_response.into(),
            runtime_output_root: std::env::temp_dir()
                .join("time_tracer_report_test_output")
                .to_string_lossy()
                .to_string(),
            target_lists: RefCell::new(HashMap::new()),
            report_window_metadata: None,
        }
    }

    pub(crate) fn with_runtime_output_root(
        mut self,
        runtime_output_root: impl Into<String>,
    ) -> Self {
        self.runtime_output_root = runtime_output_root.into();
        self
    }

    pub(crate) fn with_targets(self, target_type: &str, items: Vec<&str>) -> Self {
        self.target_lists.borrow_mut().insert(
            target_type.to_string(),
            items.into_iter().map(|value| value.to_string()).collect(),
        );
        self
    }

    pub(crate) fn with_window_metadata(mut self, metadata: ReportWindowMetadata) -> Self {
        self.report_window_metadata = Some(metadata);
        self
    }

    pub(crate) fn load_cli_config(&self) -> CliConfig {
        self.cli_config.clone()
    }

    pub(crate) fn runtime_output_root(&self) -> &str {
        &self.runtime_output_root
    }

    pub(crate) fn record_render(
        &self,
        command_name: &str,
        request: &Value,
    ) -> Result<RenderedReport, AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        Ok(RenderedReport {
            content: self.render_response.clone(),
            report_window_metadata: self.report_window_metadata.clone(),
        })
    }

    pub(crate) fn record_list_targets(
        &self,
        command_name: &str,
        target_type: &str,
    ) -> Result<Vec<String>, AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests
            .borrow_mut()
            .push(serde_json::json!({"type": target_type, "kind": "targets"}));
        Ok(self
            .target_lists
            .borrow()
            .get(target_type)
            .cloned()
            .unwrap_or_default())
    }

    pub(crate) fn record_export(
        &self,
        command_name: &str,
        request: &Value,
    ) -> Result<(), AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        Ok(())
    }

    pub(crate) fn command_names(&self) -> Vec<String> {
        self.command_names.borrow().clone()
    }

    pub(crate) fn requests(&self) -> Vec<Value> {
        self.requests.borrow().clone()
    }
}

pub(crate) struct RecordedExchangeSession {
    command_names: RefCell<Vec<String>>,
    requests: RefCell<Vec<Value>>,
    cli_config: CliConfig,
    response: String,
}

impl RecordedExchangeSession {
    pub(crate) fn new(cli_config: CliConfig, response: impl Into<String>) -> Self {
        Self {
            command_names: RefCell::new(Vec::new()),
            requests: RefCell::new(Vec::new()),
            cli_config,
            response: response.into(),
        }
    }

    pub(crate) fn load_cli_config(&self) -> CliConfig {
        self.cli_config.clone()
    }

    pub(crate) fn record_text(
        &self,
        command_name: &str,
        request: &Value,
    ) -> Result<String, AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        Ok(self.response.clone())
    }

    pub(crate) fn command_names(&self) -> Vec<String> {
        self.command_names.borrow().clone()
    }

    pub(crate) fn requests(&self) -> Vec<Value> {
        self.requests.borrow().clone()
    }
}

pub(crate) struct RecordedTxtSession {
    command_names: RefCell<Vec<String>>,
    requests: RefCell<Vec<Value>>,
    response: TxtResolveOutput,
}

impl RecordedTxtSession {
    pub(crate) fn new(response: TxtResolveOutput) -> Self {
        Self {
            command_names: RefCell::new(Vec::new()),
            requests: RefCell::new(Vec::new()),
            response,
        }
    }

    pub(crate) fn record_resolve(
        &self,
        command_name: &str,
        request: &Value,
    ) -> Result<TxtResolveOutput, AppError> {
        self.command_names
            .borrow_mut()
            .push(command_name.to_string());
        self.requests.borrow_mut().push(request.clone());
        Ok(self.response.clone())
    }

    pub(crate) fn command_names(&self) -> Vec<String> {
        self.command_names.borrow().clone()
    }

    pub(crate) fn requests(&self) -> Vec<Value> {
        self.requests.borrow().clone()
    }
}

pub(crate) fn context_with_output(output_path: impl AsRef<Path>) -> CommandContext {
    CommandContext {
        db_path: None,
        output_path: Some(output_path.as_ref().to_string_lossy().to_string()),
    }
}

pub(crate) fn default_context() -> CommandContext {
    CommandContext::default()
}

pub(crate) fn temp_output_path(prefix: &str, extension: &str) -> PathBuf {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|duration| duration.as_nanos())
        .unwrap_or_default();
    std::env::temp_dir().join(format!("{prefix}_{unique}.{extension}"))
}
