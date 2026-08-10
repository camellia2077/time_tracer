use super::{RenderedInsights, InsightsWindowMetadata};

pub(crate) fn build_empty_window_hint(rendered: &RenderedInsights) -> Option<String> {
    let metadata = rendered.insights_window_metadata.as_ref()?;
    if !should_append_empty_window_hint(metadata) {
        return None;
    }

    Some(format!(
        "\n\nWindow query returned no records.\nstart_date={}\nend_date={}\nrequested_days={}\nmatched_day_count={}\nmatched_record_count={}\n",
        metadata.start_date,
        metadata.end_date,
        metadata.requested_days,
        metadata.matched_day_count,
        metadata.matched_record_count
    ))
}

fn should_append_empty_window_hint(metadata: &InsightsWindowMetadata) -> bool {
    !metadata.has_records
        && metadata.matched_day_count == 0
        && metadata.matched_record_count == 0
        && !metadata.start_date.trim().is_empty()
        && !metadata.end_date.trim().is_empty()
        && metadata.requested_days > 0
}

#[cfg(test)]
mod tests {
    use super::build_empty_window_hint;
    use crate::commands::handlers::insights::{RenderedInsights, InsightsWindowMetadata};

    #[test]
    fn empty_window_hint_formats_expected_lines() {
        let rendered = RenderedInsights {
            content: "## Insights\n".to_string(),
            insights_window_metadata: Some(InsightsWindowMetadata {
                has_records: false,
                matched_day_count: 0,
                matched_record_count: 0,
                start_date: "2024-12-01".to_string(),
                end_date: "2024-12-31".to_string(),
                requested_days: 31,
            }),
        };

        let hint = build_empty_window_hint(&rendered).expect("empty-window hint");
        assert!(hint.contains("Window query returned no records."));
        assert!(hint.contains("start_date=2024-12-01"));
        assert!(hint.contains("end_date=2024-12-31"));
        assert!(hint.contains("requested_days=31"));
        assert!(hint.contains("matched_day_count=0"));
        assert!(hint.contains("matched_record_count=0"));
    }

    #[test]
    fn non_empty_window_does_not_append_hint() {
        let rendered = RenderedInsights {
            content: "## Insights\n".to_string(),
            insights_window_metadata: Some(InsightsWindowMetadata {
                has_records: true,
                matched_day_count: 1,
                matched_record_count: 2,
                start_date: "2026-01-01".to_string(),
                end_date: "2026-01-07".to_string(),
                requested_days: 7,
            }),
        };

        assert!(build_empty_window_hint(&rendered).is_none());
    }
}
