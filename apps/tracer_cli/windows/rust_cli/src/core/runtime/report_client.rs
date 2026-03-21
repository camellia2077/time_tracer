use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, invoke};

pub struct ReportClient<'runtime, 'api> {
    runtime: &'runtime CoreRuntime<'api>,
}

impl<'runtime, 'api> ReportClient<'runtime, 'api> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime<'api>) -> Self {
        Self { runtime }
    }

    pub fn render(&self, request: &Value) -> Result<String, AppError> {
        if request.get("days_list").is_some() {
            return invoke::run_report_batch_text(self.runtime, request);
        }
        invoke::run_report_text(self.runtime, request)
    }

    pub fn export(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_report_export(self.runtime, request)
    }
}
