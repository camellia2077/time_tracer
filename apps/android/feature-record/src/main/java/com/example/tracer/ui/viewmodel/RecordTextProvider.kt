package com.example.tracer

import android.content.Context
import com.example.tracer.feature.record.R

interface RecordTextProvider {
    fun recordedActivityStatus(
        canonicalToken: String,
        durationText: String,
        activityHierarchyCreatedText: String? = null
    ): String
    fun activityHierarchyCreated(activityName: String, categoryName: String): String =
        "$activityName saved to $categoryName category"
    fun unavailableDuration(): String
}

object DefaultRecordTextProvider : RecordTextProvider {
    override fun recordedActivityStatus(
        canonicalToken: String,
        durationText: String,
        activityHierarchyCreatedText: String?
    ): String = listOfNotNull(canonicalToken, durationText, activityHierarchyCreatedText)
        .joinToString("\n")

    override fun unavailableDuration(): String = "n/a"
}

class AndroidRecordTextProvider(
    private val context: Context
) : RecordTextProvider {
    // Keep the two-line success payload explicit in code instead of relying on a formatted
    // string resource. The earlier resource-based version could collapse line breaks during
    // formatting, which made the snackbar layout depend on localization/resource behavior.
    override fun recordedActivityStatus(
        canonicalToken: String,
        durationText: String,
        activityHierarchyCreatedText: String?
    ): String = listOfNotNull(canonicalToken, durationText, activityHierarchyCreatedText)
        .joinToString("\n")

    override fun unavailableDuration(): String =
        context.getString(R.string.record_duration_unavailable)

    override fun activityHierarchyCreated(activityName: String, categoryName: String): String =
        context.getString(R.string.record_activity_hierarchy_created, activityName, categoryName)
}
