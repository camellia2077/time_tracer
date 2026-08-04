use serde::Deserialize;

use super::super::errors::ErrorContract;
#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct ReportWindowMetadata {
    pub(crate) has_records: bool,
    pub(crate) matched_day_count: i32,
    pub(crate) matched_record_count: i32,
    pub(crate) start_date: String,
    pub(crate) end_date: String,
    pub(crate) requested_days: i32,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct ReportTextOutput {
    pub(crate) content: String,
    pub(crate) report_window_metadata: Option<ReportWindowMetadata>,
}

#[derive(Deserialize)]
pub(crate) struct AckResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct TextResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(default)]
    pub(crate) content: String,
    #[serde(default)]
    pub(crate) has_records: Option<bool>,
    #[serde(default)]
    pub(crate) matched_day_count: Option<i32>,
    #[serde(default)]
    pub(crate) matched_record_count: Option<i32>,
    #[serde(default)]
    pub(crate) start_date: Option<String>,
    #[serde(default)]
    pub(crate) end_date: Option<String>,
    #[serde(default)]
    pub(crate) requested_days: Option<i32>,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct ReportTargetsResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(default)]
    pub(crate) items: Vec<String>,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct TxtResolveResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(default)]
    pub(crate) normalized_day_marker: String,
    #[serde(default)]
    pub(crate) found: bool,
    #[serde(default)]
    pub(crate) is_marker_valid: bool,
    #[serde(default)]
    pub(crate) can_save: bool,
    #[serde(default)]
    pub(crate) day_body: String,
    #[serde(default)]
    pub(crate) day_content_iso_date: Option<String>,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct SemanticTreeResponse {
    #[serde(default)]
    pub(crate) roots: Vec<super::super::TreeNode>,
}

#[derive(Deserialize)]
pub(crate) struct TxtReplaceResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(default)]
    pub(crate) normalized_day_marker: String,
    #[serde(default)]
    pub(crate) found: bool,
    #[serde(default)]
    pub(crate) is_marker_valid: bool,
    #[serde(default)]
    pub(crate) updated_content: String,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct AliasCanonicalReplacementResponse {
    pub(crate) old_canonical: String,
    pub(crate) new_canonical: String,
}

#[derive(Deserialize)]
pub(crate) struct ActivityHierarchyOperationResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(default)]
    pub(crate) updated_toml_content: String,
    #[serde(default)]
    pub(crate) replacements: Vec<AliasCanonicalReplacementResponse>,
    #[serde(default)]
    pub(crate) alias_replacements: Vec<AliasKeyReplacementResponse>,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct ActivityHierarchyDocumentResponse {
    pub(crate) source_name: String,
    pub(crate) updated_toml_content: String,
}

#[derive(Deserialize)]
pub(crate) struct ActivityHierarchyCrossDocumentOperationResponse {
    pub(crate) ok: bool,
    #[serde(default)]
    pub(crate) error_message: String,
    #[serde(default)]
    pub(crate) updated_documents: Vec<ActivityHierarchyDocumentResponse>,
    #[serde(default)]
    pub(crate) replacements: Vec<AliasCanonicalReplacementResponse>,
    #[serde(default)]
    pub(crate) alias_replacements: Vec<AliasKeyReplacementResponse>,
    #[serde(flatten)]
    pub(crate) error_contract: ErrorContract,
}

#[derive(Deserialize)]
pub(crate) struct AliasKeyReplacementResponse {
    pub(crate) old_alias: String,
    pub(crate) new_alias: String,
}

impl TextResponse {
    pub(crate) fn into_report_text_output(self) -> ReportTextOutput {
        let report_window_metadata = self.report_window_metadata();
        ReportTextOutput {
            content: self.content,
            report_window_metadata,
        }
    }

    fn report_window_metadata(&self) -> Option<ReportWindowMetadata> {
        let has_any = self.has_records.is_some()
            || self.matched_day_count.is_some()
            || self.matched_record_count.is_some()
            || self.start_date.is_some()
            || self.end_date.is_some()
            || self.requested_days.is_some();
        if !has_any {
            return None;
        }

        Some(ReportWindowMetadata {
            has_records: self.has_records.unwrap_or(false),
            matched_day_count: self.matched_day_count.unwrap_or(0),
            matched_record_count: self.matched_record_count.unwrap_or(0),
            start_date: self.start_date.clone().unwrap_or_default(),
            end_date: self.end_date.clone().unwrap_or_default(),
            requested_days: self.requested_days.unwrap_or(0),
        })
    }
}
