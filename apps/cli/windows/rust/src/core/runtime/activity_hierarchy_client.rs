use serde::Deserialize;
use serde_json::Value;

use crate::error::AppError;

use super::{CoreRuntime, invoke};

#[derive(Clone, Debug, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "snake_case")]
pub enum ActivityHierarchyNodeKind {
    Leaf,
    Group,
}

impl Default for ActivityHierarchyNodeKind {
    fn default() -> Self {
        Self::Leaf
    }
}

#[derive(Clone, Debug, Default, Deserialize, PartialEq, Eq)]
pub struct ActivityHierarchyTreeNode {
    pub canonical_key: String,
    pub path: String,
    #[serde(default)]
    pub kind: ActivityHierarchyNodeKind,
    #[serde(default)]
    pub aliases: Vec<String>,
    #[serde(default)]
    pub children: Vec<ActivityHierarchyTreeNode>,
}

#[derive(Clone, Debug, Default, Deserialize, PartialEq, Eq)]
pub struct ActivityHierarchyTree {
    pub parent: String,
    #[serde(default)]
    pub nodes: Vec<ActivityHierarchyTreeNode>,
}

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
pub struct ActivityHierarchyOperationOutput {
    pub updated_toml_content: String,
    pub replacements: Vec<AliasCanonicalReplacement>,
    pub alias_replacements: Vec<AliasKeyReplacement>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct ActivityHierarchyDocumentOutput {
    pub source_name: String,
    pub updated_toml_content: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct ActivityHierarchyCrossDocumentOperationOutput {
    pub updated_documents: Vec<ActivityHierarchyDocumentOutput>,
    pub replacements: Vec<AliasCanonicalReplacement>,
    pub alias_replacements: Vec<AliasKeyReplacement>,
}

pub struct ActivityHierarchyClient<'runtime> {
    runtime: &'runtime CoreRuntime,
}

impl<'runtime> ActivityHierarchyClient<'runtime> {
    pub(crate) fn new(runtime: &'runtime CoreRuntime) -> Self {
        Self { runtime }
    }

    pub fn apply_operation(
        &self,
        request: &Value,
    ) -> Result<ActivityHierarchyOperationOutput, AppError> {
        invoke::run_activity_hierarchy_operation(self.runtime, request)
    }

    pub fn move_leaf_between_documents(
        &self,
        request: &Value,
    ) -> Result<ActivityHierarchyCrossDocumentOperationOutput, AppError> {
        invoke::run_activity_hierarchy_node_move(self.runtime, request)
    }

    pub fn move_node_between_documents(
        &self,
        request: &Value,
    ) -> Result<ActivityHierarchyCrossDocumentOperationOutput, AppError> {
        invoke::run_activity_hierarchy_node_move(self.runtime, request)
    }

    pub fn describe(&self, request: &Value) -> Result<ActivityHierarchyTree, AppError> {
        invoke::run_activity_hierarchy_describe(self.runtime, request)
    }
}
