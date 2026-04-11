use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, invoke};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TxtResolveOutput {
    pub normalized_day_marker: String,
    pub found: bool,
    pub is_marker_valid: bool,
    pub can_save: bool,
    pub day_body: String,
    pub day_content_iso_date: Option<String>,
}

pub struct TxtClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> TxtClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn resolve_day_block(&self, request: &Value) -> Result<TxtResolveOutput, AppError> {
        invoke::run_txt_resolve_day_block(self.runtime, request)
    }
}
