use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, invoke};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AliasCanonicalReplacement {
    pub old_canonical: String,
    pub new_canonical: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AliasKeyReplacement {
    pub old_alias: String,
    pub new_alias: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AliasHierarchyOperationOutput {
    pub updated_toml_content: String,
    pub replacements: Vec<AliasCanonicalReplacement>,
    pub alias_replacements: Vec<AliasKeyReplacement>,
}

pub struct AliasHierarchyClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> AliasHierarchyClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn apply_operation(
        &self,
        request: &Value,
    ) -> Result<AliasHierarchyOperationOutput, AppError> {
        invoke::run_alias_hierarchy_operation(self.runtime, request)
    }

    pub fn render_text(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_alias_hierarchy_text(self.runtime, request)
    }
}
