use serde::Deserialize;

use super::TreeResponse;

#[derive(Default, Deserialize)]
pub(crate) struct ErrorContract {
    #[serde(default)]
    pub(crate) error_code: String,
    #[serde(default)]
    pub(crate) error_category: String,
    #[serde(default)]
    pub(crate) hints: Vec<String>,
}

pub(crate) fn format_error_detail(error_message: String, contract: &ErrorContract) -> String {
    let mut detail = if contract.error_code.is_empty() {
        error_message
    } else if error_message.is_empty() {
        format!("[{}]", contract.error_code)
    } else {
        format!("[{}] {}", contract.error_code, error_message)
    };
    append_contract_context(&mut detail, &contract.error_category, &contract.hints);
    detail
}

pub(crate) fn format_tree_error_detail(payload: &TreeResponse) -> String {
    let mut detail = if payload.error_code.is_empty() {
        payload.error_message.clone()
    } else if payload.error_message.is_empty() {
        format!("[{}]", payload.error_code)
    } else {
        format!("[{}] {}", payload.error_code, payload.error_message)
    };
    append_contract_context(&mut detail, &payload.error_category, &payload.hints);
    detail
}

fn append_contract_context(detail: &mut String, error_category: &str, hints: &[String]) {
    let category = error_category.trim();
    let hint_text = hints
        .iter()
        .map(|item| item.trim())
        .filter(|item| !item.is_empty())
        .collect::<Vec<_>>()
        .join(" | ");

    let mut segments: Vec<String> = Vec::new();
    if !category.is_empty() {
        segments.push(format!("category={category}"));
    }
    if !hint_text.is_empty() {
        segments.push(format!("hints={hint_text}"));
    }
    if segments.is_empty() {
        return;
    }

    let suffix = format!("({})", segments.join(", "));
    if detail.is_empty() {
        *detail = suffix;
    } else {
        detail.push(' ');
        detail.push_str(&suffix);
    }
}
