use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, invoke};

pub struct TracerExchangeClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> TracerExchangeClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn export_package(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_tracer_exchange_export(self.runtime, request)
    }

    pub fn import_package(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_tracer_exchange_import(self.runtime, request)
    }

    pub fn inspect_package(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_tracer_exchange_inspect(self.runtime, request)
    }
}
