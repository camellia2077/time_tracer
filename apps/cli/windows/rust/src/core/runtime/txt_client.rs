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

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TxtReplaceOutput {
    pub normalized_day_marker: String,
    pub found: bool,
    pub is_marker_valid: bool,
    pub updated_content: String,
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

    pub fn replace_day_block(&self, request: &Value) -> Result<TxtReplaceOutput, AppError> {
        invoke::run_txt_replace_day_block(self.runtime, request)
    }

    pub fn replace_canonical_activity_names(
        &self,
        request: &Value,
    ) -> Result<TxtCanonicalReplaceOutput, AppError> {
        invoke::run_txt_replace_canonical_activity_names(self.runtime, request)
    }

    pub fn replace_alias_activity_names(
        &self,
        request: &Value,
    ) -> Result<TxtCanonicalReplaceOutput, AppError> {
        invoke::run_txt_replace_alias_activity_names(self.runtime, request)
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TxtCanonicalReplaceOutput {
    pub updated_content: String,
}
