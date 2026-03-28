use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, invoke};

pub struct PipelineClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> PipelineClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn convert(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_pipeline_convert(self.runtime, request)
    }

    pub fn import_processed(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_pipeline_import(self.runtime, request)
    }

    pub fn ingest(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_pipeline_ingest(self.runtime, request)
    }

    pub fn validate_structure(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_pipeline_validate_structure(self.runtime, request)
    }

    pub fn validate_logic(&self, request: &Value) -> Result<(), AppError> {
        invoke::run_pipeline_validate_logic(self.runtime, request)
    }
}
