use serde_json::Value;

use crate::error::AppError;

pub(crate) use super::invoke::InsightsTextOutput;
use super::{CoreRuntime, invoke};

pub struct InsightsClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> InsightsClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn render(&self, request: &Value) -> Result<InsightsTextOutput, AppError> {
        if request.get("days_list").is_some() {
            return invoke::run_insights_batch_text(self.runtime, request);
        }
        invoke::run_insights_text(self.runtime, request)
    }

    pub fn list_targets(&self, display_mode: &str) -> Result<Vec<String>, AppError> {
        invoke::run_insights_targets(self.runtime, display_mode)
    }

    pub fn export(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_insights_export(self.runtime, request)
    }
}
