use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, TreeResponse, invoke};

pub struct QueryClient<'runtime, 'api> {
    runtime: &'runtime CoreRuntime<'api>,
}

impl<'runtime, 'api> QueryClient<'runtime, 'api> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime<'api>) -> Self {
        Self { runtime }
    }

    pub fn data(&self, request: &Value) -> Result<String, AppError> {
        invoke::run_query_data(self.runtime, request)
    }

    pub fn tree(&self, request: &Value) -> Result<TreeResponse, AppError> {
        invoke::run_tree_query(self.runtime, request)
    }
}
