use serde_json::Value;

use crate::error::AppError;

pub(crate) use super::invoke::ReportTextOutput;
use super::{CoreRuntime, invoke};

pub struct ReportClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> ReportClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn render(&self, request: &Value) -> Result<ReportTextOutput, AppError> {
        if request.get("days_list").is_some() {
            return invoke::run_report_batch_text(self.runtime, request);
        }
        invoke::run_report_text(self.runtime, request)
    }

    pub fn list_targets(&self, display_mode: &str) -> Result<Vec<String>, AppError> {
        invoke::run_report_targets(self.runtime, display_mode)
    }

    pub fn export(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_report_export(self.runtime, request)
    }
}
